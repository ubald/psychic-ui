#include "TitleBar.hpp"
#include "Button.hpp"
#include "../Window.hpp"

namespace psychic_ui {

    TitleBar::TitleBar() :
        Div() {
        tag("TitleBar");
        _defaultStyle.direction(Direction::Row)
            .shrink(0);

        add<Button>(
            "Close", [this]() {
                window()->close();
            }
        ).addClassName("close");

        add<Button>(
            "Minimize", [this]() {
                window()->toggleMinimized();
            }
        ).addClassName("minimize");

        add<Button>(
            "Maximize", [this]() {
                window()->toggleFullscreen();
            }
        ).addClassName("maximize");

        _label = &add<Label>().addClassName("title");

        onMouseDown(
            [this](int /*mouseX*/, int /*mouseY*/, int /*button*/, Mod /*modifiers*/) {
                window()->startDrag();
            }
        );

        onMouseUp(
            [this](int /*mouseX*/, int /*mouseY*/, int /*button*/, Mod /*modifiers*/) {
                window()->stopDrag();
            }
        );
    };

    void TitleBar::addedToRender() {
        Div::addedToRender();
        // Lets the window manager or compositor move the window from here (on Wayland too)
        window()->addDragArea(shared_from_this());
        // TODO: Allow updates to title
        _label->text(window()->getTitle());
    }

}
