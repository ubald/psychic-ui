#include <memory>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/Icon.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/SegmentedControl.hpp>
#include <psychic-ui/components/Spacer.hpp>
#include <psychic-ui/components/StatusDot.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/icons/IconRegistry.hpp>
#include "../Compose.hpp"
#include "../Looks.hpp"
#include "../Scene.hpp"
#include "../Stage.hpp"

namespace psychic_ui::showcase {
    using namespace compose;

    namespace {
        void typography(Window &window) {
            auto &stage = backdrop(window).addClassName("row");

            auto &scale = panel(stage).addClassName("type-scale");
            scale.add<Label>("Heading one").addClassName("h1");
            scale.add<Label>("Heading two").addClassName("h2");
            scale.add<Label>("Heading three").addClassName("h3");
            scale.add<Label>("Heading four").addClassName("h4");
            scale.add<Label>("Heading five").addClassName("h5");
            scale.add<Label>("Heading six").addClassName("h6");
            scale.add<Div>().addClassName("divider");
            scale.add<Label>("Primary text");
            scale.add<Label>("Secondary text").addClassName("text-secondary");
            scale.add<Label>("Tertiary text").addClassName("text-tertiary");

            auto &prose = panel(stage, "Text", "Shaped and laid out by SkParagraph").addClassName("type-prose");
            prose.add<Text>(
                "Every line is exactly the line height. Text wraps to the width its layout gives it, "
                "measured at the widths the layout asks about."
            ).multiline(true).addClassName("text-secondary");
            prose.add<Label>("One line too long for its box ends in an ellipsis, cut after its last whole character").addClassName("ellipsis");
            auto &scripts = prose.add<Div>().addClassName("stack").addClassName("tight");
            for (const char *sample: {"Français : Brébœuf, déjà-vu", "Ελληνικά: ευχαριστώ", "Русский: спасибо", "Fractions ½ · arrows → ↵ · ⌘ ⌥ ⇧"}) {
                scripts.add<Label>(sample);
            }
        }

        void icons(Window &window) {
            auto &panel = compose::panel(backdrop(window), "Icons", "Vector paths as data, one stroke weight at any size");
            auto &grid  = panel.add<Div>().addClassName("icon-grid");
            int   count = 0;
            for (const std::string_view name: IconRegistry::shared().names()) {
                if (count++ == 84) {
                    break; // Seven rows of twelve
                }
                grid.add<Div>().addClassName("icon-tile").add<Icon>(name);
            }
        }

        void density(Window &window) {
            auto &panel = compose::panel(backdrop(window), "Fixture", "Wash R2");
            panel.addClassName("density");
            field(panel, "View").add<SegmentedControl>(std::vector<Segment>{{"Day"}, {"Week"}, {"Month"}}).selected(1);
            field(panel, "Universe").add<NumberStepper>(3).range(1, 12);
            field(panel, "Intensity").add<Range<int>>(0, 100, 72, nullptr).grow(1);
            field(panel, "Blackout").add<Switch>().checked(true);
            field(panel, "Options").add<CheckBox>("Follow on").checked(true);
            auto &actions = line(panel);
            actions.add<Spacer>();
            actions.add<Button>("Locate");
            actions.add<Button>("Cue").icon("plus").skin("primary");
        }

        /**
         * The fixtures a sorting grid shows
         */
        DataGrid &fixtureGrid(Window &window) {
            auto &card = panel(backdrop(window)).addClassName("flush").addClassName("motion-grid");
            auto &grid = card.add<DataGrid>()
                             .grow(1)
                             .addClassName("grid")
                             .columns({
                                 {"Fixture"},
                                 {.title = "Status", .width = 96, .sortable = false},
                                 {.title = "Channels", .width = 84, .align = TextAlign::End},
                             });
            for (const auto &[name, tone, status, channels]: std::vector<std::tuple<const char *, Tone, const char *, const char *>>{
                     {"Wash 1", Tone::Success, "Online", "16"},
                     {"Spot left", Tone::Warning, "Hot", "24"},
                     {"Strobe", Tone::Danger, "Offline", "4"},
                     {"Cyc 1", Tone::Success, "Online", "12"},
                     {"Haze", Tone::Success, "Online", "2"},
                     {"Blinder", Tone::Accent, "Armed", "8"},
                 }) {
                auto &row = grid.row({name, status, channels});
                row.cell(1).add(0, std::make_shared<StatusDot>(tone));
            }
            grid.select(1);
            return grid;
        }
    }

