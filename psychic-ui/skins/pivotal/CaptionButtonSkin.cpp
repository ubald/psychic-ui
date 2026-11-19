#include <array>
#include <cmath>
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "CaptionButtonSkin.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/components/Button.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "psychic-ui/paint/Crisp.hpp"
#include "psychic-ui/style/Color.hpp"
#include "psychic-ui/style/Length.hpp"

namespace psychic_ui {
    namespace {
        // The grid icons' stroke widths are given in
        constexpr float iconGrid = 16.0f;

        // How far restore's back square sits from its front one, a share of the glyph
        constexpr float restoreOffset = 0.2f;
    }

    CaptionButtonSkin::CaptionButtonSkin() :
        ButtonSkin() {
        tag("CaptionButtonSkin");
        _inlineStyle.width(Full)
            .height(Full);
    }

    void CaptionButtonSkin::draw(SkCanvas *canvas) {
        const Button *button = component();
        if (!button) {
            return;
        }
        const Style &style = button->computedStyle();

        const Color background = style.backgroundColor();
        if (background.alpha() > 0) {
            paint::fill(canvas, paint::snap(canvas, paint::Box::of(*button).outer), background, style.antiAlias());
        }

        // The glyph's square, centered, on device pixels
        const float  size   = _computedStyle.iconSize();
        const SkRect bounds = SkRect::MakeWH(static_cast<float>(_width), static_cast<float>(_height));
        const SkRect square = paint::snap(
            canvas,
            SkRRect::MakeRect(SkRect::MakeXYWH(bounds.centerX() - (size / 2.0f), bounds.centerY() - (size / 2.0f), size, size))
        ).rect();
        const float stroke = _computedStyle.iconStrokeWidth() * size / iconGrid;
        const Color color  = _computedStyle.color();

        if (button->classNames().contains("close")) {
            drawClose(canvas, square, stroke, color);
        } else if (button->classNames().contains("minimize")) {
            drawMinimize(canvas, square, stroke, color);
        } else if (button->classNames().contains("maximize")) {
            const Window *window = button->window();
            if (window && window->maximized()) {
                drawRestore(canvas, square, stroke, color);
            } else {
                drawMaximize(canvas, square, stroke, color);
            }
        }
    }

    void CaptionButtonSkin::drawMinimize(SkCanvas *canvas, const SkRect &glyph, const float stroke, const Color color) {
        // From the middle down, a whole number of device pixels thick
        const float  width = paint::crispWidth(canvas, stroke);
        const SkRect line  = SkRect::MakeLTRB(glyph.left(), glyph.centerY(), glyph.right(), glyph.centerY() + width);
        paint::fill(canvas, paint::snap(canvas, SkRRect::MakeRect(line)), color);
    }

    void CaptionButtonSkin::drawMaximize(SkCanvas *canvas, const SkRect &glyph, const float stroke, const Color color) {
        const std::array outline{paint::Ring{.color = color, .width = stroke}};
        paint::rings(canvas, SkRRect::MakeRect(glyph), outline);
    }

    void CaptionButtonSkin::drawRestore(SkCanvas *canvas, const SkRect &glyph, const float stroke, const Color color) {
        // The front square at the bottom left, the back one up and to the right, hidden where the front one is
        const float  offset = std::round(glyph.width() * restoreOffset);
        const SkRect front  = SkRect::MakeLTRB(glyph.left(), glyph.top() + offset, glyph.right() - offset, glyph.bottom());
        const SkRect back   = SkRect::MakeLTRB(glyph.left() + offset, glyph.top(), glyph.right(), glyph.bottom() - offset);
        const std::array outline{paint::Ring{.color = color, .width = stroke}};

        canvas->save();
        canvas->clipRect(paint::snap(canvas, SkRRect::MakeRect(front)).rect(), SkClipOp::kDifference);
        paint::rings(canvas, SkRRect::MakeRect(back), outline);
        canvas->restore();
        paint::rings(canvas, SkRRect::MakeRect(front), outline);
    }

    void CaptionButtonSkin::drawClose(SkCanvas *canvas, const SkRect &glyph, const float stroke, const Color color) {
        // Two diagonals, their ends half a stroke in so the cross fills the square
        const float  width = paint::crispWidth(canvas, stroke);
        const SkRect inner = glyph.makeInset(width / 2.0f, width / 2.0f);
        SkPathBuilder cross;
        cross.moveTo(inner.left(), inner.top()).lineTo(inner.right(), inner.bottom());
        cross.moveTo(inner.right(), inner.top()).lineTo(inner.left(), inner.bottom());

        SkPaint paint{};
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(width);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setColor(color.argb());
        canvas->drawPath(cross.detach(), paint);
    }

}
