#include "TextArea.hpp"

namespace psychic_ui {

    TextArea::TextArea(const std::string &text) :
        TextField<TextAreaSkin>(text, true) {
        tag("TextArea");
    }

}
