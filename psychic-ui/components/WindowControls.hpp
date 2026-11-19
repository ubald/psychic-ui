#pragma once

#include "psychic-ui/Div.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "Button.hpp"
#include "WindowControlsStyle.hpp"

namespace psychic_ui {

    /**
     * @class WindowControls
     *
     * The buttons of an undecorated window: close, minimize and maximize (classes `close`, `minimize`,
     * `maximize`). Their look and order follow its `controlsStyle`, the platform's by default
     * (`platformWindowControlsStyle`): `Mac` orders them close, minimize, maximize, and maximize toggles full
     * screen; `Windows` orders them minimize, maximize, close, and maximize toggles the window's maximized state.
     * The style is a class on it (`mac`, `windows`) from which the skin set's rules pick the buttons' skin and
     * colors. A `TitleBar` and an `AppShell`'s sidebar header show them.
     */
    class WindowControls : public Div {
    public:
        WindowControls();

        Button &closeButton() const { return *_close; }
        Button &minimizeButton() const { return *_minimize; }
        Button &maximizeButton() const { return *_maximize; }

        /**
         * How the controls look and are ordered
         */
        WindowControlsStyle controlsStyle() const { return _controlsStyle; }

        /**
         * Give the controls another platform's look and order, whatever the platform
         */
        template<class Self>
        Self &controlsStyle(this Self &self, WindowControlsStyle style) {
            static_cast<WindowControls &>(self).updateControlsStyle(style);
            return self;
        }

        /**
         * The style changed, with the new style (a bar holding the controls moves them to its start or its end)
         */
        Signal<WindowControlsStyle> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        // Children: our child list owns them
        Button *_close{nullptr};
        Button *_minimize{nullptr};
        Button *_maximize{nullptr};

        WindowControlsStyle _controlsStyle{platformWindowControlsStyle};

        void updateControlsStyle(WindowControlsStyle style);
        void applyControlsStyle();
    };
}
