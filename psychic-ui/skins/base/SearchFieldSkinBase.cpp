#include "SearchFieldSkinBase.hpp"

namespace psychic_ui {
    SearchFieldSkinBase::SearchFieldSkinBase() :
        SearchFieldSkin() {
        tag("SearchFieldSkinBase");
        _icon = &add<Icon>("search");
        _icon->addClassName("icon")
             .mouseEnabled(false);
    }

    void SearchFieldSkinBase::addedToComponent() {
        add(component()->input());
        add(component()->clearButton());
    }

    void SearchFieldSkinBase::removedFromComponent() {
        remove(component()->input());
        remove(component()->clearButton());
    }

    void SearchFieldSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void SearchFieldSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
