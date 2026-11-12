#pragma once

#include "../Skin.hpp"
#include "../Shape.hpp"
#include "../components/Label.hpp"
#include "../components/MenuButton.hpp"

namespace psychic_ui {
    class DefaultMenuButtonSkin : public MenuButtonSkin {
    public:
        DefaultMenuButtonSkin();
    protected:
        Label *label{nullptr}; // A child, which we own
        Label *shortcut{nullptr}; // A child, which we own

        void added() override;
    };

    class DefaultSubMenuButtonSkin : public DefaultMenuButtonSkin {
    public:
        DefaultSubMenuButtonSkin();
    protected:
        Shape *shape{nullptr}; // A child, which we own
    };
}

