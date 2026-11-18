#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Avatar.hpp>
#include <psychic-ui/components/Badge.hpp>
#include <psychic-ui/components/Chip.hpp>
#include <psychic-ui/components/StatusDot.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/EmptyState.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Icon.hpp>
#include <psychic-ui/components/Inspector.hpp>
#include <psychic-ui/components/Kbd.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Tree.hpp>
#include <psychic-ui/skins/pivotal/PivotalAvatarSkin.hpp>
#include <psychic-ui/themes/palette.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Rows.hpp>
#include <psychic-ui/components/Tone.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Color.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
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
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true});
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false});
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
    ListFixture fixture;
    CHECK(fixture.list.rows() == std::vector<SelectableRow *>{fixture.ada, fixture.grace, fixture.alan});
    CHECK(fixture.ada->title() == "Ada Lovelace");
    CHECK(fixture.ada->secondary() == "ada@example.com");
    CHECK(fixture.grace->secondary().empty());

    // Pivotal's default rows: 48 px, between hairlines, on the page's gutter
    CHECK(fixture.grace->getHeight() == fixture.theme.metrics.pointer.rowMd + fixture.theme.metrics.space(2));
    CHECK(fixture.ada->getHeight() > fixture.grace->getHeight()); // Two lines of text
    CHECK(fixture.ada->computedStyle().borderBottom() == fixture.theme.metrics.border);
    CHECK(fixture.alan->computedStyle().borderBottom() == 0.0f); // The last one
    CHECK(fixture.ada->computedStyle().paddingRight() == fixture.theme.metrics.gutter);

    // The parts: an avatar leads Ada, a meta and a badge trail
    const Div &lead = fixture.ada->leading();
    REQUIRE(lead.children().size() == 1);
    auto avatar = std::dynamic_pointer_cast<Avatar>(lead.children()[0]);
    REQUIRE(avatar);
    CHECK(avatar->initials() == "AL");
    CHECK(fixture.alan->trailing().children().size() == 1);

    // Another lead replaces it
    fixture.ada->icon("user");
    CHECK(lead.children().size() == 1);
    CHECK(std::dynamic_pointer_cast<Icon>(lead.children()[0]));
}

TEST_CASE("List: a click selects a row, one at a time", "[components][list]") {
    ListFixture fixture;
    std::vector<ListRow *> changes{};
    fixture.list.onSelectionChange([&](ListRow *row) { changes.push_back(row); });
    CHECK(fixture.list.selection() == nullptr);
    CHECK(fixture.list.selectedIndex() == -1);

    fixture.click(*fixture.grace);
    CHECK(fixture.list.selectedRow() == fixture.grace);
    CHECK(fixture.grace->selected());
    CHECK(fixture.grace->classNames().contains("selected"));
    CHECK(fixture.grace->computedStyle().backgroundColor() == fixture.theme.palette.raised);
    CHECK(fixture.grace->computedStyle().borderLeftColor() == fixture.theme.palette.accent);
    CHECK(fixture.list.selectedIndex() == 1);

    fixture.click(*fixture.alan);
    CHECK(fixture.alan->selected());
    CHECK_FALSE(fixture.grace->selected());
    CHECK_FALSE(fixture.grace->classNames().contains("selected"));

    fixture.click(*fixture.alan); // Already selected: no change
    fixture.alan->selected(false);
    CHECK(fixture.list.selection() == nullptr);
    fixture.list.select(0);
    CHECK(fixture.ada->selected());
    fixture.list.select(nullptr);
    CHECK(changes == std::vector<ListRow *>{fixture.grace, fixture.alan, nullptr, fixture.ada, nullptr});
}

