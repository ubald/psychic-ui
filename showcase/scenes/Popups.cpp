#include <psychic-ui/Window.hpp>
#include <psychic-ui/Overlay.hpp>
#include <psychic-ui/components/Badge.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/Select.hpp>
#include <psychic-ui/components/Spacer.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/Toast.hpp>
#include <psychic-ui/components/ToolBar.hpp>
#include <psychic-ui/Popup.hpp>
#include <psychic-ui/components/Tone.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Color.hpp>
#include <chrono>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>
#include "../Compose.hpp"
#include "../Looks.hpp"
#include "../Scene.hpp"
#include "../Stage.hpp"

namespace psychic_ui::showcase {
    using namespace compose;

    namespace {
        /**
         * What the popups float over: a cue list in a panel, its actions in its heading. Where they anchor is kept in
         * the window's tree by class name (`actions`, `list`).
         */
        Div &cueList(Window &window, const bool top = true, const bool start = false) {
            auto &stage = backdrop(window);
            if (top) {
                stage.addClassName("top");
            }
            if (start) {
                stage.addClassName("start"); // Room on the right for a menu and its submenu
            }
            auto &panel = compose::panel(stage).addClassName("flush").addClassName("cues");
            auto &head  = line(panel).addClassName("toolbar");
            auto &heading = head.add<Div>().addClassName("heading");
            heading.add<Label>("Opening night").addClassName("title");
            heading.add<Label>("Act 1 · 6 cues").addClassName("caption");
            head.add<Spacer>();
            head.add<Button>("Go").icon("play").skin("primary").addClassName("go");
            head.add<Button>().icon("more").addClassName("icon-button").addClassName("actions");

            auto &list = panel.add<List>().addClassName("list");
            list.row("House to half", "Fade 3 s").icon("play").meta("Q1");
            list.row("Preset stage", "Fade 5 s · follow on").icon("play").meta("Q2");
            list.row("Opening", "Fade 2 s").icon("play").meta("Q3").trailing().add<Badge>("Live", Tone::Success);
            list.row("Blackout", "Snap").icon("pause").meta("Q4");
            list.row("Finale", "Fade 8 s").icon("star").meta("Q5");
            list.select(2);
            return panel;
        }

        MenuItems cueMenu() {
            auto density = std::make_shared<MenuRadioGroup>("comfortable", nullptr);
            MenuItems items{
                Menu::group("Cue"),
                Menu::item("Go", [] {}, "Space", "play"),
                Menu::item("Pause", [] {}, "P", "pause"),
                Menu::item("Rename…", [] {}, "F2", "pencil"),
                Menu::item(
                    "Move to", {
                        Menu::item("Act 1", [] {}, "", "folder"),
                        Menu::item("Act 2", [] {}, "", "folder"),
                        Menu::item("Encore", [] {}, "", "folder"),
                    }, "", "move"
                ),
                Menu::separator(),
                Menu::group("View"),
                Menu::check("Show fades", true),
                Menu::check("Show timecode", false, nullptr, "T"),
                Menu::radio(density, "Comfortable", "comfortable"),
                Menu::radio(density, "Compact", "compact"),
                Menu::separator(),
                Menu::item("Delete", [] {}, "Del", "trash"),
            };
            items[2]->enabled  = false; // Pause: nothing runs
            items.back()->danger = true;
            return items;
        }
    }

