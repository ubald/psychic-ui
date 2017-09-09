#pragma once

#include "../Skin.hpp"
#include "../Shape.hpp"
#include "../components/Label.hpp"
#include "../components/MenuButton.hpp"

namespace psychicui {
    class DefaultMenuButtonSkin : public MenuButtonSkin {
    public:
        DefaultMenuButtonSkin();
    protected:
        std::shared_ptr<Label> label{nullptr};
        std::shared_ptr<Label> shortcut{nullptr};

        void added() override;
    };

    class DefaultSubMenuButtonSkin : public DefaultMenuButtonSkin {
    public:
        DefaultSubMenuButtonSkin();
    protected:
        std::shared_ptr<Shape> shape{nullptr};
    };
}

