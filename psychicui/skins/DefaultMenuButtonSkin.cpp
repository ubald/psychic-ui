#include <SkPaint.h>
#include <SkPath.h>
#include <psychicui/components/Menu.hpp>
#include "DefaultMenuButtonSkin.hpp"

namespace psychicui {
    DefaultMenuButtonSkin::DefaultMenuButtonSkin() :
        MenuButtonSkin() {
        setTag("DefaultMenuButtonSkin");

        label = add<Label>();
        label->addClassName("label");
        shortcut = add<Label>();
        shortcut->addClassName("shortcut");
    }

    void DefaultMenuButtonSkin::added() {
        label->setText(component()->menuItem()->label);
        shortcut->setText(component()->menuItem()->shortcut);
    }

    DefaultSubMenuButtonSkin::DefaultSubMenuButtonSkin() :
        DefaultMenuButtonSkin() {

        shape = add<Shape>(
            [](Shape *shape, SkCanvas *canvas) {
                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setStyle(SkPaint::kFill_Style);
                paint.setColor(shape->computedStyle()->get(color));
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
