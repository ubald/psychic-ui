#include "Label.hpp"

namespace psychic_ui {

    Label::Label(const std::string &text) :
        Text::Text(text) {
        tag("Label");
    }
}
