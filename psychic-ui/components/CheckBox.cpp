#include <iostream>
#include "CheckBox.hpp"

namespace psychic_ui {

    CheckBox::CheckBox(const std::string &label, std::function<void(bool)> onChangeCallback) :
        Component() {
        tag("CheckBox");

        _mouseChildren = false; // !important
        _focusEnabled  = true;  // Tab reaches it; Space toggles it

        _label = label;

        keyPressed.subscribe(
            [this](const Key key, const Mod /*modifiers*/) {
                if (_enabled && key == Key::SPACE) {
                    updateChecked(!_checked);
                }
            }
        );

        if (onChangeCallback) {
            changed.subscribe(std::move(onChangeCallback));
        }

        clicked.subscribe(
            [this]() {
                if (_enabled) {
                    updateChecked(!_checked);
                }
            }
        );
    }

    const std::string &CheckBox::label() const {
        return _label;
    }

    void CheckBox::updateLabel(const std::string &label) {
        _label = label;
        if (_skin) {
            _skin->setLabel(_label);
        }
    }

    bool CheckBox::checked() const {
        return _checked;
    }

    void CheckBox::updateChecked(const bool checked) {
        if (_checked != checked) {
            _checked = checked;
            if (_checked) {
                addClassName("checked");
            } else {
                removeClassName("checked");
            }
            invalidateStyle();
            changed.emit(_checked);
        }
    }

    bool CheckBox::active() const {
        return Component::active() || _checked;
    };

    void CheckBox::skinChanged() {
        if (_skin) {
            _skin->setLabel(_label);
        }
    }


}
