#include "psychic-ui/paint/Icon.hpp"
#include "CheckBoxSkinBase.hpp"

namespace psychic_ui {
    CheckBoxSkinBase::CheckBoxSkinBase() :
        CheckBoxSkin() {
        tag("CheckBoxSkinBase");
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
        if (const CheckBox *check = component(); check && check->checked()) {
            // The mark is 2 px smaller than the box, as Pivotal draws it (12 px in 14), in the box's icon stroke
            const SkRect mark = box.outer.rect().makeInset(1.0f, 1.0f);
            paint::icon(canvas, "check", mark, style.color(), style.iconStrokeWidth());
        }
    }
}
