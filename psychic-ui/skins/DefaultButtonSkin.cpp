#include "DefaultButtonSkin.hpp"
#include "DefaultSkin.hpp"

namespace psychic_ui {
    DefaultButtonSkin::DefaultButtonSkin() :
        DefaultBasicButtonSkin() {
        setTag("DefaultButtonSkin");
    }

    void DefaultButtonSkin::draw(SkCanvas *canvas) {
        SkRRect inner = drawDefaultSkinChrome(
            _width,
            _height,
            _computedStyle.get(),
            canvas
        );

        SkPaint paint{};
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(_computedStyle->get(backgroundColor));
        paint.setAntiAlias(_computedStyle->get(antiAlias));
        canvas->drawRRect(inner, paint);
    }
}
