#include "DefaultSliderSkin.hpp"
#include "DefaultChrome.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {
    using default_skins::chromeInset;

    DefaultSliderSkin::DefaultSliderSkin() :
        RangeSkinBase() {
        tag("DefaultSliderSkin");

        onMousePress(
            [this](const int mouseX, const int mouseY, MouseButton /*button*/, Mod /*modifiers*/) {
                sendMouseValue(mouseX, mouseY);
                _onMouseMove = window()->mouseMoved.subscribe(
                    [this](const int mouseX, const int mouseY, int /*buttons*/, Mod /*modifiers*/) {
                        int lx = 0;
                        int ly = 0;
                        globalToLocal(lx, ly, mouseX, mouseY);
                        sendMouseValue(lx, ly);
                    }
                );
            }
        );

        onMouseRelease(
            [this](const int /*mouseX*/, const int /*mouseY*/, MouseButton /*button*/, Mod /*modifiers*/) {
                _onMouseMove.disconnect();
            }
        );
    }

    void DefaultSliderSkin::drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState /*state*/) {
        const SkRRect inner = default_skins::drawChrome(canvas, box, _computedStyle);

        const bool aa = _computedStyle.antiAlias();
        canvas->clipRRect(inner, aa);

        if (_computedStyle.orientation() == Orientation::Vertical) {
            float h = std::round((_height - 2 * chromeInset) * _value);
            paint::fill(
                canvas,
                SkRect{
                    chromeInset,
                    (_height - h - chromeInset),
                    static_cast<float>(_width) - chromeInset,
                    _height - chromeInset
                },
                _computedStyle.backgroundColor(), aa
            );
        } else {
            paint::fill(
                canvas,
                SkRect{
                    chromeInset,
                    chromeInset,
                    chromeInset + std::round((_width - 2 * chromeInset) * _value),
                    static_cast<float>(_height) - chromeInset
                },
                _computedStyle.backgroundColor(), aa
            );
        }
    }

    void DefaultSliderSkin::removedFromRender() {
        RangeSkinBase::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-drag: the mouse up won't reach us
    }

    void DefaultSliderSkin::valueChanged() {
        if (_value >= 0.5f) {
            addClassName("inverted");
            removeClassName("normal");
        } else {
            addClassName("normal");
            removeClassName("inverted");
        }
    }

    void DefaultSliderSkin::sendMouseValue(const int x, const int y) {
        if (_computedStyle.orientation() == Orientation::Vertical) {
            component()->setLinearPercentage(
                1.0f - (
                    (static_cast<float>(y) - chromeInset) /
                    (static_cast<float>(_height) - 2 * chromeInset)
                ));
        } else {
            component()->setLinearPercentage(
                (static_cast<float>(x) - chromeInset) /
                (static_cast<float>(_width) - 2 * chromeInset)
            );
        }
    }
}
