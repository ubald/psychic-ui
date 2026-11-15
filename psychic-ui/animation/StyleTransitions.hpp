#pragma once

#include <vector>
#include "Animation.hpp"
#include "psychic-ui/style/Style.hpp"

namespace psychic_ui {
    class Div;

    namespace detail {
        /**
         * A style property on its way to a new value
         */
        struct RunningTransition {
            Property     property;
            Tween        tween;
            Length::Unit unit{Length::Unit::Px}; // A length's, kept from start to end
            bool         targetSet{true};        // Unset at the end: a color fading out to nothing
        };

        /**
         * A div's running style transitions, which its window ticks: owned by the div, which they write to
         */
        class StyleTransitions : public Animation {
        public:
            explicit StyleTransitions(Div *owner) : owner(owner) {}

            Div                            *owner; // Cleared when the div goes
            std::vector<RunningTransition> running{};

            const RunningTransition *find(Property property) const;

            bool advance(std::chrono::nanoseconds now) override;
            void finish() override;
        };

        /**
         * A property's value in a style as channels, an unset color as transparent; false when it doesn't
         * interpolate (not animatable, an auto length)
         */
        bool propertyChannels(const Style &style, Property property, Channels &channels, Length::Unit &unit);

        /**
         * Set a property from channels (a length in `unit`)
         */
        void setPropertyChannels(Style &style, Property property, const Channels &channels, Length::Unit unit);
    }
}
