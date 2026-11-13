#include <SkPaint.h>
#include <SkPath.h>
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
                SkPath path;
                path.moveTo(0, 0);
                path.lineTo(shape->getWidth(), shape->getHeight() / 2);
                path.lineTo(0, shape->getHeight());
                path.lineTo(0, 0);
                canvas->drawPath(path, paint);
            }
        );
        shape->addClassName("arrow");
    }
}
