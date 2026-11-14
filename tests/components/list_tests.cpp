#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Avatar.hpp>
#include <psychic-ui/components/Badge.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/EmptyState.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Icon.hpp>
#include <psychic-ui/components/Inspector.hpp>
#include <psychic-ui/components/Kbd.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Tree.hpp>
#include <psychic-ui/skins/default/DefaultAvatarSkin.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Fixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 1000, 700};
        const Theme                 &theme{themes::pivotalDark};

        void layout() { systemWindow.layout(); }

        void click(Div &div) {
            const auto [x, y] = center(div);
            window->handleMouseButtonChange(x, y, MouseButton::LEFT, true, Mod{});
            window->handleMouseButtonChange(x, y, MouseButton::LEFT, false, Mod{});
            layout();
        }

        void key(Key key) {
            window->handleKeyPress(key, Mod{});
            layout();
        }

        static std::pair<int, int> center(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y, div.getWidth() / 2, div.getHeight() / 2);
            return {x, y};
        }
    };

    struct ListFixture : Fixture {
        List    &list{window->appContainer().add<List>()};
        ListRow *ada{nullptr};
        ListRow *grace{nullptr};
        ListRow *alan{nullptr};

        ListFixture() {
            ada   = &list.row("Ada Lovelace", "ada@example.com").avatar("Ada Lovelace").meta("2h");
            grace = &list.row("Grace Hopper").icon("user");
            alan  = &list.row("Alan Turing", "Bletchley");
            alan->trailing().add<Badge>("New", Tone::Accent);
            layout();
        }
    };

    std::vector<std::string> values(const DataGrid &grid, std::size_t column) {
        std::vector<std::string> out{};
        for (const DataGridRow *row: grid.dataRows()) {
            out.push_back(row->value(column));
        }
        return out;
    }
}

// region List

TEST_CASE("List: rows of a title, secondary text, leading and trailing parts", "[components][list]") {
    ListFixture f;
    CHECK(f.list.rows() == std::vector<SelectableRow *>{f.ada, f.grace, f.alan});
    CHECK(f.ada->title() == "Ada Lovelace");
    CHECK(f.ada->secondary() == "ada@example.com");
    CHECK(f.grace->secondary().empty());

    // Pivotal's default rows: 48 px, between hairlines, on the page's gutter
    CHECK(f.grace->getHeight() == f.theme.metrics.pointer.rowMd + f.theme.metrics.space(2));
    CHECK(f.ada->getHeight() > f.grace->getHeight()); // Two lines of text
    CHECK(f.ada->computedStyle().borderBottom() == f.theme.metrics.border);
    CHECK(f.alan->computedStyle().borderBottom() == 0.0f); // The last one
    CHECK(f.ada->computedStyle().paddingRight() == f.theme.metrics.gutter);

    // The parts: an avatar leads Ada, a meta and a badge trail
    Div &lead = f.ada->leading();
    REQUIRE(lead.children().size() == 1);
    auto avatar = std::dynamic_pointer_cast<Avatar>(lead.children()[0]);
    REQUIRE(avatar);
    CHECK(avatar->initials() == "AL");
    CHECK(f.alan->trailing().children().size() == 1);

    // Another lead replaces it
    f.ada->icon("user");
    CHECK(lead.children().size() == 1);
    CHECK(std::dynamic_pointer_cast<Icon>(lead.children()[0]));
}

TEST_CASE("List: a click selects a row, one at a time", "[components][list]") {
    ListFixture f;
    std::vector<ListRow *> changes{};
    f.list.onSelectionChange([&](ListRow *row) { changes.push_back(row); });
    CHECK(f.list.selection() == nullptr);
    CHECK(f.list.selectedIndex() == -1);

    f.click(*f.grace);
    CHECK(f.list.selectedRow() == f.grace);
    CHECK(f.grace->selected());
    CHECK(f.grace->classNames().contains("selected"));
    CHECK(f.grace->computedStyle().backgroundColor() == f.theme.palette.raised);
    CHECK(f.grace->computedStyle().borderLeftColor() == f.theme.palette.accent);
    CHECK(f.list.selectedIndex() == 1);

    f.click(*f.alan);
    CHECK(f.alan->selected());
    CHECK_FALSE(f.grace->selected());
    CHECK_FALSE(f.grace->classNames().contains("selected"));

    f.click(*f.alan); // Already selected: no change
    f.alan->selected(false);
    CHECK(f.list.selection() == nullptr);
    f.list.select(0);
    CHECK(f.ada->selected());
    f.list.select(nullptr);
    CHECK(changes == std::vector<ListRow *>{f.grace, f.alan, nullptr, f.ada, nullptr});
}

