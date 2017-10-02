#include "DefaultCheckBoxSkin.hpp"
#include "DefaultSkin.hpp"

namespace psychic_ui {
    DefaultCheckBoxSkin::DefaultCheckBoxSkin() :
        CheckBoxSkin() {
        setTag("DefaultCheckBoxSkin");

        _box   = add<Shape>(
            [this](Shape *shape, SkCanvas *canvas) {
                SkRRect inner = default_skin::draw(shape, canvas);

                if (component()->mouseOver() || component()->checked() ) {
                    SkPaint paint{};
                    paint.setStyle(SkPaint::kFill_Style);
                    paint.setColor(shape->computedStyle()->get(backgroundColor));
                    paint.setAntiAlias(shape->computedStyle()->get(antiAlias));
                    canvas->drawRRect(inner, paint);
                }
            }
        );
        _box->addClassName("defaultSkinChrome");
        _label = add<Label>();
    }

    void DefaultCheckBoxSkin::setLabel(const std::string &label) {
        _label->setText(label);
    }

    void DefaultCheckBoxSkin::draw(SkCanvas */*canvas*/) {
        // Don't use default rendering, render out custom component chrome
        // in this case, the chrome is drawn in the shape child because it
        // has a different size than the component.
    }

}
