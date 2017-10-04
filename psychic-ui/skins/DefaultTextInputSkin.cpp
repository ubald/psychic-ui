#include "DefaultSkin.hpp"
#include "DefaultTextInputSkin.hpp"

namespace psychic_ui {
    DefaultTextInputSkin::DefaultTextInputSkin() :
        TextInputSkin() {
        setTag("DefaultTextInputSkin");
        addClassName("defaultSkinChrome");
    }

    void DefaultTextInputSkin::addedToComponent() {
        add(component()->textDisplay());
    };

    void DefaultTextInputSkin::removedFromComponent() {
        remove(component()->textDisplay());
    };

    void DefaultTextInputSkin::draw(SkCanvas *canvas) {
        // Don't use default rendering, render out custom component chrome
        default_skin::draw(this, canvas);
    }
}