TEST_CASE("List: Tab enters at one row, the arrows move the selection", "[components][list]") {
    ListFixture f;
    // Only the first row is in the Tab order
    CHECK(f.ada->focusEnabled());
    CHECK_FALSE(f.grace->focusEnabled());
    CHECK_FALSE(f.alan->focusEnabled());

    f.key(Key::TAB);
    CHECK(f.ada->hasFocus());
    CHECK(f.ada->computedStyle().outlineWidth() == f.theme.metrics.focusRing);
    CHECK(f.list.selection() == nullptr); // Focus alone doesn't select

    f.key(Key::DOWN); // From the focused row
    CHECK(f.list.selection() == f.grace);
    CHECK(f.grace->hasFocus());
    CHECK(f.grace->focusEnabled()); // The Tab stop follows the selection
    CHECK_FALSE(f.ada->focusEnabled());
    f.key(Key::DOWN);
    f.key(Key::DOWN); // Stops at the last
    CHECK(f.list.selection() == f.alan);
    f.key(Key::UP);
    CHECK(f.list.selection() == f.grace);
    f.key(Key::HOME);
    CHECK(f.list.selection() == f.ada);
    f.key(Key::END);
    CHECK(f.list.selection() == f.alan);

    // Disabled rows are skipped
    f.grace->enabled(false);
    f.layout();
    f.key(Key::UP);
    CHECK(f.list.selection() == f.ada);
}

TEST_CASE("List: a row going away stops being the selection", "[components][list]") {
    ListFixture f;
    ListRow *last = nullptr;
    int     calls = 0;
    f.list.onSelectionChange([&](ListRow *row) { last = row; ++calls; });
    f.list.select(f.grace);
    f.list.remove(*f.grace);
    f.layout();
    CHECK(f.list.selection() == nullptr);
    CHECK(last == nullptr);
    CHECK(calls == 2);
    // A row selected before it joined becomes the selection
    auto row = std::make_shared<ListRow>("Margaret Hamilton");
    row->selected(true);
    f.list.add(row);
    CHECK(f.list.selection() == row.get());
}

TEST_CASE("List: the keyboard scrolls the selection into view", "[components][list]") {
    Fixture f;
    auto &scroller = f.window->appContainer().add<Div>().height(100).overflow(Overflow::Scroll);
    auto &list     = scroller.add<List>();
    for (int i = 0; i < 10; ++i) {
        list.row("Row " + std::to_string(i));
    }
    f.layout();
    f.key(Key::TAB);
    f.key(Key::END);
    const int rowHeight = list.rows().back()->getHeight();
    CHECK(scroller.scrollY() == 100 - (list.getHeight()));
    CHECK(list.rows().back()->y() + scroller.scrollY() + rowHeight == 100);
    f.key(Key::HOME);
    CHECK(scroller.scrollY() == 0);
}

// endregion

// region Data grid

TEST_CASE("DataGrid: a header and rows of cells in columns", "[components][datagrid]") {
    Fixture f;
    auto &grid = f.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {.title = "Size", .width = 80, .align = TextAlign::End}, {.title = "Kind", .width = 120, .sortable = false}});
    auto &a = grid.row({"beta.txt", "12", "Text"});
    grid.row({"Alpha.png", "3", "Image"});
    grid.row({"gamma.wav", "120", "Audio"});
    f.layout();

    CHECK(grid.columns().size() == 3);
    CHECK(grid.header().children().size() == 3);
    CHECK(grid.header().getHeight() == f.theme.metrics.barToolbar);
    CHECK(a.cellCount() == 3);
    CHECK(a.cell(1).getWidth() == 80);
    CHECK(a.cell(2).getWidth() == 120);
    CHECK(a.cell(1).classNames().contains("end"));
    // The header's cells over the rows'
    int headerX = 0;
    int rowX    = 0;
    int dummy   = 0;
    grid.header().children()[1]->localToGlobal(headerX, dummy);
    a.cell(1).localToGlobal(rowX, dummy);
    CHECK(headerX == rowX);
    CHECK(grid.header().children()[1]->getWidth() == a.cell(1).getWidth());
    CHECK(grid.header().children()[0]->getWidth() == a.cell(0).getWidth());
}

