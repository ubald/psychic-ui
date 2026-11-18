#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include "Atom.hpp"
#include "../utils/NoDestructor.hpp"

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
            static NoDestructor<std::unordered_set<std::string, StringHash, std::equal_to<>>> strings{};
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
}
