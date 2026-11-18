#pragma once

#include <memory>
#include <string>
#include <vector>
#include "psychic-ui/Div.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "Menu.hpp"

namespace psychic_ui {

    /**
     * @class Select
     *
     * A closed field showing the chosen option and a chevron (`.value`, with the `placeholder` class while nothing is
     * chosen, and `.chevron`; Pivotal's), which opens the options in a list below it (a `Menu` with the `listbox`
     * class, at least as wide as the field, the chosen option checked after its label). Clicking it, or Enter, Space,
     * Up or Down while it has the focus, opens the list, with the chosen option highlighted; choosing one closes it
     * and emits `changed`. It has the `open` class while the list is open.
     *
     *     add<Select>(std::vector<std::string>{"Low", "Medium", "High"}, 1).onChange([](int index) { ... });
     */
    class Select : public Div {
    public:
        explicit Select(std::vector<std::string> options = {}, int selected = -1);

        const std::vector<std::string> &options() const { return _options; }

        template<class Self>
        Self &options(this Self &self, std::vector<std::string> options) {
            static_cast<Select &>(self).updateOptions(std::move(options));
            return self;
        }

        /**
         * The chosen option's index, -1 for none
         */
        int selectedIndex() const { return _selected; }

        /**
         * Choose an option (-1 for none), without emitting `changed`
         */
        template<class Self>
        Self &selectedIndex(this Self &self, int index) {
            static_cast<Select &>(self).updateSelected(index, false);
            return self;
        }

        /**
         * The chosen option's label, "" for none
         */
        std::string value() const;

        /**
         * What the field shows while nothing is chosen
         */
        template<class Self>
        Self &placeholder(this Self &self, const std::string &placeholder) {
            static_cast<Select &>(self).updatePlaceholder(placeholder);
            return self;
        }

        /**
         * Whether its list is open
         */
        bool isOpen() const {
            auto menu = _menu.lock();
            return menu && menu->isOpen(); // Not while it fades out
        }

        void open();
        void close();

        /**
         * The list, while it is open
         */
        Menu *menu() const { return _menu.lock().get(); }

        /**
         * The user chose an option, with its index
         */
        Signal<int> changed{};

        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        std::vector<std::string> _options{};
        int                      _selected{-1};
        std::string              _placeholder{};
        std::weak_ptr<Menu>      _menu{};

        // Children: our child list owns them
        Label *_value{nullptr};
        Icon  *_chevron{nullptr};

        void updateOptions(std::vector<std::string> options);
        void updateSelected(int index, bool emit);
        void updatePlaceholder(const std::string &placeholder);
        void showValue();
    };
}
