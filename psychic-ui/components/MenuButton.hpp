#pragma once

#include <functional>
#include <utility>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "Label.hpp"

namespace psychic_ui {
    class MenuButton;
    class MenuItem;

    class MenuButtonSkin : public Skin<MenuButton> {
    public:
        MenuButtonSkin() : Skin<MenuButton>() {
            tag("MenuButtonSkin");
        }

        /**
         * The item or the columns changed (a check mark): show them again
         */
        virtual void itemChanged() {}
    };

    class MenuButton : public Component<MenuButtonSkin> {
    public:
        explicit MenuButton(const MenuItem *menuItem);
        const MenuItem *menuItem() const;
        bool selected() const;

        template<class Self>
        Self &selected(this Self &self, detail::BoolArg selected) {
            static_cast<MenuButton &>(self).updateSelected(selected.value);
            return self;
        }

        bool active() const override;

        /**
         * Whether its menu keeps a column for icons (some row has one), so that labels line up
         */
        bool iconColumn() const { return _iconColumn; }

        /**
         * Whether its menu keeps a column for check marks (some row is checkable): before the label, or after it
         * with `checkTrailing` (a select's list)
         */
        bool checkColumn() const { return _checkColumn; }
        bool checkTrailing() const { return _checkTrailing; }

        /**
         * Whether its item shows a check mark now (a checked checkable or radio item)
         */
        bool checked() const;

        /**
         * Set by its menu: the columns it keeps
         */
        void setColumns(bool icon, bool check, bool checkTrailing);

        /**
         * Its item changed (checked, or a radio group's value): the skin shows it again
         */
        void itemChanged();

    protected:
        const MenuItem *_menuItem{};
        bool           _selected{false};
        bool           _iconColumn{false};
        bool           _checkColumn{false};
        bool           _checkTrailing{false};

        void updateSelected(bool selected);
        void skinChanged() override;
    };
}

