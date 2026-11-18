#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace psychic_ui {

    /**
     * An interned string, for the open sets of values (font family, skin name).
     * Copying and comparing are pointer operations, and only the first use of a spelling allocates.
     * Interned strings are never freed; like the rest of the UI, Atoms are for the UI thread only.
     */
    class Atom {
    public:
        Atom();
        explicit Atom(std::string_view string);

        const std::string &str() const { return *_string; }

        const char *c_str() const { return _string->c_str(); }

        bool empty() const { return _string->empty(); }

        bool operator==(const Atom &other) const { return _string == other._string; }

        bool operator==(std::string_view other) const { return *_string == other; }

    private:
        const std::string *_string;
    };

    std::ostream &operator<<(std::ostream &os, const Atom &atom);
}
