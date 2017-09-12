#pragma once

#include <functional>
#include <utility>
#include "psychicui/psychicui.hpp"
#include "psychicui/Component.hpp"
#include "psychicui/Skin.hpp"
#include "Label.hpp"

namespace psychicui {
    class MenuButton;

    class MenuButtonSkin : public Skin<MenuButton> {
    public:
        MenuButtonSkin() : Skin<MenuButton>() {
            setTag("MenuButtonSkin");
        }
    };

    class MenuButton : public Component<MenuButtonSkin> {
    public:
        MenuButton(const MenuItem *menuItem);
        const MenuItem *menuItem() const;
        const bool selected() const;
        void setSelected(bool selected);
        const bool active() const override;
    protected:
        const MenuItem *_menuItem{};
        bool           _selected{false};
    };
}

