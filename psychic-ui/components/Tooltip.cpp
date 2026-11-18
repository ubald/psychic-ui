#include <algorithm>
#include <string>
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "Label.hpp"
#include "Tooltip.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Popup.hpp"

namespace psychic_ui {

    Tooltip::Tooltip(const std::string &text) :
        Popup() {
        tag("Tooltip");
        side(Side::Top);
        sideOffset(6);
        dismissOnEscape(false); // The window hides it on any key; Escape stays for what is under it
        restoreFocus(false);
        mouseEnabled(false);
        mouseChildren(false);

        _label = &add<Label>(text);
        _label->addClassName("text");
    }

    void Tooltip::draw(SkCanvas *canvas) {
        Div::draw(canvas);

        // The arrow, on the side facing the anchor, at the anchor's middle (kept off the rounded corners): a triangle
        // in the background color over the border, its two outer edges in the border color
        const auto  width   = static_cast<float>(getWidth());
        const auto  height  = static_cast<float>(getHeight());
        const float half    = arrowSize + 1.0f;
        const float corner  = std::min(_computedStyle.borderRadius(), std::min(width, height) / 2.0f) + half;
        const float border  = _computedStyle.border();
        const float anchorX = std::clamp(_anchorRect.centerX() - static_cast<float>(x()), corner, std::max(corner, width - corner));
        const float anchorY = std::clamp(_anchorRect.centerY() - static_cast<float>(y()), corner, std::max(corner, height - corner));

        SkPoint base1{};
        SkPoint tip{};
        SkPoint base2{};
        switch (_placedSide) {
            case Side::Top: // Above the anchor: the arrow under the box
                base1 = {anchorX - half, height - border};
                tip   = {anchorX, height + arrowSize};
                base2 = {anchorX + half, height - border};
                break;
            case Side::Bottom:
                base1 = {anchorX - half, border};
                tip   = {anchorX, -arrowSize};
                base2 = {anchorX + half, border};
                break;
            case Side::Left: // Left of the anchor: the arrow right of the box
                base1 = {width - border, anchorY - half};
                tip   = {width + arrowSize, anchorY};
                base2 = {width - border, anchorY + half};
                break;
            case Side::Right:
                base1 = {border, anchorY - half};
                tip   = {-arrowSize, anchorY};
                base2 = {border, anchorY + half};
                break;
        }

        // `draw` paints in the parent's coordinates
        canvas->save();
        canvas->translate(static_cast<float>(x()), static_cast<float>(y()));

        SkPaint paint{};
        paint.setAntiAlias(true);
        paint.setColor(_computedStyle.backgroundColor().argb());
        canvas->drawPath(SkPathBuilder().moveTo(base1).lineTo(tip).lineTo(base2).close().detach(), paint);

        if (border > 0) {
            paint.setColor(_computedStyle.borderColor().argb());
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(border);
            canvas->drawPath(SkPathBuilder().moveTo(base1).lineTo(tip).lineTo(base2).detach(), paint);
        }
        canvas->restore();
    }
}
