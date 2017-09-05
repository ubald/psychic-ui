#include "SimpleButtonSkin.hpp"

namespace psychicui {
    SimpleButtonSkin::SimpleButtonSkin() :
        ButtonSkin() {
        _label = add<Label>();
    }

    void SimpleButtonSkin::setLabel(const std::string &label) {
        _label->setText(label);
    }

}
