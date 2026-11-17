#pragma once

#include <vector>
#include "Animation.hpp"
#include "psychic-ui/style/Style.hpp"

namespace psychic_ui {
    class Div;

    namespace detail {
        /**
         * A style property on its way to its value in `StyleTransitions::targets`
         */
        struct RunningTransition {
            Property property;
            Tween    tween;
            bool     targetSet{true}; // Unset at the end: a color fading out to nothing
        };

        /**
         * A div's running style transitions, which its window ticks: owned by the div, which they write to
         */
        class StyleTransitions : public Animation {
        public:
            explicit StyleTransitions(Div *owner) : owner(owner) {}

            Div                            *owner; // Cleared when the div goes
            std::vector<RunningTransition> running{};
            Style                          targets{}; // Where the running ones go (a length in the unit it moves in)

            const RunningTransition *find(Property property) const;

            bool advance(std::chrono::nanoseconds now) override;
            void finish() override;
        };
    }
}
