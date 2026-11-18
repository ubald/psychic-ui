#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Avatar.hpp>
#include <psychic-ui/components/Badge.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Chip.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/Inspector.hpp>
#include <psychic-ui/components/Kbd.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/StatusDot.hpp>
#include <psychic-ui/components/Tree.hpp>
#include "../Compose.hpp"
#include "../Looks.hpp"
#include "../Scene.hpp"
#include "../Stage.hpp"

namespace psychic_ui::showcase {
    using namespace compose;

    namespace {
        void lists(Window &window) {
            auto &stage = backdrop(window).addClassName("row");

            auto &people = panel(stage, "People").addClassName("flush").addClassName("people");
            auto &list   = people.add<List>();
            list.row("Ada Lovelace", "ada@analytical.engine").avatar("Ada Lovelace").meta("2 min");
            list.row("Grace Hopper", "Compilers, COBOL").avatar("Grace Hopper").meta("1 h")
                .trailing().add<Badge>("Admin", Tone::Accent);
            list.row("Alan Turing", "Bletchley Park").avatar("Alan Turing").meta("Yesterday")
                .trailing().add<StatusDot>(Tone::Success);
            list.row("Search people", "Anyone in the workspace").icon("search")
                .trailing().add<Kbd>("⌘ K");
            list.select(1);

            auto &files = panel(stage, "Show").addClassName("flush").addClassName("files");
            auto &tree  = files.add<Tree>();
            auto &show  = tree.node("Opening night", "folder").expanded(true);
            auto &cues  = show.node("Cues", "list-detail").trailing("42").expanded(true);
            cues.node("House to half", "play");
            auto &opening = cues.node("Opening", "play");
            cues.node("Blackout", "pause");
            show.node("Fixtures", "layers").trailing("128").node("Wash 1", "device");
            show.node("Palettes", "eyedropper");
            tree.node("Media", "image").node("Loop.mov", "video");
            tree.select(&opening.row());
        }

        void dataGrid(Window &window) {
            auto &card = panel(backdrop(window)).addClassName("flush").addClassName("grid-card");
            auto &grid = card.add<DataGrid>()
                             .grow(1)
                             .columns({
                                 {"Fixture"},
                                 {.title = "Kind", .width = 116},
                                 {.title = "Status", .width = 96, .sortable = false},
                                 {.title = "Channels", .width = 84, .align = TextAlign::End},
                                 {.title = "Universe", .width = 84, .align = TextAlign::End},
                             });
            struct Fixture {
                const char *name;
                const char *kind;
                Tone       tone;
                const char *status;
                const char *channels;
                const char *universe;
            };
            for (const Fixture &f: std::vector<Fixture>{
                     {"Wash 1", "Moving head", Tone::Success, "Online", "16", "1"},
                     {"Spot left", "Profile", Tone::Warning, "Hot", "24", "2"},
                     {"Strobe", "Strobe", Tone::Danger, "Offline", "4", "3"},
                     {"Cyc 1", "LED batten", Tone::Success, "Online", "12", "3"},
                     {"Haze", "Hazer", Tone::Success, "Online", "2", "4"},
                     {"Follow spot", "Profile", Tone::Neutral, "Parked", "8", "4"},
                     {"Blinder", "Blinder", Tone::Accent, "Armed", "4", "10"},
                     {"Wash 2", "Moving head", Tone::Success, "Online", "16", "1"},
                 }) {
                auto &row = grid.row({f.name, f.kind, f.status, f.channels, f.universe});
                row.cell(2).add(0, std::make_shared<StatusDot>(f.tone));
            }
            grid.sort(3, SortOrder::Descending);
            grid.select(0); // Spot left, first once sorted

            auto &inspector = card.add<Inspector>().addClassName("grid-inspector");
            inspector.title("Spot left", "Profile").icon("device");
            auto &general = inspector.section("General");
            general.field("Name", "Spot left");
            general.field("Kind", "Profile");
            general.field("Status").value().add<Badge>("Hot", Tone::Warning);
            auto &patch = inspector.section("Patch");
            patch.field("Channels", "24");
            patch.field("Universe", "2");
            patch.field("Locked").value().add<CheckBox>().checked(true);
        }

        void tags(Window &window) {
            auto &panel = compose::panel(backdrop(window), "Marks", "Badges, status dots, chips, avatars and key caps");
            const char *names[]{"Neutral", "Accent", "Success", "Warning", "Danger"};

            auto &badges = line(panel);
            for (std::size_t i = 0; i < std::size(allTones); ++i) {
                badges.add<Badge>(names[i], allTones[i]);
            }
            auto &dots = line(panel);
            for (std::size_t i = 0; i < std::size(allTones); ++i) {
                auto &item = line(dots);
                item.add<StatusDot>(allTones[i]);
                item.add<Label>(names[i]).addClassName("text-secondary");
            }
            auto &chips = line(panel);
            chips.add<Chip>("All fixtures").selected(true);
            chips.add<Chip>("Moving heads").icon("device");
            chips.add<Chip>("Universe 1").removable(true);
            chips.add<Chip>("Hot", Tone::Warning).removable(true);

            auto &avatars = line(panel);
            for (const char *name: {"Ada Lovelace", "Grace Hopper", "Alan Turing", "Katherine Johnson", "Hedy Lamarr",
                                    "Edsger Dijkstra", "Donald Knuth"}) {
                avatars.add<Avatar>(name).size(AvatarSize::Large).colorful(true);
            }
            avatars.add<Avatar>("Margaret Hamilton").size(AvatarSize::Large).status(Tone::Success);

            auto &keys = line(panel);
            keys.add<Label>("Jump to anything").addClassName("text-secondary");
            keys.add<Kbd>("⌘ K");
            keys.add<Kbd>("Ctrl Shift P");
            keys.add<Kbd>("Esc");
        }
    }

    std::vector<Scene> collectionScenes() {
        std::vector<Scene> scenes{};
        const auto         add = [&scenes](const std::vector<Scene> &more) {
            scenes.insert(scenes.end(), more.begin(), more.end());
        };

        add(inEveryLook({
            .name = "lists",
            .title = "A list with avatars, badges and key caps, and a tree",
            .width = 680,
            .height = 460,
            .build = lists,
        }));

        add(inEveryLook({
            .name = "data-grid",
            .title = "A data grid sorted by channels, its selection in an inspector",
            .width = 1040,
            .height = 440,
            .build = dataGrid,
        }));

        add(inEveryLook({
            .name = "marks",
            .title = "Badges, status dots, chips, avatars and key caps",
            .width = 680,
            .height = 460,
            .build = tags,
        }));

        return scenes;
    }
}
