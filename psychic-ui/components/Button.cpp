#include <utility>
#include "Button.hpp"

namespace psychic_ui {

    Button::Button(const std::string &label, ClickCallback onClickCallback) :
        Component() {
        setTag("Button");

        _mouseChildren = false; // !important

        _label = label;

        mouseUp.subscribe(
            [this](const int /*mouseX*/, const int /*mouseY*/, const int /*button*/, const Mod /*modifiers*/) {
                if (_enabled && !_toggle) {
                    updateSelected(false);
                }
            }
        );

        mouseDown.subscribe(
            [this](const int /*mouseX*/, const int /*mouseY*/, const int /*button*/, const Mod /*modifiers*/) {
                if (_enabled && !_toggle) {
                    updateSelected(true);
                }
            }
        );

        clicked.subscribe(
            [this]() {
                if (_enabled && _toggle && _autoToggle) {
                    updateSelected(!_selected);
                }
            }
        );

        // After the toggle above, so that the callback sees the new `selected()`
        if (onClickCallback) {
            clicked.subscribe(std::move(onClickCallback));
        }
    }

    const std::string &Button::label() const {
        return _label;
    }

    void Button::updateLabel(const std::string &label) {
        _label = label;
        if (_skin) {
            _skin->setLabel(_label);
        }
    }

    bool Button::selected() const {
        return _selected;
    }

    void Button::updateSelected(const bool selected) {
        if (_selected != selected) {
            _selected = selected;
            invalidateStyle();
            changed.emit(_selected);
        }
    }

    bool Button::active() const {
        return Component::active() || _selected;
    }

    void Button::skinChanged() {
        if (_skin) {
            _skin->setLabel(_label);
        }
    }


}
