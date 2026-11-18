#include <string>
#include "Badge.hpp"
#include "Label.hpp"
#include "Tone.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    Badge::Badge(const std::string &label, const Tone tone) :
        Div() {
        tag("Badge");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center)
            .alignSelf(Align::Center)
            .shrink(0);
        _label = &add<Label>(label);
        _label->addClassName("label");
        updateTone(tone);
    }

    void Badge::updateTone(const Tone tone) {
        _tone = tone;
        applyTone(*this, tone);
    }
}
