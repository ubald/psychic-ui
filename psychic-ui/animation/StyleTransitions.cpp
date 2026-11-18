#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include "Animation.hpp"
#include "Interpolation.hpp"
#include "StyleTransitions.hpp"
#include "Timing.hpp"
#include "Tween.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/style/Properties.hpp"
#include "psychic-ui/style/Style.hpp"
#include "psychic-ui/style/Values.hpp"

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
            auto it = std::ranges::find_if(running, [property](const auto &r) { return r.property == property; });
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
    }

    // region Property channels

    namespace {
        // Calls `f(get, set)` with an animatable property's accessors (`get(style)`, `set(style, value)`); false for
        // the others
        template<class F>
        bool withAccessors(const Property property, F &&f) {
            switch (property) {
                #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
                    case Property::name: \
                        if constexpr (animatable) { \
                            return std::forward<F>(f)([](const Style &style) { return style.name(); }, \
                                     [](Style &style, const Type &value) { style.name(value); }); \
                        } \
                        return false;
                PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
                #undef PSYCHIC_UI_X
            }
            return false;
        }

        // Unset is transparent: a color that appears fades in, one that goes fades out
        template<class T>
        T blended(const Style &style, const Property property, const T &value) {
            if constexpr (std::is_same_v<T, Color>) {
                return style.has(property) ? value : Color{};
            } else {
                return value;
            }
        }

        /**
         * Aim a property from its value in `from` to its value in `to`: where it ends goes in `targets`, the channels
         * of both ends in `start` and `end`; false when it doesn't interpolate (an auto length, another unit)
         */
        bool aim(const Property property, const Style &from, const Style &to, Style &targets, Channels &start, Channels &end) {
            return withAccessors(property, [&](auto get, auto set) {
                using T = decltype(get(from));
                const T                fromValue = blended(from, property, get(from));
                const std::optional<T> target    = Interpolation<T>::target(fromValue, blended(to, property, get(to)));
                if (!target) {
                    return false;
                }
                set(targets, *target);
                start = Interpolation<T>::channels(fromValue);
                end   = Interpolation<T>::channels(*target);
                return true;
            });
        }

        /**
         * Set a property from channels, on its way to its value in `targets`
         */
        void setChannels(Style &style, const Property property, const Channels &channels, const Style &targets) {
            withAccessors(property, [&](auto get, auto set) {
                set(style, Interpolation<decltype(get(targets))>::value(channels, get(targets)));
                return true;
            });
        }
    }

    // endregion

    // region Div

    Style Div::targetStyle() const {
        Style target = _computedStyle;
        if (_transitions) {
            for (const auto &r: _transitions->running) {
                if (r.targetSet) {
                    target.overlay(_transitions->targets, PropertyMask{}.set(index(r.property)));
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
                const Timing *timing = transition.timing(property);
                Channels     painted{};
                Channels     target{};
                if (!timing || timing->instant()
                    || !aim(property, _computedStyle, computed, _transitions->targets, painted, target)) {
                    it = running.erase(it); // The new value, at once
                    continue;
                }
                const bool set = computed.has(property);
                if (set == it->targetSet && target == it->tween.target()) {
                    // The same end: it goes on
                } else {
                    it->tween.retarget(target, *timing, start);
                    it->targetSet = set;
                }
                setChannels(computed, property, it->tween.value(), _transitions->targets);
                ++it;
            }
        }

        // The new ones
        if (!transition.empty()) {
            changed &= transition.properties();
            if (changed.any() && !_transitions) {
                _transitions = std::make_shared<detail::StyleTransitions>(this);
            }
            for (std::size_t i = 0; changed.any() && i < propertyCount; ++i) {
                if (!changed.test(i)) {
                    continue;
                }
                changed.reset(i);
                const auto   property = static_cast<Property>(i);
                const Timing *timing  = transition.timing(property);
                Channels     from{};
                Channels     to{};
                if (!timing || timing->instant()
                    || !aim(property, _computedStyle, computed, _transitions->targets, from, to) || from == to) {
                    continue;
                }
                detail::RunningTransition &r = _transitions->running.emplace_back(
                    detail::RunningTransition{property, Tween{from}, computed.has(property)}
                );
                r.tween.retarget(to, *timing, start);
                setChannels(computed, property, from, _transitions->targets);
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
                setChannels(_computedStyle, it->property, it->tween.value(), _transitions->targets);
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
            std::ranges::transform(string, string.begin(), ::tolower);
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
        return transitioning() || std::ranges::any_of(_children, [](const auto &child) {
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
