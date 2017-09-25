#include "DefaultSkin.hpp"
#include "DefaultTextInputSkin.hpp"

namespace psychic_ui {
    DefaultTextInputSkin::DefaultTextInputSkin() :
        TextInputSkin() {
        setTag("DefaultTextInputSkin");
        addClassName("defaultSkinChrome");
    }

    //void DefaultTextInputSkin::setText(const std::string &text) {
    //
    //}

    void DefaultTextInputSkin::draw(SkCanvas *canvas) {
        // Don't use default rendering, render out custom component chrome
        SkRRect inner = default_skin::draw(this, canvas);
    }
}
