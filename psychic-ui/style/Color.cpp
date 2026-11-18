#include <iomanip>
#include <ios>
#include <ostream>
#include "Color.hpp"

namespace psychic_ui {

    std::ostream &operator<<(std::ostream &os, const Color &color) {
        constexpr int hexDigits = 8; // Two per channel
        auto          flags     = os.flags();
        os << "argb(0x" << std::hex << std::uppercase << std::setw(hexDigits) << std::setfill('0') << color.argb() << ")";
        os.flags(flags);
        return os;
    }
}
