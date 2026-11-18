#include <cstddef>
#include <limits>
#include <utility>
#include <vector>
#include "include/core/SkCanvas.h"
#include "ColorSwatchesSkinBase.hpp"
#include "SkinState.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/components/ColorSwatches.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "psychic-ui/style/Color.hpp"

namespace psychic_ui {
    ColorSwatchesSkinBase::ColorSwatchesSkinBase() :
        ColorSwatchesSkin() {
        tag("ColorSwatchesSkinBase");
    }

    void ColorSwatchesSkinBase::setColors(const std::vector<Color> &colors) {
        for (const Div *swatch: _swatches) {
            remove(*swatch);
        }
        _swatches.clear();
        for (const Color &color: colors) {
            Div &swatch = add<Div>();
            swatch.addClassName("swatch")
                  .backgroundColor(color);
            _swatches.push_back(&swatch);
        }
        const int selected = _selected;
        _selected = -1;
        setSelected(selected);
        const int focused = _focused;
        _focused = -1;
        setFocused(focused);
    }

    void ColorSwatchesSkinBase::setFocused(const int index) {
        if (_focused >= 0 && std::cmp_less(_focused, _swatches.size())) {
            _swatches[static_cast<std::size_t>(_focused)]->removeClassName("focused");
        }
        _focused = index;
        if (_focused >= 0 && std::cmp_less(_focused, _swatches.size())) {
            _swatches[static_cast<std::size_t>(_focused)]->addClassName("focused");
        }
    }

    void ColorSwatchesSkinBase::setSelected(const int index) {
        if (_selected >= 0 && std::cmp_less(_selected, _swatches.size())) {
            _swatches[static_cast<std::size_t>(_selected)]->removeClassName("selected");
        }
        _selected = index;
        if (_selected >= 0 && std::cmp_less(_selected, _swatches.size())) {
            _swatches[static_cast<std::size_t>(_selected)]->addClassName("selected");
        }
    }

    int ColorSwatchesSkinBase::swatchAt(const int x, const int y) const {
        // The swatches are our children: their positions are in our coordinates
        int   nearest  = -1;
        float distance = std::numeric_limits<float>::max();
        for (std::size_t i = 0; i < _swatches.size(); ++i) {
            const Div  *swatch  = _swatches[i];
            const float dx      = static_cast<float>(x) - (static_cast<float>(swatch->x()) + static_cast<float>(swatch->getWidth()) / 2.0f);
            const float dy      = static_cast<float>(y) - (static_cast<float>(swatch->y()) + static_cast<float>(swatch->getHeight()) / 2.0f);
            const float squared = (dx * dx) + (dy * dy);
            if (squared < distance) {
                distance = squared;
                nearest  = static_cast<int>(i);
            }
        }
        return nearest;
    }

    void ColorSwatchesSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void ColorSwatchesSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
