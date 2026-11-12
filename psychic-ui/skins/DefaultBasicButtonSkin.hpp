#pragma once

#include "../components/Button.hpp"

namespace psychic_ui {
    class DefaultBasicButtonSkin : public ButtonSkin {
    public:
        DefaultBasicButtonSkin();
        void setLabel(const std::string &label) override;
    protected:
        Label *_label{nullptr}; // A child, which we own
    };
}

