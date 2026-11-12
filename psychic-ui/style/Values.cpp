#include <iomanip>
#include <unordered_set>
#include "Values.hpp"

namespace psychic_ui {

    namespace {
        struct StringHash {
            using is_transparent = void;

            std::size_t operator()(std::string_view string) const { return std::hash<std::string_view>{}(string); }
        };

        /**
         * Never destroyed, so that atoms in static objects stay valid until the very end.
         * Node-based, so the strings never move.
         */
        std::unordered_set<std::string, StringHash, std::equal_to<>> &internedStrings() {
            static auto *strings = new std::unordered_set<std::string, StringHash, std::equal_to<>>();
            return *strings;
        }

        const std::string *intern(std::string_view string) {
            auto &strings = internedStrings();
            auto it       = strings.find(string);
            if (it == strings.end()) {
                it = strings.emplace(string).first;
            }
            return &*it;
        }
    }

    Atom::Atom() {
        static const std::string *empty = intern("");
        _string = empty;
    }

    Atom::Atom(std::string_view string) :
        _string(intern(string)) {}

    std::ostream &operator<<(std::ostream &os, const Atom &atom) {
        return os << '"' << atom.str() << '"';
    }

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

    std::ostream &operator<<(std::ostream &os, const Color &color) {
        auto flags = os.flags();
        os << "argb(0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << color.argb() << ")";
        os.flags(flags);
        return os;
    }
}
