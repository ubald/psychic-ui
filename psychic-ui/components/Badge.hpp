#pragma once

#include <string>
#include "psychic-ui/Div.hpp"
#include "Label.hpp"
#include "Tone.hpp"

namespace psychic_ui {
    /**
     * @class Badge
     *
     * A short status or count in a pill outlined in its tone's color (`neutral`, `accent`, `success`, `warning`,
     * `danger` classes; see Tone), its text the `.label` part. 20 px; the `small` class makes it 16 px.
     *
     *     row.trailing().add<Badge>("Live", Tone::Success);
     */
    class Badge : public Div {
    public:
        explicit Badge(const std::string &label = "", Tone tone = Tone::Neutral);

        std::string label() const { return _label->text(); }

        template<class Self>
        Self &label(this Self &self, const std::string &label) {
            static_cast<Badge &>(self)._label->text(label);
            return self;
        }

        Tone tone() const { return _tone; }

        template<class Self>
        Self &tone(this Self &self, Tone tone) {
            static_cast<Badge &>(self).updateTone(tone);
            return self;
        }

    protected:
        Label *_label{nullptr}; // A child: our child list owns it
        Tone  _tone{Tone::Neutral};

        void updateTone(Tone tone);
    };
}
