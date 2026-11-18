#include "Length.hpp"

namespace psychic_ui {

    std::ostream &operator<<(std::ostream &os, const Length &length) {
        switch (length.unit) {
            case Length::Unit::Px:
                return os << length.value << "px";
            case Length::Unit::Percent:
                return os << length.value << "%";
            default:
                return os << "auto";
        }
    }
}
