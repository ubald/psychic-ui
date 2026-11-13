#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include <psychic-ui/components/Menu.hpp>
#include "DefaultMenuButtonSkin.hpp"

namespace psychic_ui {
    DefaultMenuButtonSkin::DefaultMenuButtonSkin() :
        MenuButtonSkin() {
        tag("DefaultMenuButtonSkin");

        label = &add<Label>();
        label->addClassName("label");
        shortcut = &add<Label>();
        shortcut->addClassName("shortcut");
    }

    void DefaultMenuButtonSkin::added() {
        label->text(component()->menuItem()->label);
        shortcut->text(component()->menuItem()->shortcut);
    }

    DefaultSubMenuButtonSkin::DefaultSubMenuButtonSkin() :
        DefaultMenuButtonSkin() {

        shape = &add<Shape>(
            [](Shape *shape, SkCanvas *canvas) {
                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setStyle(SkPaint::kFill_Style);
                paint.setColor(shape->computedStyle().color().argb());
                SkPath path = SkPathBuilder()
                    .moveTo(0, 0)
                    .lineTo(shape->getWidth(), shape->getHeight() / 2)
                    .lineTo(0, shape->getHeight())
                    .close()
                    .detach();
                canvas->drawPath(path, paint);
            }
        );
        shape->addClassName("arrow");
    }
}
