#include "include/core/SkCanvas.h"
#include "SkinState.hpp"
#include "TextInputSkinBase.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/components/TextInput.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    TextInputSkinBase::TextInputSkinBase() :
        TextInputSkin() {
        tag("TextInputSkinBase");
        // Out of the layout: the skin's justify and alignment put it where the text starts
        _placeholder = &add<Label>();
        _placeholder->addClassName("placeholder")
                    .mouseEnabled(false)
                    .position(Position::Absolute)
                    .visible(false);
        // Over the placeholder; its field takes the mouse
        _text = &add<Text>();
        _text->addClassName("text");
    }

    Text *TextInputSkinBase::textView() {
        return _text;
    }

    void TextInputSkinBase::setPlaceholder(const std::string &placeholder, const bool shown) {
        _placeholder->text(placeholder);
        _placeholder->visible(shown);
    }

    void TextInputSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void TextInputSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
