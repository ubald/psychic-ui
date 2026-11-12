#include <iostream>
#include <type_traits>
#include "Style.hpp"

namespace psychic_ui {

    std::string_view propertyName(Property property) {
        switch (property) {
            #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) case Property::name: return #name;
            PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
            #undef PSYCHIC_UI_X
        }
        return "?";
    }

    Style &Style::operator=(const Style &other) {
        if (this != &other && !(*this == other)) {
            _set    = other._set;
            _values = other._values;
            changed();
        }
        return *this;
    }

    Style &Style::unset(Property property) {
        const std::size_t i = index(property);
        if (_set.test(i)) {
            static const StyleValues defaults{};
            switch (property) {
                #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) \
                    case Property::name: _values.name = defaults.name; break;
                PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
                #undef PSYCHIC_UI_X
            }
            _set.reset(i);
            changed();
        }
        return *this;
    }

    Style &Style::clear() {
        if (_set.any()) {
            _set.reset();
            _values = StyleValues{};
            changed();
        }
        return *this;
    }

    Style &Style::overlay(const Style &style, const PropertyMask &only) {
        const PropertyMask copied = style._set & only;
        if (copied.none()) {
            return *this;
        }
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) \
            if (copied.test(index(Property::name))) { _values.name = style._values.name; }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        _set |= copied;
        changed();
        return *this;
    }

    Style &Style::defaults(const Style &style) {
        return overlay(style, ~_set);
    }

    PropertyMask Style::diff(const Style &other) const {
        PropertyMask changed = _set ^ other._set;
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) \
            if (!(_values.name == other._values.name)) { changed.set(index(Property::name)); }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        return changed;
    }

    namespace {
        template<class T>
        void printValue(std::ostream &os, const T &value) {
            if constexpr (std::is_enum_v<T>) {
                os << +static_cast<std::underlying_type_t<T>>(value);
            } else if constexpr (std::is_same_v<T, bool>) {
                os << (value ? "true" : "false");
            } else {
                os << value;
            }
        }
    }

    void Style::trace(std::ostream &os) const {
        os << "{" << std::endl;
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout) \
            if (has(Property::name)) { os << "    " #name ": "; printValue(os, _values.name); os << std::endl; }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        os << "}" << std::endl;
    }

    void Style::trace() const {
        trace(std::cout);
    }
}
