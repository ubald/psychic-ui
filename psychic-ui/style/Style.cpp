#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "Style.hpp"

namespace psychic_ui {

    // region Transition

    Transition &Transition::add(const PropertyMask &properties, const Timing &timing) {
        if (properties.none()) {
            return *this;
        }
        // The properties leave their earlier timing; entries left without one go
        std::uint8_t kept = 0;
        for (std::uint8_t i = 0; i < _count; ++i) {
            Entry entry = _entries[i];
            entry.properties &= ~properties;
            if (entry.properties.any()) {
                _entries[kept++] = entry;
            }
        }
        for (std::uint8_t i = kept; i < _count; ++i) {
            _entries[i] = Entry{};
        }
        _count = kept;

        for (std::uint8_t i = 0; i < _count; ++i) {
            if (_entries[i].timing == timing) {
                _entries[i].properties |= properties;
                return *this;
            }
        }
        if (_count == capacity) {
            throw std::length_error("A transition holds at most " + std::to_string(capacity) + " timings");
        }
        _entries[_count++] = Entry{properties, timing};
        return *this;
    }

    const Timing *Transition::timing(const Property property) const {
        for (std::uint8_t i = 0; i < _count; ++i) {
            if (_entries[i].properties.test(index(property))) {
                return &_entries[i].timing;
            }
        }
        return nullptr;
    }

    PropertyMask Transition::properties() const {
        PropertyMask m;
        for (std::uint8_t i = 0; i < _count; ++i) {
            m |= _entries[i].properties;
        }
        return m;
    }

    std::ostream &operator<<(std::ostream &os, const Transition &transition) {
        if (transition.empty()) {
            return os << "none";
        }
        bool               first      = true;
        const PropertyMask properties = transition.properties();
        for (std::size_t i = 0; i < propertyCount; ++i) {
            if (properties.test(i)) {
                const auto property = static_cast<Property>(i);
                os << (first ? "" : ", ") << propertyName(property) << " " << *transition.timing(property);
                first = false;
            }
        }
        return os;
    }

    Style &Style::transition(const Property property, const Timing &timing) {
        return transition({property}, timing);
    }

    Style &Style::transition(const std::initializer_list<Property> properties, const Timing &timing) {
        Transition updated = _values.transition; // Empty while unset
        updated.add(mask(properties), timing);
        return transition(updated);
    }

    // endregion

    std::string_view propertyName(Property property) {
        switch (property) {
            #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) case Property::name: return #name;
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
                #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
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
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
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
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
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
        os << "{\n";
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
            if (has(Property::name)) { os << "    " #name ": "; printValue(os, _values.name); os << std::endl; }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X
        os << "}\n";
    }

    void Style::trace() const {
        trace(std::cout);
    }
}
