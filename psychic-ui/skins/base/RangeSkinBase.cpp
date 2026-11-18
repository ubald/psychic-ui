#include "RangeSkinBase.hpp"

namespace psychic_ui {
    RangeSkinBase::RangeSkinBase() :
        RangeSkin() {
        tag("RangeSkinBase");
        _track = &add<Div>();
        _track->addClassName("track")
              .mouseEnabled(false);

        _fill = &_track->add<Div>();
        _fill->addClassName("fill")
             .mouseEnabled(false)
             .position(Position::Absolute);

        // A point: the thumb, bigger, overflows it evenly on every side
        _thumbAnchor = &_track->add<Div>();
        _thumbAnchor->addClassName("thumb-anchor")
                    .mouseEnabled(false)
                    .position(Position::Absolute)
                    .width(0)
                    .height(0)
                    .alignItems(Align::Center)
                    .justify(Justify::Center);

        _thumb = &_thumbAnchor->add<Div>();
        _thumb->addClassName("thumb")
              .mouseEnabled(false)
              .shrink(0);

        _valueLabel = &add<Label>();
        _valueLabel->addClassName("value")
                   .mouseEnabled(false);

        placeParts();
    }

    void RangeSkinBase::added() {
        setValue(component() ? component()->getLinearPercentage() : 0.0f);
    }

    void RangeSkinBase::styleUpdated() {
        RangeSkin::styleUpdated();
        const bool vertical = _computedStyle.orientation() == Orientation::Vertical;
        if (vertical) {
            addClassName("vertical");
            removeClassName("horizontal");
        } else {
            addClassName("horizontal");
            removeClassName("vertical");
        }
        if (vertical != _vertical) {
            _vertical = vertical;
            placeParts();
        }
    }

    void RangeSkinBase::setValue(const float value) {
        _value = value;
        _valueLabel->text(component() ? component()->valueString() : "");
        placeParts();
        valueChanged();
    }

    void RangeSkinBase::placeParts() {
        const Length at = Length::percent(_value * 100.0f);
        if (_vertical) {
            _fill->left(0).right(0).bottom(0).top(Auto).width(Auto).height(at);
            _thumbAnchor->left(50_pct).bottom(at).top(Auto);
        } else {
            _fill->left(0).top(0).bottom(0).right(Auto).height(Auto).width(at);
            _thumbAnchor->left(at).top(50_pct).bottom(Auto);
        }
    }

    float RangeSkinBase::valueAt(const int x, const int y) const {
        // The track is our child: its position is in our coordinates
        if (_vertical) {
            const auto height = static_cast<float>(_track->getHeight());
            return height > 0 ? 1.0f - (static_cast<float>(y - _track->y()) / height) : 0.0f;
        }
        const auto width = static_cast<float>(_track->getWidth());
        return width > 0 ? static_cast<float>(x - _track->x()) / width : 0.0f;
    }

    void RangeSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void RangeSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
