#include <SkPaint.h>
#include "TitleBarButtonSkin.hpp"

namespace psychic_ui {
    TitleBarButtonSkin::TitleBarButtonSkin() :
        ButtonSkin() {
        setTag("TitleBarButtonSkin");
        _inlineStyle.width(100_pct)
            .height(100_pct);
    }

    void TitleBarButtonSkin::draw(SkCanvas *canvas) {
        int hw = _width / 2;
        int hh = _height / 2;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        paint.setColor(0x10FFFFFF);
        canvas->drawCircle(hw, hh, hw, paint);

        paint.setColor(0xA0000000);
        canvas->drawCircle(hw, hh, hw - 1, paint);

        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(component()->computedStyle().backgroundColor().argb());
        canvas->drawCircle(hw, hh, hw - 2, paint);
    }

}
