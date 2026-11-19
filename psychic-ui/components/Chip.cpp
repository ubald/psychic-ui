#include <string>
#include <string_view>
#include "Chip.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "Tone.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    Chip::Chip(const std::string &label, const Tone tone) :
        Div() {
        tag("Chip");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center)
            .alignSelf(Align::Center)
            .shrink(0);
        _focusEnabled = true;

        _icon = &add<Icon>();
        _icon->addClassName("icon")
             .display(Display::None);
        _label = &add<Label>(label);
        _label->addClassName("label");
        _remove = &add<Icon>("x");
        _remove->addClassName("remove")
               .display(Display::None);
        _remove->onClick([this]() { dismiss(); });

        // Clicking it emits `clicked`, which needs a subscriber to swallow the press before a parent takes it
        clicked.subscribe([]() {});

        keyPressed.subscribe(
            [this](const Key key, const Mod /*mod*/) {
                if (!_enabled) {
                    return false;
                }
                if (key == Key::ENTER || key == Key::KP_ENTER || key == Key::SPACE) {
                    clicked.emit(mouseEventAtCenter());
                    return true;
                }
                if (key == Key::BACKSPACE || key == Key::DELETE) {
                    dismiss();
                    return true;
                }
                return false;
            }
        );

        updateTone(tone);
    }

    void Chip::dismiss() {
        if (_enabled && _removable) {
            dismissed.emit();
        }
    }

    void Chip::updateTone(const Tone tone) {
        _tone = tone;
        applyTone(*this, tone);
    }

    void Chip::updateIcon(const std::string_view icon) {
        _icon->name(icon)
             .display(icon.empty() ? Display::None : Display::Flex);
    }

    void Chip::updateRemovable(const bool removable) {
        _removable = removable;
        _remove->display(removable ? Display::Flex : Display::None);
        if (removable) {
            addClassName("removable");
        } else {
            removeClassName("removable");
        }
    }

    void Chip::updateSelected(const bool selected) {
        _selected = selected;
        if (selected) {
            addClassName("selected");
        } else {
            removeClassName("selected");
        }
    }
}
