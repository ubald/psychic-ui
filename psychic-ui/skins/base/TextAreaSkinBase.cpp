#include <memory>
#include "include/core/SkCanvas.h"
#include "SkinState.hpp"
#include "TextAreaSkinBase.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/components/Scroller.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/components/TextArea.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "psychic-ui/style/Length.hpp"

namespace psychic_ui {

    TextAreaSkinBase::TextAreaSkinBase() :
        TextAreaSkin() {
        tag("TextAreaSkinBase");
        // The viewport's content, held by its child list; its field takes the mouse
        auto text = std::make_shared<Text>();
        _text = text.get();
        _text->addClassName("text")
             .multiline(true)
             .minHeight(100_pct); // The whole viewport, empty or not
        _viewport = &add<Scroller>(text);
        _viewport->addClassName("viewport");
    }

    Text *TextAreaSkinBase::textView() {
        return _text;
    }

    void TextAreaSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void TextAreaSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
