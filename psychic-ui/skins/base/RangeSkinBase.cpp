#include "RangeSkinBase.hpp"

namespace psychic_ui {
    RangeSkinBase::RangeSkinBase() :
        RangeSkin() {
        auto &center = add<Div>()
            .mouseEnabled(false)
            .mouseChildren(false)
            .position(Position::Absolute)
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .width(100_pct)
            .height(100_pct);

        _valueLabel = &center.add<Label>();
        _valueLabel->addClassName("value");
    }

    void RangeSkinBase::added() {
        setValue(component() ? component()->getLinearPercentage() : 0.0f);
    }

    void RangeSkinBase::styleUpdated() {
        RangeSkin::styleUpdated();
        if (_computedStyle.orientation() == Orientation::Vertical) {
            addClassName("vertical");
            removeClassName("horizontal");
        } else {
            addClassName("horizontal");
            removeClassName("vertical");
        }
    }

    void RangeSkinBase::setValue(const float value) {
        _value = value;
        _valueLabel->text(component()->valueString());
        valueChanged();
    }

    void RangeSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void RangeSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
