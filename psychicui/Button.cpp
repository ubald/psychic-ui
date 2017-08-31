#include <iostream>
#include "Button.hpp"

namespace psychicui {
    Button::Button(const std::string &label)
        : Component() {
        setTag("Button");
        //TODO: Implement a createChildren method that'll be universal, whatever the constructor used
        _label = std::make_shared<Label>(label);
        // TODO: _label->style()->setValue(position, "absolute");
        addChild(_label);
    }

    std::string Button::label() {
        return _label->text();
    }

    void Button::setLabel(std::string label) {
        _label->setText(label);
    }

}