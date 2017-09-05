#pragma once

#include "../Skin.hpp"
#include "../components/Button.hpp"

namespace psychicui {
    class TitleBarButtonSkin : public ButtonSkin {
    public:
        TitleBarButtonSkin();
    protected:
        void draw(SkCanvas *canvas);
    };
}

