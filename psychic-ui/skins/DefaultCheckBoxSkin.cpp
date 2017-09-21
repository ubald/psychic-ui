#include "DefaultCheckBoxSkin.hpp"
#include "DefaultSkin.hpp"

namespace psychic_ui {
    DefaultCheckBoxSkin::DefaultCheckBoxSkin() :
        CheckBoxSkin() {
        setTag("DefaultCheckBoxSkin");

        _box   = add<Shape>(
            [this](Shape *shape, SkCanvas *canvas) {
                SkRRect inner = drawDefaultSkinChrome(
                    shape->computedStyle()->get(width),
                    shape->computedStyle()->get(height),
                    shape->computedStyle(),
                    canvas
                );

                SkPaint paint{};
                paint.setStyle(SkPaint::kFill_Style);
                paint.setColor(shape->computedStyle()->get(backgroundColor));
                paint.setAntiAlias(shape->computedStyle()->get(antiAlias));
                canvas->drawRRect(inner, paint);
            }
        );
        _label = add<Label>();
    }

    void DefaultCheckBoxSkin::setLabel(const std::string &label) {
        _label->setText(label);
    }

    void DefaultCheckBoxSkin::draw(SkCanvas *canvas) {
        // noop
    }

}
