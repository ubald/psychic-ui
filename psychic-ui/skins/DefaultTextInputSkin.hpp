#pragma once

#include "../components/TextInput.hpp"

namespace psychic_ui {
    class DefaultTextInputSkin : public TextInputSkin {
    public:
        DefaultTextInputSkin();
        void setText(const std::string &text) override;
    protected:
        void draw(SkCanvas *canvas) override;
    };
}

