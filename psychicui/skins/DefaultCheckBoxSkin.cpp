#include "DefaultCheckBoxSkin.hpp"

namespace psychicui {
    DefaultCheckBoxSkin::DefaultCheckBoxSkin() :
        CheckBoxSkin() {
        _box   = add<Shape>(
            [this](Shape *shape, SkCanvas *canvas) {
                float   radius = shape->computedStyle()->get(borderRadius);
                SkRRect rect;
                rect.setRectXY(
                    {
                        (float) shape->x(),
                        (float) shape->y(),
                        shape->x() + shape->computedStyle()->get(width),
                        shape->y() + shape->computedStyle()->get(height)
                    },
                    radius, radius
                );

                SkPaint paint;
                paint.setAntiAlias(shape->computedStyle()->get(antiAlias));
                paint.setStyle(SkPaint::kFill_Style);
                paint.setColor(shape->computedStyle()->get(backgroundColor));
                rect.inset(1.0f, 1.0f);
                canvas->drawRRect(rect, paint);

                paint.setStyle(SkPaint::kStroke_Style);
                paint.setColor(shape->computedStyle()->get(borderColor));
                rect.inset(-0.5f, -0.5f);
                canvas->drawRRect(rect, paint);

                if (component()->active()) {
                    paint.setStyle(SkPaint::kFill_Style);
                    paint.setColor(shape->computedStyle()->get(color));
                    rect.inset(1.5f, 1.5f);
                    canvas->drawRRect(rect, paint);
                }
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
