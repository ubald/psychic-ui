#pragma once

#include "DefaultBasicButtonSkin.hpp"

namespace psychic_ui {
    class DefaultButtonSkin : public DefaultBasicButtonSkin {
    public:
        DefaultButtonSkin();
    protected:
        void draw(SkCanvas *canvas) override;
    };
}

