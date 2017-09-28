#pragma once

#include "../components/TextInput.hpp"

namespace psychic_ui {
    class DefaultTextInputSkin : public TextInputSkin {
    public:
        DefaultTextInputSkin();
    protected:
        void draw(SkCanvas *canvas) override;
    };
}

