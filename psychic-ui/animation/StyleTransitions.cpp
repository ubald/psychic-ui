#include <algorithm>
#include <cmath>
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
                             && _styledGeneration == styleManager()->rulesGeneration() && !starting();
        if (!allowed) {
            // Nothing blends: the first style in a window, its starting style, new rules (a theme), reduced motion
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
                // Zero is zero in any unit: a percent translation going back to 0 px blends
                const bool    valid   = detail::propertyChannels(computed, property, target, unit)
                                        && (unit == it->unit || target[0] == 0.0f);
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
                    || !detail::propertyChannels(computed, property, to, toUnit)) {
                    continue;
                }
                // Zero is zero in any unit (a length going from 50% to 0 px blends in percent); otherwise a change of
                // unit snaps
                if (fromUnit != toUnit) {
                    if (to[0] == 0.0f) {
                        toUnit = fromUnit;
                    } else if (from[0] != 0.0f) {
                        continue;
                    }
                }
                if (from == to) {
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

    // region Entering and leaving

    namespace {
        std::string lowercase(std::string string) {
            std::transform(string.begin(), string.end(), string.begin(), ::tolower);
            return string;
        }
    }

    bool Div::starting() const {
        for (const Div *div = this; div; div = div->_parent) {
            if (div->_startingApplied) {
                return true;
            }
        }
        return false;
    }

    void Div::releaseStartingClass() {
        if (_startingApplied) {
            _startingApplied = false;
            updateClassName(_startingClass, false);
        }
    }

    void Div::exit(const std::string &className, std::function<void()> done) {
        if (_exit) {
            return;
        }
        Window *w = window();
        if (!w || w->reducedMotion() || _styledIn != w || starting()) {
            // Nothing to leave from: it was never painted, or nothing moves
            if (done) {
                done();
            }
            return;
        }
        _exit          = std::make_unique<Exit>(Exit{lowercase(className), std::move(done), _mouseEnabled, _mouseChildren});
        _mouseEnabled  = false;
        _mouseChildren = false;
        updateClassName(className, true);
        // Styled with it now, to know whether anything transitions (those started between frames start with the
        // next one): when nothing does, it leaves at once
        updateStyleRecursive();
        if (!subtreeTransitioning()) {
            finishExit();
            return;
        }
        w->_exitingDivs.push_back(weak_from_this());
    }

    void Div::cancelExit() {
        if (!_exit) {
            return;
        }
        const auto exit = std::move(_exit);
        _mouseEnabled  = exit->mouseEnabled;
        _mouseChildren = exit->mouseChildren;
        updateClassName(exit->className, false);
    }

    void Div::finishExit() {
        if (!_exit) {
            return;
        }
        const auto keepAlive = shared_from_this(); // `done` removes it
        const auto exit      = std::move(_exit);
        _mouseEnabled  = exit->mouseEnabled;
        _mouseChildren = exit->mouseChildren;
        // Without restyling: `done` takes it out of the window, and its next first style computes it anew
        _classNames.erase(exit->className);
        if (exit->done) {
            exit->done();
        }
        if (window()) {
            invalidateStyle(); // Still there after all
        }
    }

    bool Div::subtreeTransitioning() const {
        return transitioning() || std::any_of(_children.begin(), _children.end(), [](const auto &child) {
            return child->subtreeTransitioning();
        });
    }

    // endregion

    // region Layout transitions

    float Div::layoutOffsetX() const {
        return _layoutSlide ? _layoutSlide->offset->tween.value()[0] : 0.0f;
    }

    float Div::layoutOffsetY() const {
        return _layoutSlide ? _layoutSlide->offset->tween.value()[1] : 0.0f;
    }

    bool Div::layoutSliding() const {
        return _layoutSlide && _layoutSlide->offset->tween.running();
    }

    void Div::updateSlidingClass() {
        static const std::string sliding{"sliding"};
        if (layoutSliding() != _classNames.contains(sliding)) {
            updateClassName(sliding, layoutSliding());
        }
    }

    void Div::updateLayoutSlides(const SlideWalk &walk) {
        if (_computedStyle.display() == Display::None) {
            return; // Not laid out: what it holds appears in place once it is shown again
        }
        // Where its layout puts it in the window, scrolling aside (so scrolling never slides anything)
        const float x        = walk.originX + static_cast<float>(_x);
        const float y        = walk.originY + static_cast<float>(_y);
        const float paintedX = layoutOffsetX();
        const float paintedY = layoutOffsetY();

        if (!walk.timing) {
            _layoutSlide.reset(); // Its container has no layout transition (any more)
        } else {
            if (!_layoutSlide) {
                _layoutSlide = std::make_unique<LayoutSlide>();
                _layoutSlide->offset->tween  = Tween{Channels{}, 2};
                // The tween ends (or jumps) as the window advances it: the class goes with it
                _layoutSlide->offset->update = [weak = weak_from_this()](const Channels &) {
                    if (const auto div = weak.lock()) {
                        div->updateSlidingClass();
                    }
                };
            }
            LayoutSlide &slide = *_layoutSlide;
            Tween       &tween = slide.offset->tween;
            // Laid out in this container by the previous pass too: otherwise it was just added, or shown again
            const bool stayed = !walk.snap && slide.container == _parent && slide.pass + 1 == walk.pass;
            if (!stayed) {
                if (tween.running() || tween.value() != Channels{}) {
                    tween.jump(Channels{});
                }
            } else {
                // From where it was painted, relative to where its ancestors are painted from now on (an ancestor that
                // starts sliding carries it already)
                const float fromX = slide.x + paintedX + walk.paintedX - x - walk.offsetX;
                const float fromY = slide.y + paintedY + walk.paintedY - y - walk.offsetY;
                if (std::abs(fromX - paintedX) > 0.01f || std::abs(fromY - paintedY) > 0.01f) {
                    tween.jump(Channels{fromX, fromY});
                    detail::startTween(this, slide.offset, Channels{}, *walk.timing);
                }
            }
            slide.x         = x;
            slide.y         = y;
            slide.pass      = walk.pass;
            slide.container = _parent;
        }
        updateSlidingClass();

        const Timing &timing = _computedStyle.layoutTransition();
        const SlideWalk next{
            .pass     = walk.pass,
            .snap     = walk.snap,
            .originX  = x,
            .originY  = y,
            .paintedX = walk.paintedX + paintedX,
            .paintedY = walk.paintedY + paintedY,
            .offsetX  = walk.offsetX + layoutOffsetX(),
            .offsetY  = walk.offsetY + layoutOffsetY(),
            .timing   = timing.instant() ? nullptr : &timing
        };
        for (const auto &child: _children) {
            child->updateLayoutSlides(next);
        }
    }

    // endregion

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
