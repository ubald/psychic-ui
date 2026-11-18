#pragma once

#include "psychic-ui/Div.hpp"
#include "Tone.hpp"

namespace psychic_ui {
    /**
     * @class StatusDot
     *
     * A 6 px dot in its tone's color, for a status ("Connected", "Needs attention"); `Neutral` is muted
     */
    class StatusDot : public Div {
    public:
        explicit StatusDot(Tone tone = Tone::Neutral);

        Tone tone() const { return _tone; }

        template<class Self>
        Self &tone(this Self &self, Tone tone) {
            static_cast<StatusDot &>(self).updateTone(tone);
            return self;
        }

    protected:
        Tone _tone{Tone::Neutral};

        void updateTone(Tone tone);
    };
}
