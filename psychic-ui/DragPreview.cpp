#include <cmath>
#include "components/Label.hpp"
#include "DragPayload.hpp"
#include "DragPreview.hpp"
#include "Popup.hpp"
#include "psychic-ui.hpp"

namespace psychic_ui {

    DragPreview::DragPreview(const DragPayload &payload) :
        Popup() {
        tag("DragPreview");
        dismissOnOutsidePress(false); // It ends with the drag
        dismissOnEscape(false);       // The window cancels the drag on Escape
        restoreFocus(false);
        mouseEnabled(false);
        mouseChildren(false);

        if (payload.preview) {
            add(payload.preview);
            _content = payload.preview.get();
        } else {
            _label = &add<Label>(payload.label);
            _label->addClassName("label");
            _content = _label;
        }
    }

    void DragPreview::updatePointer(const float x, const float y, const InputMode mode) {
        if (mode == InputMode::Touch) {
            side(Side::Top).alignment(PopupAlign::Center).sideOffset(fingerOffset).alignOffset(0);
        } else {
            side(Side::Bottom).alignment(PopupAlign::Start).sideOffset(pointerOffset).alignOffset(pointerOffset);
        }
        anchor(static_cast<int>(std::round(x)), static_cast<int>(std::round(y)));
    }
}
