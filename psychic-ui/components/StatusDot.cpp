#include "StatusDot.hpp"
#include "Tone.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    StatusDot::StatusDot(const Tone tone) :
        Div() {
        tag("StatusDot");
        _defaultStyle.alignSelf(Align::Center)
            .shrink(0);
        updateTone(tone);
    }

    void StatusDot::updateTone(const Tone tone) {
        _tone = tone;
        applyTone(*this, tone);
    }
}
