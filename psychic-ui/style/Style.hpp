#pragma once

#include <iosfwd>
#include "Properties.hpp"

namespace psychic_ui {

    /**
     * A set of style values: a stylesheet rule, a Div's inline or default style, or a computed style.
     *
     * Each property has a typed getter and a chainable setter, generated from `PSYCHIC_UI_STYLE_PROPERTIES`:
     *
     *     manager->style("Button").padding(12).color(rgb(0x9DA5B5)).cursor(Cursor::Hand);
     *     float size = div->computedStyle().fontSize();
     *
     * A property is either set or not; the getter returns the default (see the table) when it isn't.
     * Storage is flat (one field per property plus a bit mask), so copying, overlaying and comparing styles
     * never allocate.
     */
    class Style {
    public:
        using ChangedCallback = void (*)(void *owner);

        Style() = default;

        /**
         * A style that notifies its owner when a setter changes it
         */
        Style(ChangedCallback onChanged, void *owner) :
            _onChanged(onChanged), _owner(owner) {}

        /**
         * Copies the values, not the owner
         */
        Style(const Style &other) :
            _set(other._set), _values(other._values) {}

        /**
         * Copies the values, keeps the owner (notified if anything changed)
         */
        Style &operator=(const Style &other);

        /**
         * Moving is copying: the values are plain data, and the owner stays where it is
         */
        Style(Style &&other) noexcept : Style(other) {}

        Style &operator=(Style &&other) noexcept { return *this = other; }

        ~Style() = default;

        // region Properties

        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
            const Type &name() const { return _values.name; } \
            Style &name(Argument value) { return assign(Property::name, _values.name, detail::toValue(value)); }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X

        /**
         * Add to the transition: these properties blend to their new values with this timing (see `Transition`).
         * In a rule, the transition only applies when that rule's transition wins the cascade.
         */
        Style &transition(Property property, const Timing &timing);
        Style &transition(std::initializer_list<Property> properties, const Timing &timing);

        // endregion

        /**
         * Whether the property is set
         */
        bool has(Property property) const { return _set.test(index(property)); }

        /**
         * The set properties
         */
        const PropertyMask &properties() const { return _set; }

        /**
         * Unset a property, its getter returns the default again
         */
        Style &unset(Property property);

        /**
         * Unset every property
         */
        Style &clear();

        /**
         * Set every property `style` sets, or only those in `only`
         */
        Style &overlay(const Style &style, const PropertyMask &only = PropertyMask{}.set());

        /**
         * Set the properties `style` sets and this one doesn't
         */
        Style &defaults(const Style &style);

        /**
         * The properties whose value differs, or that are set in only one of the two styles
         */
        PropertyMask diff(const Style &other) const;

        bool operator==(const Style &other) const { return _set == other._set && _values == other._values; }

        /**
         * Print the set properties
         */
        void trace(std::ostream &os) const;
        void trace() const;

    private:
        PropertyMask    _set{};
        StyleValues     _values{};
        ChangedCallback _onChanged{nullptr};
        void            *_owner{nullptr};

        void changed() const {
            if (_onChanged) {
                _onChanged(_owner);
            }
        }

        template<class T>
        Style &assign(Property property, T &field, const T &value) {
            const std::size_t i = index(property);
            if (!_set.test(i) || !(field == value)) {
                field = value;
                _set.set(i);
                changed();
            }
            return *this;
        }
    };
}
