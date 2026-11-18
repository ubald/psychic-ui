#include "WindowControls.hpp"
#include "Button.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    WindowControls::WindowControls() :
        Div() {
        tag("WindowControls");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center)
            .shrink(0);

        // The pointer's, not the keyboard's: Tab skips them
        _close = &add<Button>("Close", [this]() { window()->close(); })
            .addClassName("close")
            .focusEnabled(false);

        _minimize = &add<Button>("Minimize", [this]() { window()->toggleMinimized(); })
            .addClassName("minimize")
            .focusEnabled(false);

        _maximize = &add<Button>("Maximize", [this]() { window()->toggleFullscreen(); })
            .addClassName("maximize")
            .focusEnabled(false);
    }

}
