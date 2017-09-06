#include <iostream>
#include <utility>
#include "Button.hpp"

namespace psychicui {


    Button::Button(const std::string &label, ClickCallback onClick) :
        Component() {
        setTag("Button");
        setCursor(Cursor::Hand);

        _mouseChildren = false; // !important

        _label = label;
        _onClick = std::move(onClick);
    }

    const std::string &Button::label() const {
        return _label;
    }

    void Button::setLabel(const std::string &label) {
        _label = label;
        _skin->setLabel(_label);
    }

    const bool Button::selected() const {
        return _selected;
    }

    void Button::setSelected(const bool selected) {
        if (_selected != selected) {
            _selected = selected;
            invalidateStyle();
            if (_onChange) {
                _onChange(_selected);
            }
        }
    }

    const bool Button::active() const {
        return Div::active() || _selected;
    };

    void Button::onMouseUp(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_toggle) {
            setSelected(false);
        }
        Div::onMouseUp(mouseX, mouseY, button, modifiers);
    }

    void Button::onMouseDown(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_toggle) {
            setSelected(true);
        }
        Div::onMouseDown(mouseX, mouseY, button, modifiers);
    }

    void Button::onClick() {
        if (_toggle && _autoToggle) {
            setSelected(!_selected);
        }
        Div::onClick();
    }

    void Button::skinChanged() {
        if(_skin) {
            _skin->setLabel(_label);
        }
    }


}