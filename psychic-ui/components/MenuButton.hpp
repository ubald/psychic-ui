#pragma once

#include <functional>
#include <utility>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "Label.hpp"

namespace psychic_ui {
    class MenuButton;

    class MenuButtonSkin : public Skin<MenuButton> {
    public:
        MenuButtonSkin() : Skin<MenuButton>() {
            setTag("MenuButtonSkin");
        }
    };

    class MenuButton : public Component<MenuButtonSkin> {
    public:
        explicit MenuButton(const MenuItem *menuItem);
        const MenuItem *menuItem() const;
        bool selected() const;
        void setSelected(bool selected);
        bool active() const override;
    protected:
        const MenuItem *_menuItem{};
        bool           _selected{false};
    };
}