TEST_CASE("List: Tab enters at one row, the arrows move the selection", "[components][list]") {
    ListFixture fixture;
    // Only the first row is in the Tab order
    CHECK(fixture.ada->focusEnabled());
    CHECK_FALSE(fixture.grace->focusEnabled());
    CHECK_FALSE(fixture.alan->focusEnabled());

    fixture.key(Key::TAB);
    CHECK(fixture.ada->hasFocus());
    CHECK(fixture.ada->computedStyle().outlineWidth() == fixture.theme.metrics.focusRing);
    CHECK(fixture.list.selection() == nullptr); // Focus alone doesn't select

    fixture.key(Key::DOWN); // From the focused row
    CHECK(fixture.list.selection() == fixture.grace);
    CHECK(fixture.grace->hasFocus());
    CHECK(fixture.grace->focusEnabled()); // The Tab stop follows the selection
    CHECK_FALSE(fixture.ada->focusEnabled());
    fixture.key(Key::DOWN);
    fixture.key(Key::DOWN); // Stops at the last
    CHECK(fixture.list.selection() == fixture.alan);
    fixture.key(Key::UP);
    CHECK(fixture.list.selection() == fixture.grace);
    fixture.key(Key::HOME);
    CHECK(fixture.list.selection() == fixture.ada);
    fixture.key(Key::END);
    CHECK(fixture.list.selection() == fixture.alan);

    // Disabled rows are skipped
    fixture.grace->enabled(false);
    fixture.layout();
    fixture.key(Key::UP);
    CHECK(fixture.list.selection() == fixture.ada);
}

TEST_CASE("List: a row going away stops being the selection", "[components][list]") {
    ListFixture fixture;
    ListRow *last = nullptr;
    int     calls = 0;
    fixture.list.onSelectionChange([&](ListRow *row) { last = row; ++calls; });
    fixture.list.select(fixture.grace);
    fixture.list.remove(*fixture.grace);
    fixture.layout();
    CHECK(fixture.list.selection() == nullptr);
    CHECK(last == nullptr);
    CHECK(calls == 2);
    // A row selected before it joined becomes the selection
    auto row = std::make_shared<ListRow>("Margaret Hamilton");
    row->selected(true);
    fixture.list.add(row);
    CHECK(fixture.list.selection() == row.get());
}

TEST_CASE("List: the keyboard scrolls the selection into view", "[components][list]") {
    Fixture fixture;
    auto &scroller = fixture.window->appContainer().add<Div>().height(100).overflow(Overflow::Scroll);
    auto &list     = scroller.add<List>();
    for (int i = 0; i < 10; ++i) {
        list.row("Row " + std::to_string(i));
    }
    fixture.layout();
    fixture.key(Key::TAB);
    fixture.key(Key::END);
    const int rowHeight = list.rows().back()->getHeight();
    CHECK(scroller.scrollY() == 100 - (list.getHeight()));
    CHECK(list.rows().back()->y() + scroller.scrollY() + rowHeight == 100);
    fixture.key(Key::HOME);
    CHECK(scroller.scrollY() == 0);
}

// endregion

// region Data grid

