#include <string>
#include "Label.hpp"
#include "Text.hpp"

namespace psychic_ui {

    Label::Label(const std::string &text) :
        Text::Text(text) {
        tag("Label");
    }
}
