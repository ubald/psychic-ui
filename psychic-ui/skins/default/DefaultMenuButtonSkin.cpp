#include "psychic-ui/skins/paint/Marks.hpp"
#include "DefaultMenuButtonSkin.hpp"

namespace psychic_ui {
    DefaultMenuButtonSkin::DefaultMenuButtonSkin() :
        MenuButtonSkinBase() {
        tag("DefaultMenuButtonSkin");
    }

    DefaultSubMenuButtonSkin::DefaultSubMenuButtonSkin() :
        DefaultMenuButtonSkin() {
        _arrow = &add<Shape>(
            [](Shape *shape, SkCanvas *canvas) {
                paint::arrow(
                    canvas,
                    SkRect::MakeWH(static_cast<float>(shape->getWidth()), static_cast<float>(shape->getHeight())),
                    shape->computedStyle().color()
                );
            }
        );
        _arrow->addClassName("arrow");
    }
}
