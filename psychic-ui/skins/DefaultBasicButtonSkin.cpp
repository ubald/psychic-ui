#include "DefaultBasicButtonSkin.hpp"

namespace psychic_ui {
    DefaultBasicButtonSkin::DefaultBasicButtonSkin() :
        ButtonSkin() {
        setTag("DefaultBasicButtonSkin");
        _label = add<Label>();
    }

    void DefaultBasicButtonSkin::setLabel(const std::string &label) {
        _label->setText(label);
    }
}
