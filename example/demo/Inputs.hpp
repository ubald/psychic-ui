#pragma once

#include <format>
#include <string>
#include <tuple>
#include <vector>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/ColorSwatches.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/SearchField.hpp>
#include <psychic-ui/components/SegmentedControl.hpp>
#include <psychic-ui/components/Switch.hpp>

namespace psychic_ui {
    /**
     * The inputs: search fields, switches, segmented controls, number steppers and color swatches, each with what
     * it last reported
     */
    class Inputs : public Div {
    public:
        Inputs();

    private:
        /**
         * A titled group, with a line of explanation under its title, a row for its controls, and a line showing
         * the last event
         */
        Div &group(const std::string &title, const std::string &caption, Label *&event);
    };

    Div &Inputs::group(const std::string &title, const std::string &caption, Label *&event) {
        auto &group = add<VBox>().addClassName("group");
        group.add<Label>(title).addClassName("h4");
        group.add<Label>(caption).role(TextRole::Tertiary);
        auto &row = group.add<HBox>().addClassName("row");
        event = &group.add<Label>("—").addClassName("event").role(TextRole::Tertiary);
        return row;
    }

    Inputs::Inputs() : Div() {
        tag("Inputs")
            .classNames({"demo-panel"});

        Label *searchEvent = nullptr;
        auto  &searches    = group("Search fields", "Escape clears, Enter submits; the clear button shows once there is text", searchEvent);
        searches.add<SearchField>()
                .onChange([searchEvent](const std::string &text) { searchEvent->text("changed: \"" + text + "\""); })
                .onSubmit([searchEvent](const std::string &text) { searchEvent->text("submitted: \"" + text + "\""); });
        searches.add<SearchField>("Moving head").placeholder("Find fixtures");

        Label *switchEvent = nullptr;
        auto  &switches    = group("Switches", "A click, Space or Enter flips one, the thumb springs over; it also drags", switchEvent);
        for (const auto &[label, on, enabled]: std::vector<std::tuple<std::string, bool, bool>>{
                 {"Follow system", false, true},
                 {"Blackout on start", true, true},
                 {"Disabled", false, false},
                 {"On, disabled", true, false},
             }) {
            switches.add<Switch>(label)
                    .checked(on)
                    .enabled(enabled)
                    .onChange([switchEvent, label](bool checked) {
                        switchEvent->text(std::format("{}: {}", label, checked ? "on" : "off"));
                    });
        }

        Label *segmentEvent = nullptr;
        auto  &segments     = group("Segmented controls", "Arrows move the selection once focused; 28 px, and 24 px with the small class", segmentEvent);
        segments.add<SegmentedControl>(std::vector<Segment>{{"Day"}, {"Week"}, {"Month"}})
                .selected(1)
                .onChange([segmentEvent](int index) { segmentEvent->text(std::format("view: {}", index)); });
        segments.add<SegmentedControl>(std::vector<Segment>{{"System", "monitor"}, {"Light", "sun"}, {"Dark", "moon"}})
                .onChange([segmentEvent](int index) { segmentEvent->text(std::format("theme: {}", index)); });
        segments.add<SegmentedControl>(std::vector<Segment>{{"", "list"}, {"", "table"}, {"", "layout"}})
                .addClassName("small")
                .selected(2)
                .onChange([segmentEvent](int index) { segmentEvent->text(std::format("layout: {}", index)); });

        Label *stepperEvent = nullptr;
        auto  &steppers     = group("Number steppers", "Type a number and press Enter; Up and Down step it, Shift by ten", stepperEvent);
        steppers.add<NumberStepper>(3)
                .range(0, 12)
                .onChange([stepperEvent](double value) { stepperEvent->text(std::format("universes: {}", value)); });
        steppers.add<NumberStepper>(0)
                .range(0, 100)
                .onChange([stepperEvent](double value) { stepperEvent->text(std::format("at the minimum: {}", value)); });
        steppers.add<NumberStepper>(0.5)
                .range(0, 1)
                .step(0.05)
                .onChange([stepperEvent](double value) { stepperEvent->text(std::format("fade: {:.2f}", value)); });
        steppers.add<NumberStepper>(24)
                .range(1, 512)
                .addClassName("small");
        steppers.add<NumberStepper>(8).enabled(false);

        Label *swatchEvent = nullptr;
        auto  &swatches    = group("Color swatches", "Pivotal's palette; a click picks, the arrows move the focus and Space or Enter picks", swatchEvent);
        swatches.add<ColorSwatches>()
                .value(rgb(0x5E6AD2))
                .onChange([swatchEvent](Color color) {
                    swatchEvent->text(std::format("color: #{:06X}", color.argb() & 0xFFFFFFu));
                });
        swatches.add<ColorSwatches>(std::vector{rgb(0xE5484D), rgb(0x4CB782), rgb(0x4B8BF5)})
                .onChange([swatchEvent](Color color) {
                    swatchEvent->text(std::format("short palette: #{:06X}", color.argb() & 0xFFFFFFu));
                });
    }
}
