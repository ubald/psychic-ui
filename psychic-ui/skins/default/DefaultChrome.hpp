#pragma once

#include "include/core/SkCanvas.h"
#include "psychic-ui/style/Style.hpp"
#include "psychic-ui/skins/paint/Box.hpp"

namespace psychic_ui::default_skins {

    /**
     * The default look's chrome: a well of `contentBackgroundColor`, 1 px inside the border box when there is a
     * border, under a stroke filling the border. The rules give it a 1 px border, 2 px when focused, so content
     * starts `chromeInset` px inside, focused or not.
     */
    inline constexpr float chromeInset = 2.0f;

    /**
     * Paint the chrome, then return where the content goes, `chromeInset` inside the box
     */
    inline SkRRect drawChrome(SkCanvas *canvas, const paint::Box &box, const Style &style) {
        const bool aa = style.antiAlias();
        paint::fill(canvas, box.border > 0 ? box.inset(1.0f) : box.outer, style.contentBackgroundColor(), aa);
        if (box.border > 0) {
            paint::stroke(canvas, box.strokeLine(), style.borderColor(), box.border, aa);
        }
        return box.inset(chromeInset);
    }
}
