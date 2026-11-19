
#include "TitleBar.hpp"
#include <memory>
#include <utility>
#include "WindowControls.hpp"
#include "WindowControlsStyle.hpp"
#include "../Window.hpp"
#include "../Div.hpp"
#include "Label.hpp"
#include "../style/Values.hpp"

namespace psychic_ui {

    TitleBar::TitleBar() :
        Div() {
        tag("TitleBar");
        _defaultStyle.direction(Direction::Row)
            .shrink(0);

        _controls = &add<WindowControls>();

        _label = &add<Label>().addClassName("title");
        placeControls();
        // Where the controls' style puts them: at the start for macOS's, at the end for Windows's and Linux's
        subscribeTo(_controls->changed, [this] { placeControls(); });

        onMousePress(
            [this] {
                window()->startDrag();
            }
        );

        onMouseRelease(
            [this] {
                window()->stopDrag();
            }
        );
    };

    void TitleBar::placeControls() {
        auto controls = std::static_pointer_cast<WindowControls>(_controls->shared_from_this());
        if (windowControlsLead(_controls->controlsStyle())) {
            add(0, std::move(controls));
        } else {
            add(std::move(controls));
        }
    }

    void TitleBar::addedToRender() {
        Div::addedToRender();
        // Lets the window manager or compositor move the window from here (on Wayland too)
        window()->addDragArea(shared_from_this());
        // TODO: Allow updates to title
        _label->text(window()->getTitle());
    }

}