    std::vector<Scene> popupScenes() {
        std::vector<Scene> scenes{};
        const auto         add = [&scenes](const std::vector<Scene> &more) {
            scenes.insert(scenes.end(), more.begin(), more.end());
        };

        add(inEveryLook({
            .name = "menu",
            .title = "A menu with groups, icons, shortcuts, checks, a radio group and an open submenu",
            .width = 680,
            .height = 460,
            .build = [](Window &window) { cueList(window, true, true); },
            .pose = [](Stage &stage) {
                Menu &menu = stage.window().openMenu(cueMenu(), find(stage.window(), "actions"), Side::Bottom, PopupAlign::End);
                stage.frame();
                menu.highlight(3, true); // Move to, its submenu open beside it
            },
        }));

        add(inEveryLook({
            .name = "context-menu",
            .title = "A context menu, opened where the list was right-clicked",
            .width = 680,
            .height = 460,
            .build = [](Window &window) { cueList(window); },
            .pose = [](Stage &stage) {
                Div &list = find(stage.window(), "list");
                const MouseEvent center = list.mouseEventAtCenter();
                Menu &menu = stage.window().openContextMenu(
                    {
                        Menu::item("Cut", [] {}, "Ctrl+X"),
                        Menu::item("Copy", [] {}, "Ctrl+C", "copy"),
                        Menu::item("Paste", [] {}, "Ctrl+V", "clipboard"),
                        Menu::separator(),
                        Menu::item("Duplicate", [] {}, "Ctrl+D"),
                        Menu::item("Select all", [] {}, "Ctrl+A"),
                    },
                    center.windowX - 120, center.windowY - 20
                );
                stage.frame();
                menu.highlight(1);
            },
        }));

        add(inEveryLook({
            .name = "select",
            .title = "A select, its options open under it",
            .width = 680,
            .height = 460,
            .build = [](Window &window) {
                auto &panel = compose::panel(backdrop(window), "Fade", "How cue 3 reaches its levels");
                panel.width(400);
                field(panel, "Curve").add<Select>(std::vector<std::string>{"Linear", "Ease in", "Ease out", "Ease in and out", "Snap"}, 3)
                                     .addClassName("curve").grow(1);
                field(panel, "Universe").add<Select>(std::vector<std::string>{"Universe 1", "Universe 2"}).placeholder("Pick a universe…").grow(1);
                field(panel, "Follow on").add<Switch>().checked(true);
            },
            .pose = [](Stage &stage) {
                find<Select>(stage.window(), "curve").open();
            },
        }));

        add(inEveryLook({
            .name = "tooltip",
            .title = "A tooltip, after the mouse rested on a tool",
            .width = 680,
            .height = 460,
            .build = [](Window &window) {
                auto &panel = compose::panel(backdrop(window)).addClassName("flush").addClassName("plot-panel");
                auto &tools = panel.add<ToolBar>();
                const std::vector<std::pair<std::string, std::string>> named{
                    {"mouse-pointer-2", "Select · V"}, {"hash", "Frame · F"}, {"lightbulb", "Fixture · L"},
                    {"type", "Text · T"}, {"pencil", "Draw · P"},
                };
                Button *first = nullptr;
                for (const auto &[icon, tip]: named) {
                    auto &tool = tools.add<Button>().icon(icon).tooltip(tip).toggle(true).autoToggle(false);
                    if (!first) {
                        first = &tool;
                    }
                }
                first->selected(true);
                tools.children()[2]->addClassName("hovered");
                tools.add<Spacer>();
                tools.add<Button>("Share").icon("link");

                // A stage plot under the tools: fixtures on a dotted canvas
                auto &plot = panel.add<Div>().addClassName("plot");
                const std::vector<std::tuple<float, float, Color>> fixtures{
                    {120, 60, rgb(0xFFC98A)}, {200, 60, rgb(0xFFC98A)}, {280, 60, rgb(0xFFC98A)}, {360, 60, rgb(0xFFC98A)},
                    {240, 130, rgb(0xF56E9B)}, {160, 190, rgb(0x8DB2FF)}, {320, 190, rgb(0x8DB2FF)},
                };
                for (const auto &[x, y, color]: fixtures) {
                    plot.add<Div>().addClassName("fixture").left(x).top(y).backgroundColor(color);
                }
            },
            .pose = [](Stage &stage) {
                stage.hover(find(stage.window(), "hovered"));
                stage.advance(std::chrono::milliseconds(400)); // The tooltip's delay
            },
        }));

        add(inEveryLook({
            .name = "dialog",
            .title = "A modal dialog over its scrim, its field focused",
            .width = 680,
            .height = 460,
            .build = [](Window &window) { cueList(window, false); },
            .pose = [](Stage &stage) {
                auto &dialog = stage.window().overlay().open<Dialog>("Rename cue");
                dialog.icon("pencil")
                      .description("Cue names show in the cue list and on the stage manager's screen.");
                dialog.body().add<TextInput>("Opening");
                dialog.action("Cancel");
                dialog.action("Rename", [] {}, "primary");
            },
        }));

        add(inEveryLook({
            .name = "toasts",
            .title = "Toasts stacked in the corner",
            .width = 680,
            .height = 460,
            .build = [](Window &window) { cueList(window); },
            .pose = [](Stage &stage) {
                const auto none = std::chrono::nanoseconds::zero(); // Kept until closed, for the capture
                stage.window().toast({.title = "Show file saved", .description = "Saved to Lights/Opening night.show", .timeout = none});
                stage.frame();
                stage.window().toast({
                    .title = "Universe 2 is not responding",
                    .description = "Its node went quiet 30 s ago.",
                    .variant = ToastVariant::Warning,
                    .timeout = none,
                    .actions = {{"Retry", nullptr, true}, {"Later", nullptr}},
                });
                stage.frame();
                stage.window().toast({.title = "Patch complete", .description = "48 fixtures on universe 1.", .variant = ToastVariant::Success, .timeout = none});
            },
        }));

        return scenes;
    }
}