    std::vector<Scene> foundationScenes() {
        std::vector<Scene> scenes{};
        const auto         add = [&scenes](const std::vector<Scene> &more) {
            scenes.insert(scenes.end(), more.begin(), more.end());
        };

        add(inEveryLook({
            .name = "typography",
            .title = "The type scale, three levels of text, and scripts with fallback and bidi",
            .width = 680,
            .height = 460,
            .build = typography,
        }));

        add(inEveryLook({
            .name = "icons",
            .title = "Bundled icons: Pivotal's and Lucide's, at one weight",
            .width = 680,
            .height = 460,
            .build = icons,
        }));

        // Desk adapts its sizes to the input: the same panel with a pointer, then by touch
        add(inSkinSet({
            .name = "density-pointer",
            .title = "Desk's pointer density",
            .width = 520,
            .height = 520,
            .build = density,
        }, "desk"));
        add(inSkinSet({
            .name = "density-touch",
            .title = "Desk's touch density: larger controls, hit slop to 44 px",
            .width = 520,
            .height = 520,
            .look = {.input = InputMode::Touch},
            .build = density,
        }, "desk"));

        // Motion, as animated PNGs: a menu entering from its trigger, then leaving
        auto menu = std::make_shared<Menu *>(nullptr);
        add(inSkinSet({
            .name = "motion-menu",
            .title = "Motion: a menu scales and fades in from its trigger, its submenu follows, then it leaves",
            .width = 560,
            .height = 420,
            .build = [](Window &window) {
                auto &stage = backdrop(window).addClassName("top").addClassName("start"); // Room on the right for the submenu
                stage.add<Button>("Cue actions").icon("more").addClassName("trigger");
            },
            .motion = {
                .beats = {
                    {300, [menu](Stage &stage) {
                        *menu = &stage.window().openMenu({
                            Menu::item("Go", [] {}, "Space", "play"),
                            Menu::item("Rename…", [] {}, "F2", "pencil"),
                            Menu::item("Move to", {Menu::item("Act 1", [] {}, "", "folder"), Menu::item("Act 2", [] {}, "", "folder")}, "", "move"),
                            Menu::separator(),
                            Menu::check("Show fades", true),
                            Menu::item("Delete", [] {}, "Del", "trash"),
                        }, find(stage.window(), "trigger"), Side::Bottom, PopupAlign::Start);
                    }},
                    {1100, [menu](Stage &) { (*menu)->highlight(1); }},
                    {1500, [menu](Stage &) { (*menu)->highlight(2, true); }},
                    {2700, [](Stage &stage) { stage.window().closeMenu(); }},
                },
                .duration = 3400,
                .step = 20,
                .still = 2600, // The submenu open
            },
        }, "default"));

        // Rows sliding to their places as the grid sorts
        add(inSkinSet({
            .name = "motion-sort",
            .title = "Motion: data grid rows slide to their new places as it sorts",
            .width = 560,
            .height = 420,
            .build = [](Window &window) { fixtureGrid(window); },
            .motion = {
                .beats = {
                    {300, [](Stage &stage) { find<DataGrid>(stage.window(), "grid").sort(2, SortOrder::Descending); }},
                    {1500, [](Stage &stage) { find<DataGrid>(stage.window(), "grid").sort(0, SortOrder::Ascending); }},
                    {2700, [](Stage &stage) { find<DataGrid>(stage.window(), "grid").sort(0, SortOrder::None); }},
                },
                .duration = 3900,
                .step = 20,
                .still = 1400, // Sorted by channels
            },
        }, "default"));

        return scenes;
    }
}
