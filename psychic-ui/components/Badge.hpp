#pragma once

#include <string>
#include "psychic-ui/Div.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "Label.hpp"
#include "Tone.hpp"

namespace psychic_ui {
    class Icon;

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

        const std::string &label() const { return _label->text(); }

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
        Label *_label{nullptr}; // A child, which we own
        Tone  _tone{Tone::Neutral};

        void updateTone(Tone tone);
    };

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

    /**
     * @class Chip
     *
     * A value or filter as a pill: an optional `.icon`, the `.label`, and, when removable, a `.remove` x that
     * dismisses it (so do Backspace and Delete while it has the focus). Neutral chips are quiet (secondary text in a
     * hairline, raised under the mouse or while `selected`); the other tones tint their text and outline.
     * It takes the focus; clicking it (or Enter or Space) emits `clicked`.
     *
     * Its dismissal is `onDismiss`/`dismissed`, not `onRemove`/`removed`, which is the hierarchy's `removed()`.
     * Dismissing doesn't take the chip away: the application does, in its callback.
     *
     *     filters.add<Chip>("Status: Open").removable(true).onDismiss([&] { ... });
     */
    class Chip : public Div {
    public:
        explicit Chip(const std::string &label = "", Tone tone = Tone::Neutral);

        const std::string &label() const { return _label->text(); }

        template<class Self>
        Self &label(this Self &self, const std::string &label) {
            static_cast<Chip &>(self)._label->text(label);
            return self;
        }

        Tone tone() const { return _tone; }

        template<class Self>
        Self &tone(this Self &self, Tone tone) {
            static_cast<Chip &>(self).updateTone(tone);
            return self;
        }

        /**
         * The icon before the label, by its registered name; "" for none
         */
        template<class Self>
        Self &icon(this Self &self, detail::StringArg icon) {
            static_cast<Chip &>(self).updateIcon(icon.value);
            return self;
        }

        bool removable() const { return _removable; }

        /**
         * Whether it shows the x that dismisses it
         */
        template<class Self>
        Self &removable(this Self &self, detail::BoolArg removable) {
            static_cast<Chip &>(self).updateRemovable(removable.value);
            return self;
        }

        bool selected() const { return _selected; }

        /**
         * Whether it is chosen, as a filter that applies: raised (the `selected` class)
         */
        template<class Self>
        Self &selected(this Self &self, detail::BoolArg selected) {
            static_cast<Chip &>(self).updateSelected(selected.value);
            return self;
        }

        /**
         * Dismiss it, as its x does: emits `dismissed` when it is removable
         */
        void dismiss();

        /**
         * Its x was clicked, or Backspace or Delete pressed on it
         */
        Signal<> dismissed{};
        PSYCHIC_UI_EVENT(onDismiss, dismissed)

    protected:
        // Children, which we own
        Icon  *_icon{nullptr};
        Label *_label{nullptr};
        Icon  *_remove{nullptr};
        Tone  _tone{Tone::Neutral};
        bool  _removable{false};
        bool  _selected{false};

        void updateTone(Tone tone);
        void updateIcon(std::string_view icon);
        void updateRemovable(bool removable);
        void updateSelected(bool selected);
    };
}
