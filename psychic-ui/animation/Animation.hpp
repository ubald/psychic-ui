#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include "Timing.hpp"
#include "Tween.hpp"

namespace psychic_ui {
    class Div;
    class Window;

    /**
     * Something the window moves with time, from `Window::updateAnimations`. The window holds it weakly: whoever owns
     * it (a Div, an `Animated` member) ends it by letting it go.
     */
    class Animation {
    public:
        virtual ~Animation() = default;

        /**
         * Move to where it is at `now`
         * @return Whether it still moves; the window drops it otherwise
         */
        virtual bool advance(std::chrono::nanoseconds now) = 0;

        /**
         * Go to the end at once (reduced motion)
         */
        virtual void finish() = 0;

    private:
        friend class Window;
        Window *_scheduledIn{nullptr}; // The window ticking it
    };

    namespace detail {
        /**
         * A tween the window ticks, for `Animated` and `Div::animate`
         */
        class TweenAnimation : public Animation {
        public:
            Tween                      tween{};
            std::function<void(const Channels &)> update{}; // Called with each new value

            bool advance(std::chrono::nanoseconds now) override;
            void finish() override;
        };

        /**
         * Move a tween to `target` with `timing` in `owner`'s window, from the window's clock: at once when the owner
         * isn't in a window, the timing is instant or the window reduces motion
         */
        void startTween(Div *owner, const std::shared_ptr<TweenAnimation> &animation, const Channels &target,
                        const Timing &timing);
    }

    /**
     * A value that moves to its targets with a timing instead of jumping, for what a component draws itself (style
     * properties have `transition`). Keep it as a member of the div it belongs to:
     *
     *     Animated<float> _offset{this, 0.0f, Timing{200ms, Ease::Out}};
     *
     *     _offset.to(row.y());         // From where it is now
     *     float y = _offset.value();   // In draw()
     *
     * Its owner's window advances it every frame (`Window::updateAnimations`) and draws while it moves. It jumps when
     * its owner isn't in a window, or the window reduces motion. It ends with its owner, the window only holds it
     * weakly.
     */
    template<Interpolable T>
    class Animated {
    public:
        Animated(Div *owner, T value, Timing timing = {}) :
            _owner(owner), _timing(timing), _target(value),
            _animation(std::make_shared<detail::TweenAnimation>()) {
            _animation->tween = Tween{Interpolation<T>::channels(value), Interpolation<T>::count};
        }

        Animated(const Animated &) = delete;
        Animated &operator=(const Animated &) = delete;

        /**
         * Move to `target` from the current value (a spring keeps its velocity)
         */
        Animated &to(T target) {
            if (!Interpolation<T>::compatible(value(), target)) {
                return jump(target);
            }
            _target = target;
            detail::startTween(_owner, _animation, Interpolation<T>::channels(target), _timing);
            return *this;
        }

        /**
         * Set the value at once
         */
        Animated &jump(T value) {
            _target = value;
            _animation->tween.jump(Interpolation<T>::channels(value));
            return *this;
        }

        /**
         * How the next `to` moves (a theme's motion token, read in `styleUpdated`); a move in progress keeps its own
         */
        Animated &timing(Timing timing) {
            _timing = timing;
            return *this;
        }

        const Timing &timing() const { return _timing; }

        /**
         * Where it is, as of the window's last `updateAnimations`
         */
        T value() const { return Interpolation<T>::value(_animation->tween.value(), _target); }

        /**
         * Where it goes
         */
        const T &target() const { return _target; }

        /**
         * Whether it is on its way
         */
        bool animating() const { return _animation->tween.running(); }

    private:
        Div                                      *_owner;
        Timing                                   _timing;
        T                                        _target;
        std::shared_ptr<detail::TweenAnimation> _animation;
    };
}
