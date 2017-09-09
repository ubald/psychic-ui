#include <iostream>
#include <utility>
#include "CheckBox.hpp"

namespace psychicui {


    CheckBox::CheckBox(const std::string &label, ClickCallback onClickCallback) :
        Component() {
        setTag("CheckBox");

        _mouseChildren = false; // !important

        _label = label;

        if (onClickCallback) {
            onClick(std::move(onClickCallback));
        }

        onMouseUp.subscribe(
            [this](const int mouseX, const int mouseY, const int checkBox, const int modifiers) {
                if (!_toggle) {
                    setSelected(false);
                }
            }
        );

        onMouseDown.subscribe(
            [this](const int mouseX, const int mouseY, const int checkBox, const int modifiers) {
                if (!_toggle) {
                    setSelected(true);
                }
            }
        );

        onClick.subscribe(
            [this]() {
                if (_toggle && _autoToggle) {
                    setSelected(!_selected);
                }
            }
        );
    }

    const std::string &CheckBox::label() const {
        return _label;
    }

    void CheckBox::setLabel(const std::string &label) {
        _label = label;
        _skin->setLabel(_label);
    }

    const bool CheckBox::selected() const {
        return _selected;
    }

    void CheckBox::setSelected(const bool selected) {
        if (_selected != selected) {
            _selected = selected;
            invalidateStyle();
            if (_onChange) {
                _onChange(_selected);
            }
        }
    }

    const bool CheckBox::active() const {
        return Component::active() || _selected;
    };

    void CheckBox::skinChanged() {
        if (_skin) {
            _skin->setLabel(_label);
        }
    }


}