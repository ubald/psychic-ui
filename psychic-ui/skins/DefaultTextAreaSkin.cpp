#include "DefaultSkin.hpp"
#include "DefaultTextAreaSkin.hpp"

namespace psychic_ui {

    DefaultTextAreaSkin::DefaultTextAreaSkin() :
        TextAreaSkin() {
        setTag("DefaultTextAreaSkin");
        addClassName("defaultSkinChrome");
    }

    void DefaultTextAreaSkin::addedToComponent() {
        add(component()->textScroller());
    };

    void DefaultTextAreaSkin::removedFromComponent() {
        remove(component()->textScroller());
    };

    void DefaultTextAreaSkin::draw(SkCanvas *canvas) {
        // Don't use default rendering, render out custom component chrome
        default_skin::draw(this, canvas);
    }
}