TEST_CASE("DataGrid: clicking a header sorts, ascending, descending, then as added", "[components][datagrid]") {
    Fixture f;
    auto &grid = f.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {.title = "Size", .width = 80, .align = TextAlign::End}, {.title = "Kind", .width = 120, .sortable = false}});
    grid.row({"beta.txt", "12", "Text"});
    grid.row({"Alpha.png", "3", "Image"});
    grid.row({"gamma.wav", "120", "Audio"});
    f.layout();
    std::vector<std::pair<int, SortOrder>> sorts{};
    grid.onSortChange([&](int column, SortOrder order) { sorts.emplace_back(column, order); });

    Div &name = *grid.header().children()[0];
    Div &size = *grid.header().children()[1];
    Div &kind = *grid.header().children()[2];

    f.click(name);
    CHECK(values(grid, 0) == std::vector<std::string>{"Alpha.png", "beta.txt", "gamma.wav"}); // Without case
    CHECK(name.classNames().contains("sorted"));
    f.click(name);
    CHECK(values(grid, 0) == std::vector<std::string>{"gamma.wav", "beta.txt", "Alpha.png"});
    f.click(name);
    CHECK(values(grid, 0) == std::vector<std::string>{"beta.txt", "Alpha.png", "gamma.wav"});
    CHECK_FALSE(name.classNames().contains("sorted"));

    // Figures as numbers
    f.click(size);
    CHECK(values(grid, 1) == std::vector<std::string>{"3", "12", "120"});
    CHECK(grid.sortColumn() == 1);
    CHECK(grid.sortOrder() == SortOrder::Ascending);

    // Not sortable
    f.click(kind);
    CHECK(grid.sortColumn() == 1);

    // A row added while sorted goes in its place
    grid.row({"delta", "7", "Other"});
    CHECK(values(grid, 1) == std::vector<std::string>{"3", "7", "12", "120"});

    CHECK(sorts == std::vector<std::pair<int, SortOrder>>{
        {0, SortOrder::Ascending}, {0, SortOrder::Descending}, {-1, SortOrder::None}, {1, SortOrder::Ascending}
    });

    // The rows the keyboard walks follow the sort
    grid.sort(1, SortOrder::Descending);
    CHECK(values(grid, 1) == std::vector<std::string>{"120", "12", "7", "3"});
}

TEST_CASE("DataGrid: the selection and the focus survive a sort", "[components][datagrid]") {
    Fixture f;
    auto &grid = f.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {"Size", 80}});
    auto &b = grid.row({"b", "2"});
    auto &a = grid.row({"a", "1"});
    f.layout();
    std::vector<DataGridRow *> changes{};
    grid.onSelectionChange([&](DataGridRow *row) { changes.push_back(row); });

    f.key(Key::TAB); // The first row: the grid's Tab stop comes after its sortable headers
    while (!b.hasFocus() && !a.hasFocus()) {
        f.key(Key::TAB);
    }
    CHECK(b.hasFocus());
    f.key(Key::DOWN);
    CHECK(grid.selectedRow() == &a);
    grid.sort(0, SortOrder::Ascending);
    f.layout();
    CHECK(grid.selectedRow() == &a);
    CHECK(a.selected());
    CHECK(a.hasFocus());
    CHECK(grid.selectedIndex() == 0);
    f.key(Key::DOWN);
    CHECK(grid.selectedRow() == &b);
    CHECK(changes == std::vector<DataGridRow *>{&a, &b});
}

TEST_CASE("DataGrid: the rows scroll under the header", "[components][datagrid]") {
    Fixture f;
    auto &grid = f.window->appContainer().add<DataGrid>()
                  .height(200)
                  .columns({{"Name"}});
    for (int i = 0; i < 20; ++i) {
        grid.row({"Row " + std::to_string(i)});
    }
    f.layout();
    CHECK(grid.body().getHeight() == 200 - grid.header().getHeight());
    CHECK(grid.body().contentHeight() > grid.body().getHeight());
    grid.select(19);
    f.key(Key::TAB);
    while (!grid.dataRows().back()->hasFocus()) {
        f.key(Key::TAB);
    }
    f.key(Key::HOME);
    CHECK(grid.body().scrollY() == 0);
    f.key(Key::END);
    CHECK(grid.body().scrollY() < 0);
    CHECK(grid.header().y() == 0); // The header stays
}

