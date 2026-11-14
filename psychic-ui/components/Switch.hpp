#pragma once

#include <functional>
#include <string>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "psychic-ui/signals/Slot.hpp"

namespace psychic_ui {
    class Switch;

    class SwitchSkin : public Skin<Switch> {
    public:
        SwitchSkin() : Skin<Switch>() {
            tag("SwitchSkin");
        }

        virtual void setLabel(const std::string &/*label*/) {};

        /**
         * The thumb moved (`Switch::thumbPosition()`, 0 off, 1 on, between while it is dragged)
         */
        virtual void setThumbPosition(float /*position*/) {};

        /**
         * The thumb position (0 off, 1 on, or beyond, which the switch clamps) that puts the thumb's center at this
         * point, in the skin's coordinates. The switch handles the mouse; the skin only knows where its thumb goes.
         */
        virtual float positionAt(int x, int y) const = 0;
    };

    /**
     * @class Switch
     *
     * An on/off toggle, its thumb sliding along a track: a click, Space or Enter flips it, and the thumb can be
     * dragged, landing on the nearer side. It moves instantly: there is no animation mechanism yet.
     * It has the `checked` class while it is on (`Switch.checked .track`).
     */
    class Switch : public Component<SwitchSkin> {
    public:
        explicit Switch(const std::string &label = "", std::function<void(bool)> onChangeCallback = nullptr);

        explicit Switch(std::function<void(bool)> onChangeCallback) : Switch("", std::move(onChangeCallback)) {}

        const std::string &label() const;

        template<class Self>
        Self &label(this Self &self, const std::string &label) {
            static_cast<Switch &>(self).updateLabel(label);
            return self;
        }

        /**
         * Whether it is on
         */
        bool checked() const;

        template<class Self>
        Self &checked(this Self &self, detail::BoolArg checked) {
            static_cast<Switch &>(self).updateChecked(checked.value);
            return self;
        }

        /**
         * Where the thumb is: 0 off, 1 on, between while it is dragged
         */
        float thumbPosition() const;

        /**
         * Whether the thumb is being dragged
         */
        bool dragging() const { return _dragging; }

        bool active() const override;

        /**
         * How far (window units) the mouse moves from the press before it drags the thumb instead of clicking
         */
        static constexpr int dragThreshold = 3;

        /**
         * The state changed, with the new state
         */
        Signal<bool> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        std::string _label{};
        bool        _checked{false};
        bool        _pressed{false};  // The left button went down on us, and may drag
        bool        _dragging{false}; // Past the threshold: a release commits the thumb's side, not a click
        int         _pressX{0};       // Where it went down, in window coordinates
        int         _pressY{0};
        float       _pressPosition{0.0f}; // The skin's position under the press
        float       _dragPosition{0.0f};
        ScopedSlot  _onMouseMove{}; // On the window, only while pressed

        void updateLabel(const std::string &label);
        void updateChecked(bool checked);
        void dragTo(int windowX, int windowY);
        void endDrag();
        void skinChanged() override;
        void removedFromRender() override;
    };
}
