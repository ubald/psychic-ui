#include "DefaultSliderSkin.hpp"
#include "DefaultChrome.hpp"

namespace psychic_ui {
    using default_skins::chromeInset;

    DefaultSliderSkin::DefaultSliderSkin() :
        RangeSkinBase() {
        tag("DefaultSliderSkin");
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

    void DefaultSliderSkin::valueChanged() {
        if (_value >= 0.5f) {
            addClassName("inverted");
            removeClassName("normal");
        } else {
            addClassName("normal");
            removeClassName("inverted");
        }
    }

    float DefaultSliderSkin::valueAt(const int x, const int y) const {
        // Where the fill would end: it fills the chrome's inside, from the left or the bottom
        if (_computedStyle.orientation() == Orientation::Vertical) {
            return 1.0f - (
                (static_cast<float>(y) - chromeInset) /
                (static_cast<float>(_height) - 2 * chromeInset)
            );
        }
        return (static_cast<float>(x) - chromeInset) /
               (static_cast<float>(_width) - 2 * chromeInset);
    }
}
