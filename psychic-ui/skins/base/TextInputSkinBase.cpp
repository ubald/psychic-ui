#include "TextInputSkinBase.hpp"

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
    }

    void TextInputSkinBase::setText(const std::string &text) {
        _placeholder->visible(text.empty() && !_placeholder->text().empty());
    }

    void TextInputSkinBase::setPlaceholder(const std::string &placeholder) {
        _placeholder->text(placeholder);
        setText(component() ? component()->text() : "");
    }

    void TextInputSkinBase::addedToComponent() {
        add(component()->textDisplay());
    }

    void TextInputSkinBase::removedFromComponent() {
        remove(component()->textDisplay());
    }

    void TextInputSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void TextInputSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