// endregion

// region Tree

namespace {
    struct TreeFixture : Fixture {
        Tree     &tree{window->appContainer().add<Tree>()};
        TreeNode *src{nullptr};
        TreeNode *ui{nullptr};
        TreeNode *div{nullptr};
        TreeNode *main{nullptr};
        TreeNode *readme{nullptr};

        TreeFixture() {
            src    = &tree.node("src", "folder");
            ui     = &src->node("ui", "folder");
            div    = &ui->node("Div.cpp", "file");
            main   = &src->node("main.cpp", "file");
            readme = &tree.node("README.md", "file");
            layout();
        }
    };
}

TEST_CASE("Tree: nested nodes, indented, collapsed at first", "[components][tree]") {
    TreeFixture f;
    CHECK(f.src->level() == 0);
    CHECK(f.ui->level() == 1);
    CHECK(f.div->level() == 2);
    CHECK(f.div->parentNode() == f.ui);
    CHECK(f.src->parentNode() == nullptr);
    CHECK(f.src->nodes() == std::vector<TreeNode *>{f.ui, f.main});
    CHECK(f.tree.nodes() == std::vector<TreeNode *>{f.src, f.readme});
    CHECK(f.src->hasChildren());
    CHECK_FALSE(f.main->hasChildren());

    // Only the top rows show
    CHECK(f.tree.rows() == std::vector<SelectableRow *>{&f.src->row(), &f.readme->row()});
    CHECK(f.src->row().getHeight() == f.theme.metrics.pointer.rowSm);

    f.src->expanded(true);
    f.ui->expanded(true);
    f.layout();
    CHECK(f.tree.rows() == std::vector<SelectableRow *>{&f.src->row(), &f.ui->row(), &f.div->row(), &f.main->row(), &f.readme->row()});
    // A step per level, the icons lined up across levels (leaves keep the chevron's column)
    int srcIcon = 0;
    int mainIcon = 0;
    int divIcon = 0;
    int dummy = 0;
    const auto iconX = [&](TreeNode *node, int &x) {
        for (const auto &child: node->row().children()) {
            if (child->classNames().contains("icon")) {
                child->localToGlobal(x, dummy);
            }
        }
    };
    iconX(f.src, srcIcon);
    iconX(f.main, mainIcon);
    iconX(f.div, divIcon);
    CHECK(mainIcon - srcIcon == static_cast<int>(f.tree.indent()));
    CHECK(divIcon - srcIcon == 2 * static_cast<int>(f.tree.indent()));
}

TEST_CASE("Tree: the chevron expands and collapses", "[components][tree]") {
    TreeFixture f;
    std::vector<std::pair<TreeNode *, bool>> changes{};
    f.tree.onExpansionChange([&](TreeNode *node, bool expanded) { changes.emplace_back(node, expanded); });
    Div *chevron = nullptr;
    for (const auto &child: f.src->row().children()) {
        if (child->classNames().contains("disclosure")) {
            chevron = child.get();
        }
    }
    REQUIRE(chevron);
    f.click(*chevron);
    CHECK(f.src->expanded());
    CHECK(f.src->classNames().contains("expanded"));
    CHECK(f.tree.selection() == nullptr); // The chevron doesn't select
    f.click(*chevron);
    CHECK_FALSE(f.src->expanded());
    f.src->toggle();
    CHECK(changes == std::vector<std::pair<TreeNode *, bool>>{{f.src, true}, {f.src, false}, {f.src, true}});

    // Collapsing a node that holds the selection selects it
    f.ui->expanded(true);
    f.layout();
    f.click(f.div->row());
    CHECK(f.tree.selectedNode() == f.div);
    f.src->expanded(false);
    CHECK(f.tree.selectedNode() == f.src);
}

