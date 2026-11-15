#include <algorithm>
#include <utility>
#include <vector>
#include "psychic-ui/Window.hpp"
#include "Dialog.hpp"

namespace psychic_ui {

    Dialog::Dialog(const std::string &title) :
        Popup() {
        tag("Dialog");
        modal(true);
        trapFocus(true);
        // The backdrop: the whole overlay, the panel in its middle
        _defaultStyle.left(0)
            .top(0)
            .width(100_pct)
            .height(100_pct)
            .alignItems(Align::Center)
            .justify(Justify::Center);

        _panel = &add<Div>();
        _panel->addClassName("panel")
              .direction(Direction::Column)
              .shrink(1);
        _surface = _panel;

        _header = &_panel->add<Div>();
        _header->addClassName("header")
               .direction(Direction::Row)
               .alignItems(Align::Center);

        _icon = &_header->add<Icon>();
        _icon->addClassName("icon")
             .display(Display::None);

        _title = &_header->add<Label>(title);
        _title->addClassName("title")
              .grow(1)
              .shrink(1);

        _close = &_header->add<Button>();
        _close->icon("x")
              .addClassName("icon-button")
              .addClassName("close")
              .shrink(0);
        _close->onClick([this]() { close(); });

        _body = &_panel->add<Div>();
        _body->addClassName("body")
             .direction(Direction::Column)
             .shrink(1);

        _description = &_body->add<Text>();
        _description->multiline(true)
                    .selectable(false)
                    .addClassName("description")
                    .display(Display::None);

        _footer = &_panel->add<Div>();
        _footer->addClassName("footer")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .justify(Justify::End)
               .display(Display::None);
    }

    void Dialog::updateIcon(const std::string_view icon) {
        _icon->name(icon)
             .display(icon.empty() ? Display::None : Display::Flex);
    }

    void Dialog::updateDescription(const std::string &description) {
        _description->text(description)
                    .display(description.empty() ? Display::None : Display::Flex);
    }

    Button &Dialog::action(const std::string &label, std::function<void()> callback, const std::string &variant, const bool closes) {
        _footer->display(Display::Flex);
        auto &button = _footer->add<Button>(label);
        button.skin(variant);
        // The dialog owns the button: a raw pointer to it. Closing frees the dialog, so nothing of it is used after.
        button.onClick(
            [this, callback = std::move(callback), closes]() {
                auto call = callback;
                if (closes) {
                    close();
                }
                if (call) {
                    call();
                }
            }
        );
        return button;
    }

    void Dialog::opened() {
        // The panel holds the focus until it is laid out with what was added to it: nothing shows a ring yet
        if (Window *w = window()) {
            _openedVisible = w->focusVisible();
            w->requestFocus(_panel, false);
        }
    }

    Div *Dialog::initialFocus() const {
        if (auto div = _initialFocus.lock()) {
            return div.get();
        }
        std::vector<Div *> body{};
        Window::focusables(_body, body);
        const auto isButton = [](const Div *div) { return dynamic_cast<const Button *>(div) != nullptr; };
        if (const auto field = std::find_if_not(body.begin(), body.end(), isButton); field != body.end()) {
            return *field;
        }
        Window::focusables(_footer, body);
        const auto button = std::find_if(body.begin(), body.end(), isButton);
        return button != body.end() ? *button : nullptr;
    }

    void Dialog::laidOut() {
        Window *w = window();
        if (!w || w->focusedDiv() != _panel) {
            return; // The focus went elsewhere already
        }
        if (Div *div = initialFocus()) {
            // A field shows it has the focus anyway; a button's ring shows when the dialog came from the keyboard
            w->requestFocus(div, _openedVisible);
        }
    }
}
