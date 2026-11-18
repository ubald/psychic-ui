#pragma once

#include <string>
#include <vector>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Avatar.hpp>
#include <psychic-ui/components/Badge.hpp>
#include <psychic-ui/components/Chip.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/EmptyState.hpp>
#include <psychic-ui/components/Inspector.hpp>
#include <psychic-ui/components/Kbd.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/components/StatusDot.hpp>
#include <psychic-ui/components/Tree.hpp>

namespace psychic_ui {

    /**
     * Lists, a data grid with an inspector, a tree, badges, chips, avatars, key caps and an empty state
     */
    class Lists : public Div {
    public:
        Lists();

    private:
        Inspector      *_inspector{nullptr};
        InspectorField *_name{nullptr};
        InspectorField *_kind{nullptr};
        InspectorField *_channels{nullptr};
        InspectorField *_universe{nullptr};
        Badge          *_status{nullptr};

        /**
         * A titled group, with a line of explanation under its title, then its content
         */
        static Div &group(Div &parent, const std::string &title, const std::string &caption);

        void inspect(const DataGridRow *row);
    };

    inline Div &Lists::group(Div &parent, const std::string &title, const std::string &caption) {
        auto &group = parent.add<VBox>().addClassName("group");
        group.add<Label>(title).addClassName("h4");
        group.add<Label>(caption).addClassName("text-tertiary");
        return group;
    }

