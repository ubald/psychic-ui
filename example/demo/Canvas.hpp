#pragma once

#include <string>
#include <vector>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Badge.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/ColorSwatches.hpp>
#include <psychic-ui/components/Icon.hpp>
#include <psychic-ui/components/Inspector.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/ToolBar.hpp>
#include <psychic-ui/components/Tree.hpp>

namespace psychic_ui {

    /**
     * A design tool's canvas, as on the Float board: the page is a dotted canvas (`dotGridColor`), with panels
     * floating over it, positioned absolutely: the document's name and its layers on the left, an inspector on
     * the right, a tool bar at the bottom, and a stage in the middle. Library components only; the rules are the
     * demo's (DemoStyleSheet, `CanvasPage`).
     */
    class CanvasPage : public Div {
    public:
        CanvasPage();

    private:
        std::vector<Button *> _tools{};

        void pick(Button *tool);
    };

    inline CanvasPage::CanvasPage() : Div() {
        tag("CanvasPage");

        // region Stage, centered under everything

        auto &middle = add<Div>().addClassName("fill");
        auto &stage  = middle.add<Div>().addClassName("stage");
        stage.add<Div>().addClassName("truss");
        struct Fixture {
            float x;
            float y;
            Color color;
        };
        const Color amber = rgb(0xFFC98A);
        const Color pink  = rgb(0xF56E9B);
        const Color blue  = rgb(0x8DB2FF);
        for (const Fixture &f: std::vector<Fixture>{
                 {84, 68, amber}, {159, 68, amber}, {308, 68, amber}, {383, 68, amber},
                 {233, 179, pink}, {122, 245, blue}, {346, 245, blue},
             }) {
            stage.add<Div>()
                 .addClassName("fixture")
                 .left(f.x - 14)
                 .top(f.y - 14)
                 .backgroundColor(f.color);
        }
        auto &selection = stage.add<Div>().addClassName("selection");
        selection.add<Label>("2 fixtures").addClassName("selection-label");
        stage.add<Label>("Stage · 12 × 8 m").addClassName("caption");

        // endregion

        // region The document and its layers

        auto &left = add<VBox>().addClassName("left");

        auto &document = left.add<HBox>().addClassName("floating").addClassName("document");
        document.add<Label>("P").addClassName("mark");
        document.add<Label>("Main stage").addClassName("name");
        document.add<Icon>("chevron-down");

        auto &layers = left.add<VBox>().addClassName("floating").addClassName("layers");
        layers.add<Label>("Layers").addClassName("heading");
        auto &tree      = layers.add<Tree>();
        auto &mainStage = tree.node("Main stage", "hash").expanded(true);
        auto &stageLeft = mainStage.node("Stage left", "folder").expanded(true);
        stageLeft.node("Wash L1", "lightbulb");
        stageLeft.node("Wash L2", "lightbulb");
        auto &stageRight = mainStage.node("Stage right", "folder").expanded(true);
        stageRight.node("Wash R1", "lightbulb");
        auto &washR2 = stageRight.node("Wash R2", "lightbulb");
        mainStage.node("Strobe chase", "chart");
        tree.select(&washR2.row());

        layers.add<Label>("Palettes").addClassName("heading");
        auto &palettes = layers.add<Tree>();
        palettes.node("Warm amber", "sliders");
        palettes.node("Cold blue", "sliders");

        // endregion

        // region Inspector

        auto &inspector = add<Inspector>()
            .addClassName("floating")
            .addClassName("inspector")
            .title("2 fixtures", "Wash R1, Wash R2")
            .icon("lightbulb");

        auto &position = inspector.section("Position");
        position.field("X").value().add<NumberStepper>(8.4).decimals(1).step(0.1).addClassName("small");
        position.field("Y").value().add<NumberStepper>(1.2).decimals(1).step(0.1).addClassName("small");

        auto &look = inspector.section("Look");
        look.field("Intensity").value().add<Range<int>>(0, 100, 72, nullptr).grow(1);
        look.field("Colour").value()
            .add<ColorSwatches>(std::vector{amber, pink, blue, rgb(0x7B6CFF)})
            .selected(0);
        look.field("Blackout").value().add<Switch>().checked(true);
        auto &tags = look.field("Tags").value();
        tags.add<Badge>("Wash", Tone::Accent);
        tags.add<Badge>("On", Tone::Success);

        auto &actions = inspector.add<HBox>().addClassName("actions");
        actions.add<Button>("Locate").grow(1);
        actions.add<Button>("Cue").icon("plus").skin("primary").grow(1);

        // endregion

        // region Tool bar, centered at the bottom

        auto &dock  = add<HBox>().addClassName("dock");
        auto &tools = dock.add<ToolBar>().addClassName("floating");
        for (const auto &[icon, name]: std::vector<std::pair<std::string, std::string>>{
                 {"mouse-pointer-2", "Select"},
                 {"hash", "Frame"},
                 {"lightbulb", "Fixture"},
                 {"type", "Text"},
                 {"pencil", "Draw"},
             }) {
            auto &tool = tools.add<Button>().icon(icon).tooltip(name).toggle(true).autoToggle(false);
            tool.onClick([this, &tool]() { pick(&tool); });
            _tools.push_back(&tool);
        }
        pick(_tools.front());

        // endregion
    }

    inline void CanvasPage::pick(Button *tool) {
        // One tool at a time, as a radio group
        for (Button *other: _tools) {
            other->selected(other == tool);
        }
    }
}
