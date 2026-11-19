#pragma once

#include <format>
#include <string>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Inspector.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Splitter.hpp>

namespace psychic_ui {
    /**
     * Splitters: an editor's frame, a stage over a timeline beside a docked inspector, each boundary dragged, moved
     * by the arrows once focused, and reset by a double-click
     */
    class Splitters : public Div {
    public:
        Splitters();

    private:
        /**
         * A pane's placeholder content: its name and a line about it
         */
        static void describe(Div &pane, const std::string &name, const std::string &caption);
    };

    inline void Splitters::describe(Div &pane, const std::string &name, const std::string &caption) {
        pane.addClassName("demo-pane");
        pane.add<Label>(name).addClassName("h5");
        pane.add<Label>(caption).addClassName("text-tertiary");
    }

    inline Splitters::Splitters() : Div() {
        tag("Splitters")
            .classNames({"demo-panel"});

        add<Label>("Splitters").addClassName("h4");
        add<Label>("Drag a divider, or Tab to it and use the arrows (Shift for bigger steps); a double-click resets it. The inspector keeps its width as the window resizes.")
            .addClassName("text-tertiary");
        Label *event = &add<Label>("—").addClassName("event").addClassName("text-tertiary");

        auto &frame = add<Div>().addClassName("frame");

        // The inspector, kept, at the end; the stage and the timeline share the rest
        auto &workspace = frame.add<Splitter>()
                               .kept(SplitterPane::Second)
                               .defaultPosition(280)
                               .minimum(SplitterPane::First, 240)
                               .minimum(SplitterPane::Second, 200)
                               .onChange([event](float position) { event->text(std::format("inspector: {:.0f} px", position)); });

        auto &editor = workspace.first().add<Splitter>()
                                .orientation(Orientation::Vertical)
                                .kept(SplitterPane::Second)
                                .defaultPosition(140)
                                .minimum(SplitterPane::First, 120)
                                .minimum(SplitterPane::Second, 80)
                                .onChange([event](float position) { event->text(std::format("timeline: {:.0f} px", position)); });
        describe(editor.first(), "Stage", "Resizes with the window");
        describe(editor.second(), "Timeline", "Keeps its height, at least 80 px");

        auto &inspector = workspace.second().add<Inspector>().title("Moving head", "Fixture");
        auto &position  = inspector.section("Position");
        position.field("Pan", "180°");
        position.field("Tilt", "45°");
        auto &beam = inspector.section("Beam");
        beam.field("Zoom", "24°");
        beam.field("Focus", "60 %");
    }
}
