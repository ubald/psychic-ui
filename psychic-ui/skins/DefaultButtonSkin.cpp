#include "DefaultButtonSkin.hpp"

namespace psychic_ui {
    DefaultButtonSkin::DefaultButtonSkin() :
        ButtonSkin() {
        setTag("DefaultButtonSkin");
        _label = add<Label>();
    }

    void DefaultButtonSkin::setLabel(const std::string &label) {
        _label->setText(label);
    }

}