    inline Lists::Lists() : Div() {
        tag("Lists")
            .direction(Direction::Row)
            .padding(0); // We're using a scroller so no padding

        auto &scroller = add<Div>()
            .addClassName("scroller")
            .grow(1)
            .shrink(1)
            .height(100_pct)
            .overflow(Overflow::Scroll);
        add<ScrollBar>(scroller).height(100_pct);

        // region List and tree

        auto &top = scroller.add<HBox>().addClassName("columns");

        auto &people = group(top, "List", "Up and Down move the selection; Tab enters the list once").grow(1);
        auto &list   = people.add<Div>().addClassName("card").add<List>();
        list.row("Ada Lovelace", "ada@analytical.engine").avatar("Ada Lovelace").meta("2 min");
        list.row("Grace Hopper", "Compilers, COBOL").avatar("Grace Hopper").meta("1 h")
            .trailing().add<Badge>("Admin", Tone::Accent);
        list.row("Alan Turing", "Bletchley Park").avatar("Alan Turing").meta("Yesterday")
            .trailing().add<StatusDot>(Tone::Success);
        list.row("Search people", "Find anyone in the workspace").icon("search")
            .trailing().add<Kbd>("⌘ K");
        list.row("Archive", "Moved out of the way, never deleted").icon("archive").meta("128");
        list.select(1);

        auto &files = group(top, "Tree", "Right opens a folder or goes in, Left closes it or goes up").width(300);
        auto &tree  = files.add<Div>().addClassName("card").addClassName("tree-card").add<Tree>();
        auto &show  = tree.node("Show", "folder").expanded(true);
        auto &cues  = show.node("Cues", "list-detail").trailing("42").expanded(true);
        cues.node("Opening", "play");
        cues.node("Blackout", "pause");
        cues.node("Finale", "star");
        show.node("Fixtures", "layers").trailing("128").node("Wash 1", "device");
        show.node("Palettes", "eyedropper");
        tree.node("Media", "image").node("Loop.mov", "video");
        tree.node("Settings", "settings");
        tree.select(&cues.row());

        // endregion

        // region Data grid and inspector

        auto &patch = group(scroller, "Data grid and inspector", "Click a header to sort, again to reverse, a third time to undo; the inspector follows the selection");
        auto &card  = patch.add<HBox>().addClassName("card").addClassName("grid-card");
        auto &grid  = card.add<DataGrid>()
                          .grow(1)
                          .columns({
                              {"Fixture"},
                              {.title = "Kind", .width = 120},
                              {.title = "Status", .width = 96, .sortable = false},
                              {.title = "Channels", .width = 72, .align = TextAlign::End},
                              {.title = "Universe", .width = 72, .align = TextAlign::End},
                          });
        struct Fixture {
            const char *name;
            const char *kind;
            Tone       tone;
            const char *status;
            const char *channels;
            const char *universe;
        };
        const std::vector<Fixture> fixtures{
            {"Wash 1", "Moving head", Tone::Success, "Online", "16", "1"},
            {"Wash 2", "Moving head", Tone::Success, "Online", "16", "1"},
            {"Spot left", "Profile", Tone::Warning, "Hot", "24", "2"},
            {"Spot right", "Profile", Tone::Success, "Online", "24", "2"},
            {"Strobe", "Strobe", Tone::Danger, "Offline", "4", "3"},
            {"Cyc 1", "LED batten", Tone::Success, "Online", "12", "3"},
            {"Cyc 2", "LED batten", Tone::Neutral, "Parked", "12", "3"},
            {"Haze", "Hazer", Tone::Success, "Online", "2", "4"},
            {"Follow spot", "Profile", Tone::Neutral, "Parked", "8", "4"},
            {"Blinder", "Blinder", Tone::Accent, "Armed", "4", "10"},
        };
        for (const Fixture &f: fixtures) {
            auto &row = grid.row({f.name, f.kind, f.status, f.channels, f.universe});
            // The status as a dot before its text
            row.cell(2).add(0, std::make_shared<StatusDot>(f.tone));
        }

        _inspector = &card.add<Inspector>().width(300);
        auto &general = _inspector->section("General");
        _name     = &general.field("Name");
        _kind     = &general.field("Kind");
        auto &status = general.field("Status");
        _status   = &status.value().add<Badge>();
        auto &patching = _inspector->section("Patch");
        _channels = &patching.field("Channels");
        _universe = &patching.field("Universe");
        patching.field("Locked").value().add<CheckBox>();

        grid.onSelectionChange([this](DataGridRow *row) { inspect(row); });
        grid.select(2);

        // endregion

        // region Badges, chips, avatars, keys

        auto &marks = scroller.add<HBox>().addClassName("columns");

        auto &badges = group(marks, "Badges and status dots", "Neutral, accent, success, warning, danger; 20 px, 16 px with the small class");
        auto &badgeRow = badges.add<HBox>().addClassName("row");
        const char *const toneNames[]{"Neutral", "Accent", "Success", "Warning", "Danger"};
        for (std::size_t i = 0; i < std::size(allTones); ++i) {
            badgeRow.add<Badge>(toneNames[i], allTones[i]);
        }
        auto &smallRow = badges.add<HBox>().addClassName("row");
        for (std::size_t i = 0; i < std::size(allTones); ++i) {
            smallRow.add<Badge>(toneNames[i], allTones[i]).addClassName("small");
        }
        auto &dotRow = badges.add<HBox>().addClassName("row");
        for (std::size_t i = 0; i < std::size(allTones); ++i) {
            auto &item = dotRow.add<HBox>().addClassName("dot-item");
            item.add<StatusDot>(allTones[i]);
            item.add<Label>(toneNames[i]).addClassName("text-secondary");
        }

        auto &chips = group(marks, "Chips", "Filters and values: Backspace dismisses the focused one");
        auto &chipRow = chips.add<HBox>().addClassName("row");
        chipRow.add<Chip>("All fixtures").selected(true);
        chipRow.add<Chip>("Moving heads").icon("device");
        chipRow.add<Chip>("Universe 1").removable(true);
        auto &toneRow = chips.add<HBox>().addClassName("row");
        for (std::size_t i = 1; i < std::size(allTones); ++i) {
            auto &chip = toneRow.add<Chip>(toneNames[i], allTones[i]).removable(true);
            chip.onDismiss([&toneRow, &chip]() { toneRow.remove(chip); });
        }

        auto &people2 = scroller.add<HBox>().addClassName("columns");

        auto &avatars = group(people2, "Avatars", "Initials, neutral as Pivotal's or tinted from the name; 16 to 32 px, with a status");
        auto &avatarRow = avatars.add<HBox>().addClassName("row");
        const std::vector<std::string> names{"Ada Lovelace", "Grace Hopper", "Alan Turing", "Katherine Johnson", "Linus", "margaret@apollo.nasa",
                                             "Edsger Dijkstra", "Hedy Lamarr", "Tim Berners-Lee", "Donald Knuth"};
        avatarRow.add<Avatar>("Ada Lovelace").size(AvatarSize::XSmall);
        avatarRow.add<Avatar>("Ada Lovelace").size(AvatarSize::Small);
        avatarRow.add<Avatar>("Ada Lovelace");
        avatarRow.add<Avatar>("Ada Lovelace").size(AvatarSize::Large).status(Tone::Success);
        auto &tintRow = avatars.add<HBox>().addClassName("row");
        for (const std::string &name: names) {
            tintRow.add<Avatar>(name).size(AvatarSize::Large).colorful(true);
        }
        tintRow.add<Avatar>("Grace Hopper").size(AvatarSize::Large).colorful(true).status(Tone::Warning);

        auto &keys = group(people2, "Keyboard hints", "A cap per key, in a menu row, a list row or a tool tip");
        auto &keyRow = keys.add<HBox>().addClassName("row");
        keyRow.add<Kbd>("⌘ K");
        keyRow.add<Kbd>("Ctrl Shift P");
        keyRow.add<Kbd>("⌥ ⇧ ↵");
        keyRow.add<Kbd>("Esc");
        auto &hint = keys.add<HBox>().addClassName("row");
        hint.add<Label>("Jump to anything").addClassName("text-secondary");
        hint.add<Kbd>("⌘ J");

        // endregion

        // region Empty state

        auto &nothing = group(scroller, "Empty state", "What fills a pane with nothing to show, and the way forward");
        auto &emptyCard = nothing.add<Div>().addClassName("card");
        auto &empty = emptyCard.add<EmptyState>("inbox", "No cues yet", "Cues record the stage as it is, so a show can play it back. Record one to start.");
        empty.action("Record a cue").icon("plus");

        // endregion
    }

    inline void Lists::inspect(const DataGridRow *row) {
        if (!row) {
            _inspector->title("Nothing selected").icon("");
            return;
        }
        _inspector->title(row->value(0), row->value(1)).icon("device");
        _name->text(row->value(0));
        _kind->text(row->value(1));
        _channels->text(row->value(3));
        _universe->text(row->value(4));
        const std::string &status = row->value(2);
        Tone tone = Tone::Neutral;
        if (status == "Online") {
            tone = Tone::Success;
        } else if (status == "Hot") {
            tone = Tone::Warning;
        } else if (status == "Offline") {
            tone = Tone::Danger;
        } else if (status == "Armed") {
            tone = Tone::Accent;
        }
        _status->label(status).tone(tone);
    }
}
