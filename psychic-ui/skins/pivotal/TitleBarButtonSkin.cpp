#include <algorithm>
#include "include/core/SkCanvas.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "psychic-ui/paint/Marks.hpp"
#include "TitleBarButtonSkin.hpp"
#include "psychic-ui/components/Button.hpp"
#include "psychic-ui/style/Length.hpp"

namespace psychic_ui {
    TitleBarButtonSkin::TitleBarButtonSkin() :
        ButtonSkin() {
        tag("TitleBarButtonSkin");
        _inlineStyle.width(100_pct)
            .height(100_pct);
    }

    void TitleBarButtonSkin::draw(SkCanvas *canvas) {
        const SkRect  bounds = SkRect::Make(SkIRect::MakeWH(_width, _height));
        const float   radius = std::min(bounds.width(), bounds.height()) / 2.0f;
        const SkPoint center = bounds.center();
        if (const Button *button = component()) {
            paint::disc(canvas, center, radius, button->computedStyle().backgroundColor());
        }
    }

}
