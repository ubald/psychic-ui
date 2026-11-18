#include "Tone.hpp"
#include <string>
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    void applyTone(Div &div, const Tone tone) {
        for (const Tone other: allTones) {
            if (other != tone) {
                div.removeClassName(std::string{toneClass(other)});
            }
        }
        div.addClassName(std::string{toneClass(tone)});
    }
}
