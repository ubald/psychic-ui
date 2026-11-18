#pragma once

#include <string>
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    class Icon;
    class Label;

    /**
     * @class InspectorField
     *
     * A label/value row of an inspector section: the `.name` (a fixed width, muted; not `.label`, which would also
     * match the labels of what the value holds), then the `.value`, which shows its text (`.text`) or holds what is
     * added to `value()` (a check box, a badge, a field)
     */
    class InspectorField : public Div {
    public:
        explicit InspectorField(const std::string &label = "", const std::string &text = "");

        std::string label() const;

        template<class Self>
        Self &label(this Self &self, const std::string &label) {
            static_cast<InspectorField &>(self).updateLabel(label);
            return self;
        }

        std::string text() const;

        /**
         * The value as text; "" to show only what is added to `value()`
         */
        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<InspectorField &>(self).updateText(text);
            return self;
        }

        /**
         * The `.value` part, for controls: `field.value().add<CheckBox>()`
         */
        Div &value() const { return *_value; }

    protected:
        // Children: our child list owns them
        Label *_label{nullptr};
        Div   *_value{nullptr};
        Label *_text{nullptr};

        void updateLabel(const std::string &label);
        void updateText(const std::string &text);
    };

    /**
     * @class InspectorSection
     *
     * A group of an inspector's fields under a `.header` (its `.title`, then what is added to `header()`), its
     * fields in the `.body`. Sections are set apart by a hairline.
     */
    class InspectorSection : public Div {
    public:
        explicit InspectorSection(const std::string &title = "");

        std::string title() const;

        /**
         * Add a field: `body().add<InspectorField>(label, text)`
         */
        InspectorField &field(const std::string &label, const std::string &text = "");

        Div &header() const { return *_header; }
        Div &body() const { return *_body; }

    protected:
        // Children: our child list owns them
        Div   *_header{nullptr};
        Label *_title{nullptr};
        Div   *_body{nullptr};
    };

    /**
     * @class Inspector
     *
     * A property panel on the surface layer: an optional `.heading` (its `.media` icon, `.title` and `.subtitle`),
     * then sections of label/value fields in two columns
     *
     *     auto &inspector = add<Inspector>().title("Cue 12", "Fade, 3 s").icon("play");
     *     auto &timing    = inspector.section("Timing");
     *     timing.field("Duration", "3.0 s");
     *     timing.field("Loop").value().add<CheckBox>();
     */
    class Inspector : public Div {
    public:
        Inspector();

        /**
         * The heading's title and subtitle ("" for none); no heading while both are empty
         */
        template<class Self>
        Self &title(this Self &self, const std::string &title, const std::string &subtitle = "") {
            static_cast<Inspector &>(self).updateTitle(title, subtitle);
            return self;
        }

        /**
         * The heading's icon, by its registered name; "" for none
         */
        template<class Self>
        Self &icon(this Self &self, detail::StringArg icon) {
            static_cast<Inspector &>(self).updateIcon(icon.value);
            return self;
        }

        /**
         * Add a section, under an optional title
         */
        InspectorSection &section(const std::string &title = "");

    protected:
        // Children: our child list owns them
        Div   *_heading{nullptr};
        Div   *_media{nullptr};
        Icon  *_icon{nullptr};
        Label *_title{nullptr};
        Label *_subtitle{nullptr};

        void updateTitle(const std::string &title, const std::string &subtitle);
        void updateIcon(std::string_view icon);
    };
}
