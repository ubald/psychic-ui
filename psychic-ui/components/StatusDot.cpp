#include "StatusDot.hpp"

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
