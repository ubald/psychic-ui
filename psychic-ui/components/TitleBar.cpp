
#include "TitleBar.hpp"
#include "WindowControls.hpp"
#include "../Window.hpp"

namespace psychic_ui {

    TitleBar::TitleBar() :
        Div() {
        tag("TitleBar");
        _defaultStyle.direction(Direction::Row)
            .shrink(0);

        _controls = &add<WindowControls>();

        _label = &add<Label>().addClassName("title");

        onMousePress(
            [this](int /*mouseX*/, int /*mouseY*/, int /*button*/, Mod /*modifiers*/) {
                window()->startDrag();
            }
        );

        onMouseRelease(
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
