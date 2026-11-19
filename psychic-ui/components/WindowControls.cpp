#include "WindowControls.hpp"
#include <memory>
#include <string>
#include "Button.hpp"
#include "WindowControlsStyle.hpp"
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

        // macOS's green button goes full screen; Windows's and Linux's maximize the window
        _maximize = &add<Button>(
                "Maximize",
                [this]() {
                    if (_controlsStyle == WindowControlsStyle::Mac) {
                        window()->toggleFullscreen();
                    } else {
                        window()->toggleMaximized();
                    }
                }
            )
            .addClassName("maximize")
            .focusEnabled(false);

        applyControlsStyle();
    }

    void WindowControls::updateControlsStyle(const WindowControlsStyle style) {
        if (style == _controlsStyle) {
            return;
        }
        _controlsStyle = style;
        applyControlsStyle();
        changed.emit(style);
    }

    void WindowControls::applyControlsStyle() {
        for (const WindowControlsStyle other: allWindowControlsStyles) {
            if (other != _controlsStyle) {
                removeClassName(std::string{windowControlsStyleClass(other)});
            }
        }
        addClassName(std::string{windowControlsStyleClass(_controlsStyle)});

        // Moved in place, in the style's order
        const auto shared = [](Button *button) {
            return std::static_pointer_cast<Button>(button->shared_from_this());
        };
        if (_controlsStyle == WindowControlsStyle::Mac) {
            add(0, shared(_close));
            add(1, shared(_minimize));
            add(2, shared(_maximize));
        } else {
            add(0, shared(_minimize));
            add(1, shared(_maximize));
            add(2, shared(_close));
        }
    }

}