TEST_CASE("Tree: the keyboard walks and opens the tree", "[components][tree]") {
    TreeFixture f;
    f.key(Key::TAB);
    CHECK(f.src->row().hasFocus());
    CHECK(f.tree.selectedNode() == nullptr);
    f.key(Key::SPACE); // Selects the focused row
    CHECK(f.tree.selectedNode() == f.src);
    f.key(Key::RIGHT); // Expands
    CHECK(f.src->expanded());
    CHECK(f.tree.selectedNode() == f.src);
    f.key(Key::RIGHT); // To the first child
    CHECK(f.tree.selectedNode() == f.ui);
    f.key(Key::RIGHT);
    f.key(Key::RIGHT);
    CHECK(f.tree.selectedNode() == f.div);
    f.key(Key::RIGHT); // A leaf: nothing
    CHECK(f.tree.selectedNode() == f.div);
    f.key(Key::DOWN);
    CHECK(f.tree.selectedNode() == f.main);
    f.key(Key::UP);
    f.key(Key::LEFT); // A leaf: to its parent
    CHECK(f.tree.selectedNode() == f.ui);
    f.key(Key::LEFT); // Collapses
    CHECK_FALSE(f.ui->expanded());
    CHECK(f.tree.selectedNode() == f.ui);
    f.key(Key::DOWN);
    CHECK(f.tree.selectedNode() == f.main); // Past the collapsed children
    f.key(Key::END);
    CHECK(f.tree.selectedNode() == f.readme);
    f.key(Key::HOME);
    f.key(Key::ENTER); // Toggles
    CHECK_FALSE(f.src->expanded());
    CHECK(f.src->row().hasFocus());
}

TEST_CASE("Tree: a subtree built apart joins the tree", "[components][tree]") {
    TreeFixture f;
    auto lib = std::make_shared<TreeNode>("lib", "folder");
    auto &color = lib->node("color", "folder");
    color.node("Color.hpp", "file");
    f.tree.add(lib);
    lib->expanded(true);
    f.layout();
    CHECK(color.level() == 1);
    CHECK(color.nodes().front()->level() == 2);
    f.tree.select(&color.row());
    CHECK(f.tree.selectedNode() == &color);
}

// endregion

// region Inspector, badges, chips, avatars, keys, empty states

TEST_CASE("Inspector: sections of label and value fields", "[components][inspector]") {
    Fixture f;
    auto &inspector = f.window->appContainer().add<Inspector>().width(320).title("Cue 12", "Fade").icon("play");
    auto &timing    = inspector.section("Timing");
    auto &duration  = timing.field("Duration", "3.0 s");
    auto &loop      = timing.field("Loop");
    loop.value().add<Badge>("On", Tone::Success);
    f.layout();

    CHECK(timing.title() == "Timing");
    CHECK(duration.label() == "Duration");
    CHECK(duration.text() == "3.0 s");
    CHECK(inspector.computedStyle().backgroundColor() == f.theme.palette.surface);
    // Two columns: the labels a fixed width, the values after them, lined up
    int durationX = 0;
    int loopX     = 0;
    int dummy     = 0;
    duration.value().localToGlobal(durationX, dummy);
    loop.value().localToGlobal(loopX, dummy);
    CHECK(durationX == loopX);
    CHECK(duration.children()[0]->getWidth() == 104);
}

TEST_CASE("Badges, status dots and chips take their tone's class", "[components][badge]") {
    Fixture f;
    auto &row   = f.window->appContainer().add<Div>().row();
    auto &badge = row.add<Badge>("Live", Tone::Success);
    auto &dot   = row.add<StatusDot>(Tone::Danger);
    auto &chip  = row.add<Chip>("Open", Tone::Accent);
    f.layout();
    const Palette &p = f.theme.palette;

    CHECK(badge.classNames().contains("success"));
    CHECK(badge.computedStyle().color() == p.success);
    CHECK(badge.getHeight() == f.theme.metrics.pointer.controlXs);
    badge.tone(Tone::Warning);
    f.layout();
    CHECK_FALSE(badge.classNames().contains("success"));
    CHECK(badge.computedStyle().color() == p.warningText);

    CHECK(dot.getWidth() == 6);
    CHECK(dot.computedStyle().backgroundColor() == p.danger);
    CHECK(chip.computedStyle().color() == p.accentText);
    CHECK(chip.getHeight() == f.theme.metrics.pointer.controlMd);
}

