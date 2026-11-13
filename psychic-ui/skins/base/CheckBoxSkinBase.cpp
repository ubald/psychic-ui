#include "CheckBoxSkinBase.hpp"

namespace psychic_ui {
    CheckBoxSkinBase::CheckBoxSkinBase() :
        CheckBoxSkin() {
        _box = &add<Shape>(
            [this](Shape *shape, SkCanvas *canvas) {
                drawChrome(canvas, paint::Box::of(*shape), SkinState::of(component()));
            }
        );
        _box->addClassName("box");
        _label = &add<Label>();
        _label->addClassName("label");
    }

    void CheckBoxSkinBase::setLabel(const std::string &label) {
        _label->text(label);
    }

    void CheckBoxSkinBase::draw(SkCanvas */*canvas*/) {}

    void CheckBoxSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState /*state*/) {
        const Style &style = _box->computedStyle();
        paint::fill(canvas, box.outer, style.backgroundColor(), style.antiAlias());
        if (box.border > 0) {
            paint::stroke(canvas, box.strokeLine(), style.borderColor(), box.border, style.antiAlias());
        }
    }
}
