#include "DefaultSkin.hpp"
#include "DefaultTextInputSkin.hpp"

namespace psychic_ui {
    DefaultTextInputSkin::DefaultTextInputSkin() :
        TextInputSkin() {
        setTag("DefaultTextInputSkin");
        addClassName("defaultSkinChrome");

        textDisplay = add<Text>();
        textDisplay
            ->style()
            ->set(left, 0.0f)
            ->set(top, 0.0f)
            ->set(right, 0.0f)
            ->set(bottom, 0.0f);
    }

    void DefaultTextInputSkin::draw(SkCanvas *canvas) {
        // Don't use default rendering, render out custom component chrome
        SkRRect inner = default_skin::draw(this, canvas);
    }
}
