#include "SliderRangeSkin.hpp"

namespace psychicui {
    SliderRangeSkin::SliderRangeSkin() :
        RangeSkin() {
        setTag("Slider");

        auto container = add<Div>();
        container
            ->style()
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f);

        _track = container->add<Div>();
        _track
            ->onMouseDown(
                [this](const int mouseX, const int mouseY, int button, int modifiers) {
                    sendMouseValue(mouseX, mouseY);
                    _dragging = true;
                }
            )
            ->onMouseUp(
                [this](const int mouseX, const int mouseY, int button, int modifiers) {
                    _dragging = false;
                }
            )
            ->onMouseMove(
                [this](const int mouseX, const int mouseY, int button, int modifiers) {
                    if (_dragging) {
                        sendMouseValue(mouseX, mouseY);
                    }
                }
            )
            ->setClassNames({"track"})
            ->style()
            ->set(position, "absolute")
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f);

        _range = container->add<Div>();
        _range
            ->setClassNames({"range"})
            ->style()
            ->set(position, "absolute");

        auto label = add<Div>();
        label
            ->style()
            ->set(position, "absolute")
            ->set(alignItems, "center")
            ->set(justifyContent, "center")
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f);

        _value = label->add<Label>();
        _value
            ->style();
    }

    void SliderRangeSkin::added() {
        setValue(component() ? component()->getLinearPercentage() : 0.0f);
    }

    void SliderRangeSkin::styleUpdated() {
        Div::styleUpdated();
        if (_computedStyle->get(orientation) == "vertical") {
            // TODO: This creates another invalidation cycle, fix
            addClassName("vertical");
            removeClassName("horizontal");
            _range
                ->style()
                ->set(left, 0.f)
                ->set(right, 0.f)
                ->set(top, NAN)
                ->set(bottom, 0.f);
            _value->setVisible(false);
        } else {
            // TODO: This creates another invalidation cycle, fix
            addClassName("horizontal");
            removeClassName("vertical");
            _range
                ->style()
                ->set(left, 0.f)
                ->set(right, NAN)
                ->set(top, 0.f)
                ->set(bottom, 0.f);
            _value->setVisible(true);
        }
    }

    void SliderRangeSkin::setValue(const float value) {
        if (value >= 0.5f) {
            addClassName("inverted");
        } else {
            removeClassName("inverted");
        }

        _value->setText(component()->valueString());

        if (_computedStyle->get(orientation) == "vertical") {
            _range->style()->set(widthPercent, 1.0f);
            _range->style()->set(heightPercent, value);
        } else {
            _range->style()->set(widthPercent, value);
            _range->style()->set(heightPercent, 1.0f);
        }
    }

    void SliderRangeSkin::sendMouseValue(const int x, const int y) {
        if (_computedStyle->get(orientation) == "vertical") {
            component()->setLinearPercentage(1.0f - ((float) y / (float) _track->getHeight()));
        } else {
            component()->setLinearPercentage((float) x / (float) _track->getWidth());
        }
    }

}