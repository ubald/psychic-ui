#pragma once

#include "psychic-ui/Div.hpp"
#include "Button.hpp"

namespace psychic_ui {

    /**
     * @class WindowControls
     *
     * The buttons of an undecorated window: close, minimize and maximize (classes `close`, `minimize`,
     * `maximize`), in the "title-bar" skin variant. A `TitleBar` and an `AppShell`'s sidebar header show them.
     */
    class WindowControls : public Div {
    public:
        WindowControls();

        Button &closeButton() const { return *_close; }
        Button &minimizeButton() const { return *_minimize; }
        Button &maximizeButton() const { return *_maximize; }

    protected:
        // Children: our child list owns them
        Button *_close{nullptr};
        Button *_minimize{nullptr};
        Button *_maximize{nullptr};
    };
}
