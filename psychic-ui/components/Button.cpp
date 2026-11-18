#include <utility>
#include "Button.hpp"

namespace psychic_ui {

    Button::Button(const std::string &label, ClickCallback onClickCallback) :
        Component(),
        _label(label) {
        tag("Button");

        _mouseChildren = false; // !important
        _focusEnabled  = true;  // Tab reaches it; Space or Enter clicks it

        keyPressed.subscribe(
            [this](const Key key, const Mod /*modifiers*/) {
                if (_enabled && (key == Key::SPACE || key == Key::ENTER || key == Key::KP_ENTER)) {
                    MouseEvent click = mouseEventAtCenter();
                    click.button     = MouseButton::LEFT;
                    clicked.emit(click);
                }
            }
        );

        mouseReleased.subscribe(
            [this] {
                if (_enabled && !_toggle) {
                    updateSelected(false);
                }
            }
        );

        mousePressed.subscribe(
            [this](const MouseEvent &event) {
                if (_enabled && !_toggle && event.button == MouseButton::LEFT) { // Only the left button clicks
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

    void Button::updateIcon(const std::string &icon) {
        _icon = icon;
        if (_skin) {
            _skin->setIcon(_icon);
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
            _skin->setIcon(_icon);
            _skin->setLabel(_label);
        }
    }


}
