#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "psychic-ui/Window.hpp"
#include "Select.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Popup.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "Menu.hpp"
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    Select::Select(std::vector<std::string> options, const int selected) :
        Div(),
        _options(std::move(options)),
        _selected(selected >= 0 && std::cmp_less(selected, _options.size()) ? selected : -1) {
        tag("Select");
        _focusEnabled  = true;
        _mouseChildren = false; // The field takes the presses and the focus, not its parts
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center);

        _value = &add<Label>();
        _value->addClassName("value")
              .shrink(1);

        _chevron = &add<Icon>("chevron-down");
        _chevron->addClassName("chevron")
                .shrink(0)
                .marginLeft(Length{0, Length::Unit::Auto});

        showValue();

        onClick([this]() { isOpen() ? close() : open(); });
        keyPressed.subscribe(
            [this](const Key key, const Mod /*modifiers*/) {
                if (key != Key::ENTER && key != Key::KP_ENTER && key != Key::SPACE && key != Key::DOWN && key != Key::UP) {
                    return false;
                }
                open();
                return true;
            }
        );
    }

    std::string Select::value() const {
        return _selected >= 0 ? _options[_selected] : std::string{};
    }

    void Select::updateOptions(std::vector<std::string> options) {
        close();
        _options = std::move(options);
        if (std::cmp_greater_equal(_selected, _options.size())) {
            _selected = -1;
        }
        showValue();
    }

    void Select::updateSelected(const int index, const bool emit) {
        const int selected = index >= 0 && std::cmp_less(index, _options.size()) ? index : -1;
        if (selected == _selected) {
            return;
        }
        _selected = selected;
        showValue();
        if (emit) {
            changed.emit(_selected);
        }
    }

    void Select::updatePlaceholder(const std::string &placeholder) {
        _placeholder = placeholder;
        showValue();
    }

    void Select::showValue() {
        _value->text(_selected >= 0 ? _options[_selected] : _placeholder);
        if (_selected < 0) {
            _value->addClassName("placeholder");
        } else {
            _value->removeClassName("placeholder");
        }
    }

    void Select::open() {
        Window *w = window();
        if (!w || !_enabled || isOpen() || _options.empty()) {
            return;
        }

        // The options as radio items of one group, the chosen one's index as its value
        auto group = std::make_shared<MenuRadioGroup>(
            std::to_string(_selected),
            [this, weak = weak_from_this()](const std::string &value) {
                if (weak.lock()) {
                    updateSelected(std::stoi(value), true);
                }
            }
        );
        MenuItems items{};
        for (std::size_t i = 0; i < _options.size(); ++i) {
            auto item = Menu::radio(group, _options[i], std::to_string(i));
            item->closes = true;
            items.push_back(item);
        }

        auto menu = std::make_shared<Menu>(items);
        menu->anchor(*this)
            .side(Side::Bottom)
            .alignment(PopupAlign::Start)
            .sideOffset(6)
            .minWidth(static_cast<float>(getWidth()))
            .addClassName("listbox");
        menu->checksAfter(true);
        // The menu holds the group, which calls back into this select: it must not outlive it
        menu->onClose(
            [this, weak = weak_from_this()]() {
                if (weak.lock()) {
                    updateClassName("open", false);
                }
            }
        );
        _menu = menu;
        w->openMenu(menu);
        if (_selected >= 0) {
            menu->highlight(_selected);
        }
        updateClassName("open", true);
    }

    void Select::close() {
        if (auto menu = _menu.lock()) {
            menu->close();
        }
    }
}
