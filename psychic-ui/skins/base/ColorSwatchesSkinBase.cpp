#include <limits>
#include "ColorSwatchesSkinBase.hpp"

namespace psychic_ui {
    ColorSwatchesSkinBase::ColorSwatchesSkinBase() :
        ColorSwatchesSkin() {}

    void ColorSwatchesSkinBase::setColors(const std::vector<Color> &colors) {
        for (Div *swatch: _swatches) {
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
        if (_focused >= 0 && _focused < static_cast<int>(_swatches.size())) {
            _swatches[static_cast<std::size_t>(_focused)]->removeClassName("focused");
        }
        _focused = index;
        if (_focused >= 0 && _focused < static_cast<int>(_swatches.size())) {
            _swatches[static_cast<std::size_t>(_focused)]->addClassName("focused");
        }
    }

    void ColorSwatchesSkinBase::setSelected(const int index) {
        if (_selected >= 0 && _selected < static_cast<int>(_swatches.size())) {
            _swatches[static_cast<std::size_t>(_selected)]->removeClassName("selected");
        }
        _selected = index;
        if (_selected >= 0 && _selected < static_cast<int>(_swatches.size())) {
            _swatches[static_cast<std::size_t>(_selected)]->addClassName("selected");
        }
    }

    int ColorSwatchesSkinBase::swatchAt(const int x, const int y) const {
        // The swatches are our children: their positions are in our coordinates
        int   nearest  = -1;
        float distance = std::numeric_limits<float>::max();
        for (std::size_t i = 0; i < _swatches.size(); ++i) {
            const Div  *swatch = _swatches[i];
            const float dx     = static_cast<float>(x) - (static_cast<float>(swatch->x()) + static_cast<float>(swatch->getWidth()) / 2.0f);
            const float dy     = static_cast<float>(y) - (static_cast<float>(swatch->y()) + static_cast<float>(swatch->getHeight()) / 2.0f);
            const float d      = dx * dx + dy * dy;
            if (d < distance) {
                distance = d;
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
