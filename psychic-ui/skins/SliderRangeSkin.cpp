#include "SliderRangeSkin.hpp"
#include "DefaultSkin.hpp"
#include "../Window.hpp"

namespace psychic_ui {
    SliderRangeSkin::SliderRangeSkin() :
        RangeSkin() {
        setTag("Slider");
        addClassName("defaultSkinChrome");

        onMouseDown(
            [this](const int mouseX, const int mouseY, int button, int modifiers) {
                sendMouseValue(mouseX, mouseY);
                _onMouseMove = window()->onMouseMove(
                    [this](const int mouseX, const int mouseY, int button, int modifiers) {
                        int lx = 0;
                        int ly = 0;
                        globalToLocal(lx, ly, mouseX, mouseY);
                        sendMouseValue(lx, ly);
                    }
                );
            }
        );

        onMouseUp(
            [this](const int mouseX, const int mouseY, int button, int modifiers) {
                if (_onMouseMove) {
                    _onMouseMove->disconnect();
                    _onMouseMove = nullptr;
                }
            }
        );

        auto label = add<Div>();
        label
            ->setMouseEnabled(false)
            ->setMouseChildren(false)
            ->style()
            ->set(position, "absolute")
            ->set(alignItems, "center")
            ->set(justifyContent, "center")
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f);

        _valueLabel = label->add<Label>();
        _valueLabel
            ->style();
    }

    void SliderRangeSkin::draw(SkCanvas *canvas) {
        // Don't use default rendering, render out custom component chrome
        SkRRect inner = default_skin::draw(this, canvas);

        bool aa = _computedStyle->get(antiAlias);
        canvas->clipRRect(inner, aa);

        SkPaint paint{};
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(_computedStyle->get(backgroundColor));
        paint.setAntiAlias(aa);

        if (_computedStyle->get(orientation) == "vertical") {
            float h = std::round((_height - 2 * default_skin::padding) * _value);
            canvas->drawRect(
                SkRect{
                    default_skin::padding,
                    (_height - h - default_skin::padding),
                    static_cast<float>(_width) - default_skin::padding,
                    _height - default_skin::padding
                },
                paint
            );
        } else {
            canvas->drawRect(
                SkRect{
                    default_skin::padding,
                    default_skin::padding,
                    default_skin::padding + std::round((_width - 2 * default_skin::padding) * _value),
                    static_cast<float>(_height) - default_skin::padding
                },
                paint
            );
        }
    }

    void SliderRangeSkin::added() {
        setValue(component() ? component()->getLinearPercentage() : 0.0f);
    }

    void SliderRangeSkin::styleUpdated() {
        RangeSkin::styleUpdated();
        if (_computedStyle->get(orientation) == "vertical") {
            addClassName("vertical");
            removeClassName("horizontal");
            _valueLabel->setVisible(false);
        } else {
            addClassName("horizontal");
            removeClassName("vertical");
            _valueLabel->setVisible(true);
        }
    }

    void SliderRangeSkin::setValue(const float value) {
        _value = value;
        if (_value >= 0.5f) {
            addClassName("inverted");
            removeClassName("normal");
        } else {
            addClassName("normal");
            removeClassName("inverted");
        }
        _valueLabel->setText(component()->valueString());
    }

    void SliderRangeSkin::sendMouseValue(const int x, const int y) {
        if (_computedStyle->get(orientation) == "vertical") {
            component()->setLinearPercentage(
                1.0f - (
                    (static_cast<float>(y) - default_skin::padding) /
                    (static_cast<float>(_height) - 2 * default_skin::padding)
                ));
        } else {
            component()->setLinearPercentage(
                (static_cast<float>(x) - default_skin::padding) /
                (static_cast<float>(_width) - 2 * default_skin::padding)
            );
        }
    }
}
