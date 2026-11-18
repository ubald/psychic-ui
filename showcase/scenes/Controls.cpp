#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/ColorSwatches.hpp>
#include <psychic-ui/components/EmptyState.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/MenuBar.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/PageHeader.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/SearchField.hpp>
#include <psychic-ui/components/SegmentedControl.hpp>
#include <psychic-ui/components/Select.hpp>
#include <psychic-ui/components/Spacer.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/Tabs.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/ToolBar.hpp>
#include "../Compose.hpp"
#include "../Looks.hpp"
#include "../Scene.hpp"
#include "../Stage.hpp"

namespace psychic_ui::showcase {
    using namespace compose;

    namespace {
        /**
         * A small heading over a line of controls
         */
        Div &group(Div &parent, const std::string &name) {
            auto &group = parent.add<Div>().addClassName("stack").addClassName("tight");
            group.add<Label>(name).addClassName("h6");
            return line(group).addClassName("wrap");
        }

        void buttons(Window &window) {
            auto &panel = compose::panel(backdrop(window), "Buttons", "Variants are skins: the same component, drawn another way");

            auto &variants = group(panel, "Variants");
            variants.add<Button>("Add fixtures").icon("plus").skin("primary");
            variants.add<Button>("Secondary");
            variants.add<Button>("Ghost").skin("ghost");
            variants.add<Button>("Quiet").skin("quiet");
            variants.add<Button>("Remove").icon("trash").skin("danger");

            auto &sizes = group(panel, "Sizes and icons");
            sizes.add<Button>("Search").icon("search");
            sizes.add<Button>("Small").addClassName("small");
            sizes.add<Button>("Upload").icon("arrow-up").addClassName("small").skin("quiet");
            sizes.add<Button>().icon("more").addClassName("icon-button");
            sizes.add<Button>().icon("sidebar").addClassName("icon-button");
            sizes.add<Button>().icon("settings").addClassName("icon-button");

            auto &states = group(panel, "Toggles and states");
            states.add<Button>("Toggle").toggle(true);
            states.add<Button>("Selected").toggle(true).selected(true);
            states.add<Button>("Disabled").enabled(false);
            states.add<Button>("Primary, disabled").skin("primary").enabled(false);

            auto &checks = group(panel, "Check boxes");
            checks.add<CheckBox>("Invert pan");
            checks.add<CheckBox>("Include in blackout").checked(true);
            checks.add<CheckBox>("Disabled").enabled(false);
        }

        void inputs(Window &window) {
            auto &stage = backdrop(window).addClassName("row");

            auto &fixture = panel(stage, "Fixture", "Wash R2 · moving head");
            fixture.width(310);
            field(fixture, "Name").add<TextInput>("Wash R2").grow(1);
            field(fixture, "Find").add<SearchField>("Moving head").grow(1);
            field(fixture, "View").add<SegmentedControl>(std::vector<Segment>{{"Day"}, {"Week"}, {"Month"}}).selected(1);
            field(fixture, "Curve").add<Select>(std::vector<std::string>{"Linear", "Ease in", "Ease out", "Ease in and out"}, 3).grow(1);
            field(fixture, "Universe").add<NumberStepper>(3).range(1, 12);

            auto &look = panel(stage, "Look", "Colour, level and behaviour");
            look.width(290);
            field(look, "Intensity").add<Range<int>>(0, 100, 72, nullptr).grow(1);
            field(look, "Speed").add<Range<int>>(0, 100, 40, nullptr).grow(1);
            field(look, "Colour").add<ColorSwatches>(std::vector{rgb(0xE5484D), rgb(0xF2994A), rgb(0xF2C94C), rgb(0x4CB782), rgb(0x4B8BF5), rgb(0x5E6AD2)}).value(rgb(0x5E6AD2));
            field(look, "Blackout").add<Switch>().checked(true);
            field(look, "Follow").add<Switch>();
            field(look, "Options").add<CheckBox>("Include in chases").checked(true);
        }

        void chrome(Window &window) {
            auto &panel = compose::panel(backdrop(window)).addClassName("flush").addClassName("chrome");
            auto &header = panel.add<PageHeader>("Opening night", "list-detail");
            header.add<MenuBar>(std::vector<std::shared_ptr<MenuItem>>{
                Menu::item("File", MenuItems{Menu::item("Save", [] {}, "Ctrl+S")}),
                Menu::item("Edit", MenuItems{Menu::item("Copy", [] {}, "Ctrl+C")}),
                Menu::item("View", MenuItems{Menu::item("Zoom in", [] {}, "Ctrl++")}),
                Menu::item("Help", [] {}),
            });
            header.add<Spacer>();
            header.add<Button>("Share").icon("link").addClassName("small");

            panel.add<Tabs<>>(
                std::vector<std::string>{"Cues", "Fixtures", "Palettes", "Media"},
                [](const std::string &name) { return name; }
            ).select("Cues");

            auto &tools = panel.add<ToolBar>();
            tools.add<Button>("New").icon("plus");
            tools.add<Button>("Start").icon("play");
            tools.add<Button>("Stop").icon("pause");
            tools.add<Spacer>();
            tools.add<Button>("Help").icon("question");

            auto &body = panel.add<Div>().addClassName("chrome-body");
            body.add<EmptyState>("inbox", "No cues yet", "Cues record the stage as it is, so a show can play it back.")
                .action("Record a cue").icon("plus");
        }

        void focus(Window &window) {
            auto &panel = compose::panel(backdrop(window), "Keyboard", "Tab moves the focus; only the keyboard shows its ring");
            panel.width(440);
            field(panel, "Search").add<SearchField>().placeholder("Find fixtures").grow(1);
            field(panel, "Theme").add<SegmentedControl>(std::vector<Segment>{{"System", "monitor"}, {"Light", "sun"}, {"Dark", "moon"}});
            field(panel, "Blackout").add<Switch>().checked(true);
            field(panel, "Options").add<CheckBox>("Confirm before deleting");
            panel.add<Div>().addClassName("divider");
            auto &actions = line(panel);
            actions.add<Spacer>();
            actions.add<Button>("Cancel");
            actions.add<Button>("Save").skin("primary");
        }
    }

    std::vector<Scene> controlScenes() {
        std::vector<Scene> scenes{};
        const auto         add = [&scenes](const std::vector<Scene> &more) {
            scenes.insert(scenes.end(), more.begin(), more.end());
        };

        add(inEveryLook({
            .name = "buttons",
            .title = "Buttons: variants, sizes, icon buttons, toggles, check boxes",
            .width = 680,
            .height = 460,
            .build = buttons,
        }));

        add(inEveryLook({
            .name = "inputs",
            .title = "Inputs: text, search, segmented control, select, stepper, sliders, swatches, switches",
            .width = 680,
            .height = 460,
            .build = inputs,
        }));

        add(inEveryLook({
            .name = "chrome",
            .title = "App chrome: a page header with a menu bar, tabs, a tool bar and an empty state",
            .width = 680,
            .height = 460,
            .build = chrome,
        }));

        add(inEveryLook({
            .name = "focus",
            .title = "Keyboard focus: Tab to the segmented control, then Right",
            .width = 680,
            .height = 460,
            .build = focus,
            .pose = [](Stage &stage) {
                // The search field, then the segmented control, whose selection the arrows move
                stage.key(Key::TAB);
                stage.key(Key::TAB);
                stage.key(Key::RIGHT);
            },
        }));

        return scenes;
    }
}