TEST_CASE("DataGrid: a header and rows of cells in columns", "[components][datagrid]") {
    Fixture fixture;
    auto &grid = fixture.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {.title = "Size", .width = 80, .align = TextAlign::End}, {.title = "Kind", .width = 120, .sortable = false}});
    auto &a = grid.row({"beta.txt", "12", "Text"});
    grid.row({"Alpha.png", "3", "Image"});
    grid.row({"gamma.wav", "120", "Audio"});
    fixture.layout();

    CHECK(grid.columns().size() == 3);
    CHECK(grid.header().children().size() == 3);
    CHECK(grid.header().getHeight() == fixture.theme.metrics.barToolbar);
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
    Fixture fixture;
    auto &grid = fixture.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {.title = "Size", .width = 80, .align = TextAlign::End}, {.title = "Kind", .width = 120, .sortable = false}});
    grid.row({"beta.txt", "12", "Text"});
    grid.row({"Alpha.png", "3", "Image"});
    grid.row({"gamma.wav", "120", "Audio"});
    fixture.layout();
    std::vector<std::pair<int, SortOrder>> sorts{};
    grid.onSortChange([&](int column, SortOrder order) { sorts.emplace_back(column, order); });

    // The sort buttons in the header's cells
    Div &name = *grid.header().children()[0]->children()[0];
    Div &size = *grid.header().children()[1]->children()[0];
    Div &kind = *grid.header().children()[2]; // Not sortable: its title only

    fixture.click(name);
    CHECK(values(grid, 0) == std::vector<std::string>{"Alpha.png", "beta.txt", "gamma.wav"}); // Without case
    CHECK(name.classNames().contains("sorted"));
    fixture.click(name);
    CHECK(values(grid, 0) == std::vector<std::string>{"gamma.wav", "beta.txt", "Alpha.png"});
    fixture.click(name);
    CHECK(values(grid, 0) == std::vector<std::string>{"beta.txt", "Alpha.png", "gamma.wav"});
    CHECK_FALSE(name.classNames().contains("sorted"));

    // Figures as numbers
    fixture.click(size);
    CHECK(values(grid, 1) == std::vector<std::string>{"3", "12", "120"});
    CHECK(grid.sortColumn() == 1);
    CHECK(grid.sortOrder() == SortOrder::Ascending);

    // Not sortable
    fixture.click(kind);
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
    Fixture fixture;
    auto &grid = fixture.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {"Size", 80}});
    auto &b = grid.row({"b", "2"});
    auto &a = grid.row({"a", "1"});
    fixture.layout();
    std::vector<DataGridRow *> changes{};
    grid.onSelectionChange([&](DataGridRow *row) { changes.push_back(row); });

    fixture.key(Key::TAB); // The first row: the grid's Tab stop comes after its sortable headers
    while (!b.hasFocus() && !a.hasFocus()) {
        fixture.key(Key::TAB);
    }
    CHECK(b.hasFocus());
    fixture.key(Key::DOWN);
    CHECK(grid.selectedRow() == &a);
    grid.sort(0, SortOrder::Ascending);
    fixture.layout();
    CHECK(grid.selectedRow() == &a);
    CHECK(a.selected());
    CHECK(a.hasFocus());
    CHECK(grid.selectedIndex() == 0);
    fixture.key(Key::DOWN);
    CHECK(grid.selectedRow() == &b);
    CHECK(changes == std::vector<DataGridRow *>{&a, &b});
}

TEST_CASE("DataGrid: the rows scroll under the header", "[components][datagrid]") {
    Fixture fixture;
    auto &grid = fixture.window->appContainer().add<DataGrid>()
                  .height(200)
                  .columns({{"Name"}});
    for (int i = 0; i < 20; ++i) {
        grid.row({"Row " + std::to_string(i)});
    }
    fixture.layout();
    CHECK(grid.body().getHeight() == 200 - grid.header().getHeight());
    CHECK(grid.body().contentHeight() > grid.body().getHeight());
    grid.select(19);
    fixture.key(Key::TAB);
    while (!grid.dataRows().back()->hasFocus()) {
        fixture.key(Key::TAB);
    }
    fixture.key(Key::HOME);
    CHECK(grid.body().scrollY() == 0);
    fixture.key(Key::END);
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
    TreeFixture fixture;
    CHECK(fixture.src->level() == 0);
    CHECK(fixture.ui->level() == 1);
    CHECK(fixture.div->level() == 2);
    CHECK(fixture.div->parentNode() == fixture.ui);
    CHECK(fixture.src->parentNode() == nullptr);
    CHECK(fixture.src->nodes() == std::vector<TreeNode *>{fixture.ui, fixture.main});
    CHECK(fixture.tree.nodes() == std::vector<TreeNode *>{fixture.src, fixture.readme});
    CHECK(fixture.src->hasChildren());
    CHECK_FALSE(fixture.main->hasChildren());

    // Only the top rows show
    CHECK(fixture.tree.rows() == std::vector<SelectableRow *>{&fixture.src->row(), &fixture.readme->row()});
    CHECK(fixture.src->row().getHeight() == fixture.theme.metrics.pointer.rowSm);

    fixture.src->expanded(true);
    fixture.ui->expanded(true);
    fixture.layout();
    CHECK(fixture.tree.rows() == std::vector<SelectableRow *>{&fixture.src->row(), &fixture.ui->row(), &fixture.div->row(), &fixture.main->row(), &fixture.readme->row()});
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
    iconX(fixture.src, srcIcon);
    iconX(fixture.main, mainIcon);
    iconX(fixture.div, divIcon);
    CHECK(mainIcon - srcIcon == static_cast<int>(fixture.tree.indent()));
    CHECK(divIcon - srcIcon == 2 * static_cast<int>(fixture.tree.indent()));
}

