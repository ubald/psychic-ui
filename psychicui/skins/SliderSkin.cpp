#include "SliderSkin.hpp"
#include "../components/Slider.hpp"

namespace psychicui {
    SliderSkin::SliderSkin() :
        Skin() {

        auto container = add<Component>();
        container
            ->style()
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f);

        _track = container->add<Component>();
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
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f);

        _range = container->add<Component>();
        _range
            ->setClassNames({"range"})
            ->style()
            ->set(top, 0.f)
            ->set(bottom, 0.f)
            ->set(position, "absolute");

        auto label = add<Component>();
        label
            ->style()
            ->set(position, "absolute")
            ->set(alignItems, "center")
            ->set(justifyContent, "center")
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f);

        _value = label->add<Label>();
        _value
            ->style()
            ;
    }

    void SliderSkin::componentChanged() {
            setValue(_component ? _component->getValuePercent() : 0.0f);
        }

    void SliderSkin::setValue(const float value) {
        if (value >= 0.5f) {
            _component->addClassName("inverted");
        } else {
            _component->removeClassName("inverted");
        }
        _value->setText(_component->valueString());
        _range->style()->set(widthPercent, value);
    }

    void SliderSkin::sendMouseValue(const int x, const int y) {
        _component->setValuePercent((float) x / (float) _track->getWidth());
    }

}