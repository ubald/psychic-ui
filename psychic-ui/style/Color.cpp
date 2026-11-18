#include <iomanip>
#include "Color.hpp"

namespace psychic_ui {

    std::ostream &operator<<(std::ostream &os, const Color &color) {
        auto flags = os.flags();
        os << "argb(0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << color.argb() << ")";
        os.flags(flags);
        return os;
    }
}
