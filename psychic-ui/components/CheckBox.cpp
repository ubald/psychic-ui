#include <iostream>
#include <utility>
#include "CheckBox.hpp"

namespace psychic_ui {


    CheckBox::CheckBox(const std::string &label, std::function<void(bool)> onChangeCallback) :
        Component() {
        setTag("CheckBox");

        _mouseChildren = false; // !important

        _label = label;

        if (onChangeCallback) {
            onChange(std::move(onChangeCallback));
        }

        onClick.subscribe(
            [this]() {
                setChecked(!_checked);
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

    const bool CheckBox::checked() const {
        return _checked;
    }

    void CheckBox::setChecked(const bool checked) {
        if (_checked != checked) {
            _checked = checked;
            invalidateStyle();
            onChange(_checked);
        }
    }

    const bool CheckBox::active() const {
        return Component::active() || _checked;
    };

    void CheckBox::skinChanged() {
        if (_skin) {
            _skin->setLabel(_label);
        }
    }


}
