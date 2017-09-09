#include "DefaultCheckBoxSkin.hpp"

namespace psychicui {
    DefaultCheckBoxSkin::DefaultCheckBoxSkin() :
        CheckBoxSkin() {
        _label = add<Label>();
    }

    void DefaultCheckBoxSkin::setLabel(const std::string &label) {
        _label->setText(label);
    }

}
