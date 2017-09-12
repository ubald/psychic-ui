#include "TitleBar.hpp"
#include "Button.hpp"
#include "../Window.hpp"

namespace psychicui {

    TitleBar::TitleBar() :
        Div() {
        setTag("TitleBar");
        _defaultStyle
            ->set(direction, "row")
            ->set(shrink, 0);

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

        onMouseDown.subscribe(
            [this](int mouseX, int mouseY, int button, int modifiers) {
                window()->startDrag();
            }
        );

        onMouseUp.subscribe(
            [this](int mouseX, int mouseY, int button, int modifiers) {
                window()->stopDrag();
            }
        );
    };

    void TitleBar::addedToRender() {
        Div::addedToRender();
        // TODO: Allow updates to title
        _label->setText(window()->title());
    }

}
