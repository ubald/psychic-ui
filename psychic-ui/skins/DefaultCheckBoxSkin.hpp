#pragma once

#include "../Skin.hpp"
#include "../Shape.hpp"
#include "../components/CheckBox.hpp"

namespace psychic_ui {
    class DefaultCheckBoxSkin : public CheckBoxSkin {
    public:
        DefaultCheckBoxSkin();
        void setLabel(const std::string &label) override;
    protected:
        Shape *_box{nullptr}; // A child, which we own
        Label *_label{nullptr}; // A child, which we own
        void draw(SkCanvas *canvas) override;
    };
}

