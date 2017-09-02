#include "Labels.hpp"
#include <psychicui/Label.hpp>

namespace psychicui {

    Labels::Labels():Component() {
        add(std::make_shared<Label>("Label"));
    }

}