TEST_CASE("Tree: the chevron expands and collapses", "[components][tree]") {
    TreeFixture fixture;
    std::vector<std::pair<TreeNode *, bool>> changes{};
    fixture.tree.onExpansionChange([&](TreeNode *node, bool expanded) { changes.emplace_back(node, expanded); });
    Div *chevron = nullptr;
    for (const auto &child: fixture.src->row().children()) {
        if (child->classNames().contains("disclosure")) {
            chevron = child.get();
        }
    }
    REQUIRE(chevron);
    fixture.click(*chevron);
    CHECK(fixture.src->expanded());
    CHECK(fixture.src->classNames().contains("expanded"));
    CHECK(fixture.tree.selection() == nullptr); // The chevron doesn't select
    fixture.click(*chevron);
    CHECK_FALSE(fixture.src->expanded());
    fixture.src->toggle();
    CHECK(changes == std::vector<std::pair<TreeNode *, bool>>{{fixture.src, true}, {fixture.src, false}, {fixture.src, true}});

    // Collapsing a node that holds the selection selects it
    fixture.ui->expanded(true);
    fixture.layout();
    fixture.click(fixture.div->row());
    CHECK(fixture.tree.selectedNode() == fixture.div);
    fixture.src->expanded(false);
    CHECK(fixture.tree.selectedNode() == fixture.src);
}

TEST_CASE("Tree: the keyboard walks and opens the tree", "[components][tree]") {
    TreeFixture fixture;
    fixture.key(Key::TAB);
    CHECK(fixture.src->row().hasFocus());
    CHECK(fixture.tree.selectedNode() == nullptr);
    fixture.key(Key::SPACE); // Selects the focused row
    CHECK(fixture.tree.selectedNode() == fixture.src);
    fixture.key(Key::RIGHT); // Expands
    CHECK(fixture.src->expanded());
    CHECK(fixture.tree.selectedNode() == fixture.src);
    fixture.key(Key::RIGHT); // To the first child
    CHECK(fixture.tree.selectedNode() == fixture.ui);
    fixture.key(Key::RIGHT);
    fixture.key(Key::RIGHT);
    CHECK(fixture.tree.selectedNode() == fixture.div);
    fixture.key(Key::RIGHT); // A leaf: nothing
    CHECK(fixture.tree.selectedNode() == fixture.div);
    fixture.key(Key::DOWN);
    CHECK(fixture.tree.selectedNode() == fixture.main);
    fixture.key(Key::UP);
    fixture.key(Key::LEFT); // A leaf: to its parent
    CHECK(fixture.tree.selectedNode() == fixture.ui);
    fixture.key(Key::LEFT); // Collapses
    CHECK_FALSE(fixture.ui->expanded());
    CHECK(fixture.tree.selectedNode() == fixture.ui);
    fixture.key(Key::DOWN);
    CHECK(fixture.tree.selectedNode() == fixture.main); // Past the collapsed children
    fixture.key(Key::END);
    CHECK(fixture.tree.selectedNode() == fixture.readme);
    fixture.key(Key::HOME);
    fixture.key(Key::ENTER); // Toggles
    CHECK_FALSE(fixture.src->expanded());
    CHECK(fixture.src->row().hasFocus());
}

TEST_CASE("Tree: a subtree built apart joins the tree", "[components][tree]") {
    TreeFixture fixture;
    auto lib = std::make_shared<TreeNode>("lib", "folder");
    auto &color = lib->node("color", "folder");
    color.node("Color.hpp", "file");
    fixture.tree.add(lib);
    lib->expanded(true);
    fixture.layout();
    CHECK(color.level() == 1);
    CHECK(color.nodes().front()->level() == 2);
    fixture.tree.select(&color.row());
    CHECK(fixture.tree.selectedNode() == &color);
}

