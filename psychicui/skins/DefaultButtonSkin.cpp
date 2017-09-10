#include "DefaultButtonSkin.hpp"

namespace psychicui {
    DefaultButtonSkin::DefaultButtonSkin() :
        ButtonSkin() {
        _label = add<Label>();
    }

    void DefaultButtonSkin::setLabel(const std::string &label) {
        _label->setText(label);
    }

}
