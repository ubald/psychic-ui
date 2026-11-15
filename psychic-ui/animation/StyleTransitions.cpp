#include <algorithm>
#include <type_traits>
#include "StyleTransitions.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    // Every animatable property's type has an interpolation
    #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
        static_assert(!(animatable) || Interpolable<Type>, #name " is animatable but its type doesn't interpolate");
    PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
    #undef PSYCHIC_UI_X

    namespace detail {

        // region Animations

        bool TweenAnimation::advance(const std::chrono::nanoseconds now) {
            const bool running = tween.advance(now);
            if (update) {
                update(tween.value());
            }
            return running;
        }

        void TweenAnimation::finish() {
            tween.finish();
            if (update) {
                update(tween.value());
            }
        }

        void startTween(Div *owner, const std::shared_ptr<TweenAnimation> &animation, const Channels &target, const Timing &timing) {
            Window *window = owner ? owner->window() : nullptr;
            if (!window || window->reducedMotion() || timing.instant()) {
                animation->tween.jump(target);
                if (animation->update) {
                    animation->update(target);
                }
                return;
            }
            if (animation->tween.running() || animation->tween.value() != target) {
                animation->tween.retarget(target, timing, window->animationClock());
                window->startAnimation(animation);
            }
        }

        const RunningTransition *StyleTransitions::find(const Property property) const {
            auto it = std::find_if(running.begin(), running.end(), [property](const auto &r) { return r.property == property; });
            return it == running.end() ? nullptr : &*it;
        }

        bool StyleTransitions::advance(const std::chrono::nanoseconds now) {
            return owner && owner->advanceTransitions(now);
        }

        void StyleTransitions::finish() {
            if (owner) {
                owner->finishTransitions();
            }
        }

        // endregion

        // region Property channels

        namespace {
            template<class T>
            bool channelsOf(const Style &style, const Property property, const T &value, Channels &channels, Length::Unit &unit) {
                if constexpr (std::is_same_v<T, Color>) {
                    // Unset is transparent: a color that appears fades in, one that goes fades out
                    channels = style.has(property) ? Interpolation<Color>::channels(value) : Channels{};
                    return true;
                } else if constexpr (std::is_same_v<T, Length>) {
                    if (value.isAuto()) {
                        return false;
                    }
                    channels = Interpolation<Length>::channels(value);
                    unit     = value.unit;
                    return true;
                } else if constexpr (Interpolable<T>) {
                    channels = Interpolation<T>::channels(value);
                    return true;
                } else {
                    return false;
                }
            }

            template<class T, class Set>
            void assignChannels(const Channels &channels, const Length::Unit unit, Set set) {
                if constexpr (std::is_same_v<T, Length>) {
                    set(Length{channels[0], unit});
                } else if constexpr (Interpolable<T>) {
                    set(Interpolation<T>::value(channels, T{}));
                }
            }
        }

        bool propertyChannels(const Style &style, const Property property, Channels &channels, Length::Unit &unit) {
            switch (property) {
                #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
                    case Property::name: return (animatable) && channelsOf<Type>(style, property, style.name(), channels, unit);
                PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
                #undef PSYCHIC_UI_X
            }
            return false;
        }

        void setPropertyChannels(Style &style, const Property property, const Channels &channels, const Length::Unit unit) {
            switch (property) {
                #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
                    case Property::name: \
                        assignChannels<Type>(channels, unit, [&style](const Type &value) { style.name(value); }); \
                        break;
                PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
                #undef PSYCHIC_UI_X
            }
        }

        // endregion
    }

    // region Div

    Style Div::targetStyle() const {
        Style target = _computedStyle;
        if (_transitions) {
            for (const auto &r: _transitions->running) {
                if (r.targetSet) {
                    detail::setPropertyChannels(target, r.property, r.tween.target(), r.unit);
                } else {
                    target.unset(r.property);
                }
            }
        }
        return target;
    }

    bool Div::transitioning() const {
        return _transitions && !_transitions->running.empty();
    }

    bool Div::transitioning(const Property property) const {
        return _transitions && _transitions->find(property);
    }

    void Div::updateTransitions(Style &computed) {
        Window *window = this->window();
        const bool allowed = window && !window->reducedMotion() && _styledIn == window
                             && _styledGeneration == styleManager()->rulesGeneration();
        if (!allowed) {
            // Nothing blends: the first style in a window, new rules (a theme), reduced motion
            cancelTransitions();
            return;
        }

        const Transition &transition = computed.transition();
        const auto        start      = window->animationClock();
        // What changed from what is painted now
        PropertyMask changed = _computedStyle.diff(computed) & animatableProperties;

        // Those running keep going to the same value, head to a new one, or stop where the new style says (it has no
        // transition for them any more, or the new value doesn't interpolate)
        if (_transitions) {
            auto &running = _transitions->running;
            for (auto it = running.begin(); it != running.end();) {
                const Property property = it->property;
                changed.reset(index(property));
                Channels      target{};
                Length::Unit  unit{it->unit};
                const Timing  *timing = transition.timing(property);
                const bool    valid   = detail::propertyChannels(computed, property, target, unit) && unit == it->unit;
                const bool    set     = computed.has(property);
                if (!valid || !timing || timing->instant()) {
                    it = running.erase(it); // The new value, at once
                    continue;
                }
                if (set == it->targetSet && target == it->tween.target()) {
                    // The same end: it goes on
                } else {
                    it->tween.retarget(target, *timing, start);
                    it->targetSet = set;
                }
                detail::setPropertyChannels(computed, property, it->tween.value(), it->unit);
                ++it;
            }
        }

        // The new ones
        if (!transition.empty()) {
            changed &= transition.properties();
            for (std::size_t i = 0; changed.any() && i < propertyCount; ++i) {
                if (!changed.test(i)) {
                    continue;
                }
                changed.reset(i);
                const auto   property = static_cast<Property>(i);
                const Timing *timing  = transition.timing(property);
                Channels     from{};
                Channels     to{};
                Length::Unit fromUnit{};
                Length::Unit toUnit{};
                if (!timing || timing->instant()
                    || !detail::propertyChannels(_computedStyle, property, from, fromUnit)
                    || !detail::propertyChannels(computed, property, to, toUnit)
                    || fromUnit != toUnit || from == to) {
                    continue;
                }
                if (!_transitions) {
                    _transitions = std::make_shared<detail::StyleTransitions>(this);
                }
                detail::RunningTransition &r = _transitions->running.emplace_back(
                    detail::RunningTransition{property, Tween{from}, toUnit, computed.has(property)}
                );
                r.tween.retarget(to, *timing, start);
                detail::setPropertyChannels(computed, property, from, toUnit);
            }
        }

        if (_transitions && !_transitions->running.empty()) {
            window->startAnimation(_transitions);
        }
    }

    bool Div::advanceTransitions(const std::chrono::nanoseconds now) {
        if (!_transitions) {
            return false;
        }
        PropertyMask changed{};
        auto         &running = _transitions->running;
        for (auto it = running.begin(); it != running.end();) {
            const bool moving = it->tween.advance(now);
            if (moving || it->targetSet) {
                detail::setPropertyChannels(_computedStyle, it->property, it->tween.value(), it->unit);
            } else {
                _computedStyle.unset(it->property);
            }
            changed.set(index(it->property));
            it = moving ? it + 1 : running.erase(it);
        }
        if (changed.any()) {
            transitionsChanged(changed);
        }
        return !running.empty();
    }

    void Div::finishTransitions() {
        if (!_transitions) {
            return;
        }
        for (auto &r: _transitions->running) {
            r.tween.finish();
        }
        advanceTransitions(std::chrono::nanoseconds::zero()); // Finished: they write their ends and go
    }

    void Div::cancelTransitions() {
        if (_transitions) {
            _transitions->running.clear();
        }
    }

    void Div::transitionsChanged(const PropertyMask &changed) {
        updateLayout(changed);
        styleUpdated();
        // The children that inherit them take the new values with their next style
        for (const auto &child: _children) {
            if ((changed & child->inheritedProperties()).any()) {
                child->invalidateStyle();
            }
        }
    }

    void Div::animate(const float from, const float to, const Timing &timing, std::function<void(float)> update) {
        std::erase_if(_animations, [](const auto &animation) {
            return !static_cast<const detail::TweenAnimation &>(*animation).tween.running();
        });
        auto animation    = std::make_shared<detail::TweenAnimation>();
        animation->tween  = Tween{Interpolation<float>::channels(from), 1};
        animation->update = [update = std::move(update)](const Channels &channels) { update(channels[0]); };
        animation->update(animation->tween.value());
        detail::startTween(this, animation, Interpolation<float>::channels(to), timing);
        if (animation->tween.running()) {
            _animations.push_back(std::move(animation));
        }
    }

    // endregion
}
