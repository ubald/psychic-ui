#include "SearchFieldSkinBase.hpp"

namespace psychic_ui {
    SearchFieldSkinBase::SearchFieldSkinBase() :
        SearchFieldSkin() {
        tag("SearchFieldSkinBase");
        _icon = &add<Icon>("search");
        _icon->addClassName("icon")
             .mouseEnabled(false);

        // The text, and under it the placeholder, out of the layout: the box's justify and alignment put it where
        // the text starts
        _field = &add<Div>();
        _field->addClassName("field");
        _placeholder = &_field->add<Label>();
        _placeholder->addClassName("placeholder")
                    .mouseEnabled(false)
                    .position(Position::Absolute)
                    .visible(false);
        _text = &_field->add<Text>(); // Its field takes the mouse
        _text->addClassName("text");

        // Its field clears itself on a press there (`clearAt`); the mouse reaches it for its hover
        _clear = &add<Div>();
        _clear->addClassName("clear")
              .visible(false);
        _clear->add<Icon>("x").mouseEnabled(false);
    }

    Text *SearchFieldSkinBase::textView() {
        return _text;
    }

    void SearchFieldSkinBase::setPlaceholder(const std::string &placeholder, const bool shown) {
        _placeholder->text(placeholder);
        _placeholder->visible(shown);
    }

    void SearchFieldSkinBase::setClearable(const bool clearable) {
        _clear->visible(clearable);
    }

    bool SearchFieldSkinBase::clearAt(const float x, const float y) const {
        return _clear->visible() && _clear->contains(x, y); // A child of ours: its place is in our coordinates
    }

    void SearchFieldSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void SearchFieldSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
