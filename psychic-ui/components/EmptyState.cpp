#include <string>
#include <string_view>
#include "EmptyState.hpp"
#include "Button.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "Text.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    EmptyState::EmptyState(const std::string &icon, const std::string &title, const std::string &description) :
        Div() {
        tag("EmptyState");
        _defaultStyle.direction(Direction::Column)
            .alignItems(Align::Center)
            .justify(Justify::Center);

        _tile = &add<Div>();
        _tile->addClassName("icon-tile")
             .alignItems(Align::Center)
             .justify(Justify::Center);
        _icon = &_tile->add<Icon>();
        _icon->addClassName("icon");
        updateIcon(icon);

        _title = &add<Label>(title);
        _title->addClassName("title");

        _description = &add<Text>();
        _description->multiline(true)
                    .addClassName("description");
        updateDescription(description);
    }

    std::string EmptyState::title() const {
        return _title->text();
    }

    std::string EmptyState::description() const {
        return _description->text();
    }

    Button &EmptyState::action(const std::string &label) {
        if (!_action) {
            _action = &add<Button>(label);
            _action->addClassName("action")
                    .skin("primary");
        } else {
            _action->label(label);
        }
        return *_action;
    }

    void EmptyState::updateTitle(const std::string &title) {
        _title->text(title);
    }

    void EmptyState::updateDescription(const std::string &description) {
        _description->text(description)
                    .display(description.empty() ? Display::None : Display::Flex);
    }

    void EmptyState::updateIcon(const std::string_view icon) {
        _icon->name(icon);
        _tile->display(icon.empty() ? Display::None : Display::Flex);
    }
}
