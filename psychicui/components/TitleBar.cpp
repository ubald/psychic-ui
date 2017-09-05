#include "TitleBar.hpp"
#include "Button.hpp"
#include "../Window.hpp"

namespace psychicui {

    TitleBar::TitleBar() :
        Component() {
        setTag("TitleBar");
        _defaults->set(direction, "row");

        add<Button>(
            "Close", [this]() {
                window()->close();
            }
        )->addClassName("close");

        add<Button>(
            "Minimize", [this]() {
                window()->toggleMinimized();
            }
        )->addClassName("minimize");

        add<Button>(
            "Maximize", [this]() {
                window()->toggleFullscreen();
            }
        )->addClassName("maximize");

        _label = add<Label>();
        _label->addClassName("title");
    };

    void TitleBar::addedToRender() {
        Component::addedToRender();
        // TODO: Allow updates to title
        _label->setText(window()->title());
    }

    void TitleBar::onMouseDown(int mouseX, int mouseY, int button, int modifiers) {
        Component::onMouseDown(mouseX, mouseY, button, modifiers);
        window()->startDrag();
        _children[0]->computedStyle()->trace();
    }

    void TitleBar::onMouseUp(int mouseX, int mouseY, int button, int modifiers) {
        Component::onMouseUp(mouseX, mouseY, button, modifiers);
        window()->stopDrag();
    }
}