// endregion

// region Inspector, badges, chips, avatars, keys, empty states

TEST_CASE("Inspector: sections of label and value fields", "[components][inspector]") {
    Fixture fixture;
    auto &inspector = fixture.window->appContainer().add<Inspector>().width(320).title("Cue 12", "Fade").icon("play");
    auto &timing    = inspector.section("Timing");
    auto &duration  = timing.field("Duration", "3.0 s");
    auto &loop      = timing.field("Loop");
    loop.value().add<Badge>("On", Tone::Success);
    fixture.layout();

    CHECK(timing.title() == "Timing");
    CHECK(duration.label() == "Duration");
    CHECK(duration.text() == "3.0 s");
    CHECK(inspector.computedStyle().backgroundColor() == fixture.theme.palette.surface);
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
    Fixture fixture;
    auto &row   = fixture.window->appContainer().add<Div>().row();
    auto &badge = row.add<Badge>("Live", Tone::Success);
    auto &dot   = row.add<StatusDot>(Tone::Danger);
    auto &chip  = row.add<Chip>("Open", Tone::Accent);
    fixture.layout();
    const Palette &palette = fixture.theme.palette;

    CHECK(badge.classNames().contains("success"));
    CHECK(badge.computedStyle().color() == palette.success);
    CHECK(badge.getHeight() == fixture.theme.metrics.pointer.controlXs);
    badge.tone(Tone::Warning);
    fixture.layout();
    CHECK_FALSE(badge.classNames().contains("success"));
    CHECK(badge.computedStyle().color() == palette.warningText);

    CHECK(dot.getWidth() == 6);
    CHECK(dot.computedStyle().backgroundColor() == palette.danger);
    CHECK(chip.computedStyle().color() == palette.accentText);
    CHECK(chip.getHeight() == fixture.theme.metrics.pointer.controlMd);
}

TEST_CASE("Chip: removable, dismissed by its x or the keyboard", "[components][badge]") {
    Fixture fixture;
    auto &chip = fixture.window->appContainer().add<Chip>("Status: Open").alignSelf(Align::Start);
    int  dismissals = 0;
    int  clicks     = 0;
    chip.onDismiss([&]() { ++dismissals; });
    chip.onClick([&]() { ++clicks; });
    fixture.layout();

    Div *x = chip.children().back().get();
    CHECK(x->computedStyle().display() == Display::None);
    chip.dismiss(); // Not removable: nothing
    CHECK(dismissals == 0);

    chip.removable(true);
    fixture.layout();
    CHECK(x->computedStyle().display() == Display::Flex);
    fixture.click(*x);
    CHECK(dismissals == 1);
    CHECK(clicks == 0); // The x isn't the chip's click
    fixture.click(chip.children()[1].operator*()); // The label
    CHECK(clicks == 1);

    fixture.key(Key::TAB);
    CHECK(chip.hasFocus());
    fixture.key(Key::BACKSPACE);
    fixture.key(Key::DELETE);
    CHECK(dismissals == 3);
    fixture.key(Key::ENTER);
    CHECK(clicks == 2);

    chip.selected(true);
    fixture.layout();
    CHECK(chip.computedStyle().backgroundColor() == fixture.theme.palette.raised);
}

