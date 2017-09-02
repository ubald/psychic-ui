#include "Buttons.hpp"
#include <psychicui/Button.hpp>

namespace psychicui {

    Buttons::Buttons():Component() {
        add(std::make_shared<Button>("Button"));
    }

}