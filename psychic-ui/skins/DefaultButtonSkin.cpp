#include "DefaultButtonSkin.hpp"
#include "DefaultSkin.hpp"

namespace psychic_ui {
    DefaultButtonSkin::DefaultButtonSkin() :
        DefaultBasicButtonSkin() {
        setTag("DefaultButtonSkin");
        addClassName("defaultSkinChrome");
    }

    void DefaultButtonSkin::draw(SkCanvas *canvas) {
        // Don't use default rendering, render out custom component chrome
        SkRRect inner = default_skin::draw(this, canvas);

        // Add the button's surface
        SkPaint paint{};
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(_computedStyle->get(backgroundColor));
        paint.setAntiAlias(_computedStyle->get(antiAlias));
        canvas->drawRRect(inner, paint);
    }
}
