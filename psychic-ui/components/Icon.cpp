#include <algorithm>
#include <string_view>
#include "include/core/SkCanvas.h"
#include "psychic-ui/paint/Icon.hpp"
#include "Icon.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Measure.hpp"

namespace psychic_ui {

    Icon::Icon(std::string_view name) :
        Div(),
        _name(name) {
        tag("Icon");
        setMeasurable();
    }

    void Icon::updateName(std::string_view name) {
        _name = name;
    }

    void Icon::styleUpdated() {
        Div::styleUpdated();
        // Yoga caches measures: measure again when the size changes
        const float size = _computedStyle.iconSize();
        if (size != _measuredSize) {
            _measuredSize = size;
            invalidate();
        }
    }

    Size Icon::measure(Constraint width, Constraint height) {
        const float size = _computedStyle.iconSize();
        return Size{width.apply(size), height.apply(size)};
    }

    void Icon::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        if (_name.empty()) {
            return;
        }
        const float size = std::min({_computedStyle.iconSize(), _paddedRect.width(), _paddedRect.height()});
        const SkRect box = SkRect::MakeXYWH(
            _paddedRect.centerX() - (size / 2.0f),
            _paddedRect.centerY() - (size / 2.0f),
            size,
            size
        );
        paint::icon(canvas, _name, box, _computedStyle.color(), _computedStyle.iconStrokeWidth());
    }
}
