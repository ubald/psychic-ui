#include "include/core/SkCanvas.h"
#include "SkinState.hpp"
#include "SplitterDividerSkinBase.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/components/SplitterDivider.hpp"
#include "psychic-ui/paint/Box.hpp"

namespace psychic_ui {
    SplitterDividerSkinBase::SplitterDividerSkinBase() :
        SplitterDividerSkin() {
        tag("SplitterDividerSkinBase");
        _line = &add<Div>();
        _line->addClassName("line")
             .mouseEnabled(false)
             .shrink(0);
    }

    void SplitterDividerSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void SplitterDividerSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