TEST_CASE("Avatar: initials, sizes, a status and a tint", "[components][avatar]") {
    CHECK(Avatar::initialsOf("Ada Lovelace") == "AL");
    CHECK(Avatar::initialsOf("Grace Brewster Murray Hopper") == "GH");
    CHECK(Avatar::initialsOf("plato") == "P");
    CHECK(Avatar::initialsOf("ada.lovelace@example.com") == "A");
    CHECK(Avatar::initialsOf("  ") == "");
    CHECK(Avatar::initialsOf("élodie roy") == "éR");

    Fixture fixture;
    auto &avatar = fixture.window->appContainer().add<Avatar>("Ada Lovelace").alignSelf(Align::Start);
    fixture.layout();
    CHECK(avatar.getWidth() == fixture.theme.metrics.pointer.controlSm);
    avatar.size(AvatarSize::Large).status(Tone::Success);
    fixture.layout();
    CHECK(avatar.getWidth() == 32);
    CHECK(avatar.getHeight() == 32);
    REQUIRE(avatar.skin());
    auto skin = std::dynamic_pointer_cast<PivotalAvatarSkin>(avatar.skin());
    REQUIRE(skin);

    // The tint: the same name gets the same, one of the swatches' hues; names spread over many of them
    const Color tint = PivotalAvatarSkin::tint("Ada Lovelace");
    CHECK(tint == PivotalAvatarSkin::tint("Ada Lovelace"));
    const auto hues = std::span(palette::pivotalSwatches).first(palette::pivotalHueCount);
    CHECK(std::ranges::find(hues, tint) != hues.end());
    std::set<std::uint32_t> picked{};
    for (const char *name: {"Ada Lovelace", "Grace Hopper", "Alan Turing", "Edsger Dijkstra", "Barbara Liskov",
                            "Donald Knuth", "Margaret Hamilton", "Ken Thompson", "Radia Perlman", "John McCarthy"}) {
        picked.insert(PivotalAvatarSkin::tint(name).argb());
    }
    CHECK(picked.size() >= 6);

    // The initials contrast with every hue: at least 3:1 (WCAG's large text), white or near black
    for (const Color hue: hues) {
        const Color text = PivotalAvatarSkin::initialsOn(hue);
        const auto  lum  = [](const Color color) {
            const auto channel = [](std::uint32_t value) {
                const double x = static_cast<double>(value & 0xFFu) / 255.0;
                return x <= 0.04045 ? x / 12.92 : std::pow((x + 0.055) / 1.055, 2.4);
            };
            return (0.2126 * channel(color.argb() >> 16)) + (0.7152 * channel(color.argb() >> 8)) + (0.0722 * channel(color.argb()));
        };
        const double a = lum(hue);
        const double b = lum(text);
        CHECK((std::max(a, b) + 0.05) / (std::min(a, b) + 0.05) >= 3.0);
    }
    CHECK(PivotalAvatarSkin::initialsOn(rgb(0xF2C94C)) != rgb(0xFFFFFF)); // Yellow: dark initials
    CHECK(PivotalAvatarSkin::initialsOn(rgb(0x5E6AD2)) == rgb(0xFFFFFF)); // Indigo: white

    avatar.colorful(true);
    fixture.layout();
    const Div *initials = skin->children()[0].get();
    CHECK(initials->computedStyle().color() == PivotalAvatarSkin::initialsOn(tint));
    avatar.colorful(false);
    fixture.layout();
    CHECK(initials->computedStyle().color() == fixture.theme.palette.textSecondary);
}

TEST_CASE("Kbd: a cap per key", "[components][kbd]") {
    Fixture fixture;
    auto &kbd = fixture.window->appContainer().add<Kbd>("⌘ K").alignSelf(Align::Start);
    fixture.layout();
    CHECK(kbd.caps() == std::vector<std::string>{"⌘", "K"});
    CHECK(kbd.children()[0]->getHeight() == 18);
    CHECK(kbd.children()[1]->getWidth() >= 18);
    kbd.keys("Ctrl  Shift P");
    CHECK(kbd.caps() == std::vector<std::string>{"Ctrl", "Shift", "P"});
}

TEST_CASE("EmptyState: an icon, a title, text and an action", "[components][empty]") {
    Fixture fixture;
    auto &empty = fixture.window->appContainer().add<EmptyState>("inbox", "No messages", "New messages show up here.");
    int  clicks = 0;
    empty.action("Compose").onClick([&]() { ++clicks; });
    fixture.layout();
    CHECK(empty.title() == "No messages");
    CHECK(empty.description() == "New messages show up here.");
    REQUIRE(empty.actionButton());
    CHECK(&empty.action("Write") == empty.actionButton()); // The same button, relabeled
    CHECK(empty.actionButton()->label() == "Write");
    fixture.click(*empty.actionButton());
    CHECK(clicks == 1);
}

// endregion