TEST_CASE("Chip: removable, dismissed by its x or the keyboard", "[components][badge]") {
    Fixture f;
    auto &chip = f.window->appContainer().add<Chip>("Status: Open").alignSelf(Align::Start);
    int  dismissals = 0;
    int  clicks     = 0;
    chip.onDismiss([&]() { ++dismissals; });
    chip.onClick([&]() { ++clicks; });
    f.layout();

    Div *x = chip.children().back().get();
    CHECK(x->computedStyle().display() == Display::None);
    chip.dismiss(); // Not removable: nothing
    CHECK(dismissals == 0);

    chip.removable(true);
    f.layout();
    CHECK(x->computedStyle().display() == Display::Flex);
    f.click(*x);
    CHECK(dismissals == 1);
    CHECK(clicks == 0); // The x isn't the chip's click
    f.click(chip.children()[1].operator*()); // The label
    CHECK(clicks == 1);

    f.key(Key::TAB);
    CHECK(chip.hasFocus());
    f.key(Key::BACKSPACE);
    f.key(Key::DELETE);
    CHECK(dismissals == 3);
    f.key(Key::ENTER);
    CHECK(clicks == 2);

    chip.selected(true);
    f.layout();
    CHECK(chip.computedStyle().backgroundColor() == f.theme.palette.raised);
}

TEST_CASE("Avatar: initials, sizes, a status and a tint", "[components][avatar]") {
    CHECK(Avatar::initialsOf("Ada Lovelace") == "AL");
    CHECK(Avatar::initialsOf("Grace Brewster Murray Hopper") == "GH");
    CHECK(Avatar::initialsOf("plato") == "P");
    CHECK(Avatar::initialsOf("ada.lovelace@example.com") == "A");
    CHECK(Avatar::initialsOf("  ") == "");
    CHECK(Avatar::initialsOf("élodie roy") == "éR");

    Fixture f;
    auto &avatar = f.window->appContainer().add<Avatar>("Ada Lovelace").alignSelf(Align::Start);
    f.layout();
    CHECK(avatar.getWidth() == f.theme.metrics.pointer.controlSm);
    avatar.size(AvatarSize::Large).status(Tone::Success);
    f.layout();
    CHECK(avatar.getWidth() == 32);
    CHECK(avatar.getHeight() == 32);
    REQUIRE(avatar.skin());
    auto skin = std::dynamic_pointer_cast<DefaultAvatarSkin>(avatar.skin());
    REQUIRE(skin);

    // The tint: the same name gets the same, from the theme
    const Color tint = DefaultAvatarSkin::tint("Ada Lovelace", f.theme.palette);
    CHECK(tint == DefaultAvatarSkin::tint("Ada Lovelace", f.theme.palette));
    CHECK(tint != Color{});
    avatar.colorful(true);
    f.layout();
    Div *initials = skin->children()[0].get();
    CHECK(initials->computedStyle().color() == tint);
    avatar.colorful(false);
    f.layout();
    CHECK(initials->computedStyle().color() == f.theme.palette.textSecondary);
}

TEST_CASE("Kbd: a cap per key", "[components][kbd]") {
    Fixture f;
    auto &kbd = f.window->appContainer().add<Kbd>("⌘ K").alignSelf(Align::Start);
    f.layout();
    CHECK(kbd.caps() == std::vector<std::string>{"⌘", "K"});
    CHECK(kbd.children()[0]->getHeight() == 18);
    CHECK(kbd.children()[1]->getWidth() >= 18);
    kbd.keys("Ctrl  Shift P");
    CHECK(kbd.caps() == std::vector<std::string>{"Ctrl", "Shift", "P"});
}

TEST_CASE("EmptyState: an icon, a title, text and an action", "[components][empty]") {
    Fixture f;
    auto &empty = f.window->appContainer().add<EmptyState>("inbox", "No messages", "New messages show up here.");
    int  clicks = 0;
    empty.action("Compose").onClick([&]() { ++clicks; });
    f.layout();
    CHECK(empty.title() == "No messages");
    CHECK(empty.description() == "New messages show up here.");
    REQUIRE(empty.actionButton());
    CHECK(&empty.action("Write") == empty.actionButton()); // The same button, relabeled
    CHECK(empty.actionButton()->label() == "Write");
    f.click(*empty.actionButton());
    CHECK(clicks == 1);
}

// endregion
