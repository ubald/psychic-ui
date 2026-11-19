#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/AppShell.hpp>
#include <psychic-ui/components/PageHeader.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/components/Sidebar.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Length.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Shell {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 1000, 700};
        const Theme                 &theme{themes::pivotalDark};
        AppShell                    &shell{window->appContainer().add<AppShell>()};
        SidebarItem                 *inbox{nullptr};
        SidebarItem                 *cues{nullptr};
        SidebarItem                 *fixtures{nullptr};

        Shell() {
            shell.sidebar().title("Lights").mark("L");
            auto &top = shell.sidebar().section();
            inbox = &top.item("Inbox", "inbox").count("3");
            auto &show = shell.sidebar().section("Show");
            cues     = &show.item("Cues", "list-detail").count("42");
            fixtures = &show.item("Fixtures", "layers");
            layout();
        }

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

        static std::pair<int, int> origin(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y);
            return {x, y};
        }
    };
}

TEST_CASE("AppShell: the sidebar the window's height, the page inset beside it", "[components][appshell]") {
    Shell s;
    const Sidebar &sidebar = s.shell.sidebar();
    const Div     &page    = s.shell.page();
    const float   inset    = s.theme.metrics.frameInset;

    CHECK(s.shell.getWidth() == s.window->appContainer().getWidth());
    CHECK(s.shell.getHeight() == s.window->appContainer().getHeight());
    CHECK(sidebar.getWidth() == s.theme.metrics.sidebarWidth);
    CHECK(sidebar.getHeight() == s.shell.getHeight());
    CHECK(sidebar.header().getHeight() == s.theme.metrics.barHeader);

    // The page: flush with the sidebar, inset from the window's other edges, rounded, clipping
    const auto [pageX, pageY] = Shell::origin(page);
    const auto [shellX, shellY] = Shell::origin(s.shell);
    CHECK(pageX == shellX + static_cast<int>(sidebar.getWidth()));
    CHECK(pageY == shellY + static_cast<int>(inset));
    CHECK(page.getWidth() == s.shell.getWidth() - sidebar.getWidth() - inset);
    CHECK(page.getHeight() == s.shell.getHeight() - (2 * inset));
    CHECK(page.computedStyle().borderRadius() == s.theme.metrics.radius.page);
    CHECK(page.computedStyle().overflow() == Overflow::Hidden);
    CHECK(page.computedStyle().backgroundColor() == s.theme.palette.page);
    CHECK(s.shell.computedStyle().backgroundColor() == s.theme.palette.frame);

    // Sections after the first set apart by a hairline
    const Div *first  = s.inbox->parent();
    const Div *second = s.cues->parent();
    CHECK(first->computedStyle().borderTop() == 0.0f);
    CHECK(second->computedStyle().borderTop() == s.theme.metrics.border);

    // Rows: 32 px, the count after the label
    CHECK(s.cues->getHeight() == s.theme.metrics.pointer.rowSm);
    CHECK(s.cues->count() == "42");
    CHECK(s.fixtures->count().empty());
}

TEST_CASE("AppShell: one current item, picked by a click", "[components][appshell]") {
    Shell s;
    std::vector<SidebarItem *> changes{};
    s.shell.sidebar().onChange([&](SidebarItem *item) { changes.push_back(item); });
    CHECK(s.shell.sidebar().current() == nullptr);
    CHECK(s.shell.sidebar().items() == std::vector{s.inbox, s.cues, s.fixtures});

    s.click(*s.cues);
    CHECK(s.shell.sidebar().current() == s.cues);
    CHECK(s.cues->current());
    CHECK(s.cues->classNames().contains("current"));
    CHECK(s.cues->computedStyle().backgroundColor() == s.theme.palette.raised);
    CHECK(s.cues->computedStyle().color() == s.theme.palette.text);

    s.click(*s.fixtures);
    CHECK(s.fixtures->current());
    CHECK_FALSE(s.cues->current());
    CHECK_FALSE(s.cues->classNames().contains("current"));

    s.click(*s.fixtures); // Already current: no change
    s.fixtures->current(false);
    CHECK(s.shell.sidebar().current() == nullptr);
    s.shell.sidebar().select(s.inbox);
    CHECK(s.inbox->current());
    CHECK(changes == std::vector<SidebarItem *>{s.cues, s.fixtures, nullptr, s.inbox});

    // An item that acts instead of navigating only clicks
    auto &action = s.shell.sidebar().section().item("Theme", "sun").selectable(false);
    int  clicks  = 0;
    action.onClick([&]() { ++clicks; });
    s.layout();
    s.click(action);
    CHECK(clicks == 1);
    CHECK_FALSE(action.current());
    CHECK(s.inbox->current());
}

TEST_CASE("AppShell: the keyboard moves along the items and picks one", "[components][appshell]") {
    Shell s;
    s.key(Key::TAB);
    CHECK(s.inbox->hasFocus());
    CHECK(s.inbox->computedStyle().outlineWidth() == s.theme.metrics.focusRing);
    s.key(Key::DOWN);
    CHECK(s.cues->hasFocus()); // Into the next section
    CHECK(s.cues->computedStyle().outlineWidth() == s.theme.metrics.focusRing);
    CHECK(s.shell.sidebar().current() == nullptr); // The focus moves, not the current item
    s.key(Key::DOWN);
    CHECK(s.fixtures->hasFocus());
    s.key(Key::DOWN);
    CHECK(s.fixtures->hasFocus()); // The last one
    s.key(Key::HOME);
    CHECK(s.inbox->hasFocus());
    s.key(Key::END);
    CHECK(s.fixtures->hasFocus());
    s.key(Key::UP);
    CHECK(s.cues->hasFocus());
    s.key(Key::ENTER);
    CHECK(s.shell.sidebar().current() == s.cues);
    s.key(Key::DOWN);
    s.key(Key::SPACE);
    CHECK(s.shell.sidebar().current() == s.fixtures);
}

TEST_CASE("AppShell: the sidebar is one Tab stop, the current item else the first", "[components][appshell]") {
    Shell s;
    auto &after = s.shell.page().add<Button>("After");
    s.layout();
    const auto items = s.shell.sidebar().items();
    const auto inSidebar = [&]() {
        return std::ranges::any_of(items, [](const SidebarItem *item) { return item->hasFocus(); });
    };
    // Tab until the focus is on an item or the button after the sidebar (the window controls come first)
    const auto tabToStop = [&](const Mod mod = Mod{}) {
        for (int i = 0; i < 10; ++i) {
            s.window->handleKeyPress(Key::TAB, mod);
            s.layout();
            if (inSidebar() || after.hasFocus()) {
                return;
            }
        }
    };

    // Tab enters on the first item, and leaves the sidebar from it
    tabToStop();
    CHECK(s.inbox->hasFocus());
    tabToStop();
    CHECK(after.hasFocus());
    tabToStop();
    CHECK(s.inbox->hasFocus());

    // With a current item, it is the stop
    tabToStop();
    CHECK(after.hasFocus());
    s.shell.sidebar().select(s.cues);
    s.layout();
    CHECK(s.cues->focusEnabled());
    CHECK_FALSE(s.inbox->focusEnabled());
    CHECK_FALSE(s.fixtures->focusEnabled());
    tabToStop();
    CHECK(s.cues->hasFocus());

    // Moving away and leaving: the stop goes back to the current item
    s.key(Key::DOWN);
    CHECK(s.fixtures->hasFocus());
    tabToStop();
    CHECK(after.hasFocus());
    CHECK(s.cues->focusEnabled());
    CHECK_FALSE(s.fixtures->focusEnabled());
    tabToStop();
    CHECK(s.cues->hasFocus());

    // Shift+Tab from the page comes back to it too
    tabToStop();
    CHECK(after.hasFocus());
    tabToStop(Mod{.shift = true});
    CHECK(s.cues->hasFocus());
}

TEST_CASE("AppShell: the sidebar's header moves the window, its controls and the items don't", "[components][appshell][drag]") {
    const Shell s;
    const SidebarHeader &header = s.shell.sidebar().header();
    int x = 0;
    int y = 0;
    header.localToGlobal(x, y, header.getWidth() - 4, header.getHeight() / 2);
    CHECK(s.window->isDragArea(x, y));

    const auto [closeX, closeY] = Shell::center(header.controls().closeButton());
    CHECK_FALSE(s.window->isDragArea(closeX, closeY));
    // Grey until the pointer is over them
    CHECK(header.controls().closeButton().computedStyle().backgroundColor() == s.theme.palette.lineStrong);
    CHECK(header.controls().maximizeButton().computedStyle().backgroundColor() == s.theme.palette.lineStrong);

    const auto [itemX, itemY] = Shell::center(*s.inbox);
    CHECK_FALSE(s.window->isDragArea(itemX, itemY));
    CHECK(header.title() == "Lights");
}

TEST_CASE("Text levels: three classes the skin set colors", "[components][text]") {
    Shell s;
    auto &label = s.shell.page().add<Label>("Text");
    s.layout();
    CHECK(label.computedStyle().color() == s.theme.palette.text);

    label.addClassName("text-secondary");
    s.layout();
    CHECK(label.computedStyle().color() == s.theme.palette.textSecondary);

    label.removeClassName("text-secondary");
    label.addClassName("text-tertiary");
    s.layout();
    CHECK(label.computedStyle().color() == s.theme.palette.textMuted);

    label.removeClassName("text-tertiary");
    label.addClassName("text-primary");
    s.layout();
    CHECK(label.computedStyle().color() == s.theme.palette.text);

    // Under a secondary parent, inherited
    auto &box   = s.shell.page().add<Div>();
    auto &inner = box.add<Label>("Inner");
    box.addClassName("text-secondary");
    s.layout();
    CHECK(inner.computedStyle().color() == s.theme.palette.textSecondary);
}

TEST_CASE("AppShell: a page's scroller ends with the page panel, so its content's end shows", "[components][appshell][scroll]") {
    Shell s;
    Div &page = s.shell.page();
    page.add<PageHeader>("Long", "file");
    auto &content  = page.add<Div>().grow(1).shrink(1).basis(0);
    auto &scroller = content.add<Div>().height(100_pct).padding(24).overflow(Overflow::Scroll);
    scroller.add<Div>().height(2000).shrink(0);
    s.layout();

    // Within the page, under its header: never taller than the window
    const auto [pageX, pageY]         = Shell::origin(page);
    const auto [scrollerX, scrollerY] = Shell::origin(scroller);
    CHECK(scrollerY == pageY + static_cast<int>(s.theme.metrics.barHeader));
    CHECK(scrollerY + scroller.getHeight() == pageY + page.getHeight());

    // Scrolled all the way, the content's end shows, with the scroller's bottom padding after it
    scroller.scroll(0, -100000);
    s.layout();
    CHECK(scroller.scrollY() == static_cast<int>(scroller.getHeight()) - (24 + 2000 + 24));
}

namespace {
    // A session item holding two banks, then a patch item, in a section after the show's
    struct Nested : Shell {
        SidebarItem *session{nullptr};
        SidebarItem *bank1{nullptr};
        SidebarItem *bank2{nullptr};
        SidebarItem *patch{nullptr};

        Nested() {
            auto &section = shell.sidebar().section("Session");
            session       = &section.item("Session", "layers");
            bank1         = &session->item("Bank 1", "list");
            bank2         = &session->item("Bank 2", "list");
            patch         = &section.item("Patch", "settings");
            layout();
        }
    };
}

TEST_CASE("Sidebar: items nested under an item, the parent marked while one is current", "[components][appshell][sidebar]") {
    Nested s;
    Sidebar &sidebar = s.shell.sidebar();
    CHECK(sidebar.items() == std::vector{s.inbox, s.cues, s.fixtures, s.session, s.bank1, s.bank2, s.patch});
    CHECK(s.session->items() == std::vector{s.bank1, s.bank2});
    CHECK(s.bank1->parentItem() == s.session);
    CHECK(s.session->parentItem() == nullptr);

    // Collapsed at first: the banks don't show
    CHECK_FALSE(s.session->expanded());
    CHECK(s.bank1->getHeight() == 0);

    // A bank made current: the session expands and says it holds the current item
    std::vector<SidebarItem *> changes{};
    sidebar.onChange([&](SidebarItem *item) { changes.push_back(item); });
    sidebar.select(s.bank2);
    s.layout();
    CHECK(sidebar.current() == s.bank2);
    CHECK_FALSE(s.session->current());
    CHECK(s.session->expanded());
    CHECK(s.session->classNames().contains("has-current"));
    CHECK_FALSE(s.session->classNames().contains("current"));
    CHECK(s.session->computedStyle().color() == s.theme.palette.text);
    CHECK(s.bank2->getHeight() == s.theme.metrics.pointer.rowSm);

    // Indented by the rules
    const auto [sessionX, sessionY] = Shell::origin(*s.session);
    const auto [bankX, bankY]       = Shell::origin(*s.bank2);
    CHECK(bankX > sessionX);
    CHECK(bankY > sessionY);

    // Another item current: the session isn't marked any more, and stays expanded
    s.click(*s.patch);
    CHECK(s.patch->current());
    CHECK_FALSE(s.session->classNames().contains("has-current"));
    CHECK(s.session->expanded());
    s.click(*s.bank1);
    CHECK(s.bank1->current());
    CHECK(s.session->classNames().contains("has-current"));
    CHECK(changes == std::vector<SidebarItem *>{s.bank2, s.patch, s.bank1});

    // Several levels: every parent up the line is marked
    auto &deep = s.bank1->item("Deep");
    sidebar.select(&deep);
    s.layout();
    CHECK(s.bank1->classNames().contains("has-current"));
    CHECK(s.session->classNames().contains("has-current"));
    CHECK(s.bank1->expanded());
}

TEST_CASE("Sidebar: collapsing hides the nested items, expanding shows them", "[components][appshell][sidebar]") {
    Nested s;
    Sidebar &sidebar = s.shell.sidebar();
    s.session->expanded(true);
    s.layout();
    CHECK(s.session->classNames().contains("expanded"));
    CHECK(s.bank1->getHeight() == s.theme.metrics.pointer.rowSm);
    const auto [patchX, patchY] = Shell::origin(*s.patch);

    s.session->expanded(false);
    s.layout();
    CHECK_FALSE(s.session->classNames().contains("expanded"));
    CHECK(s.bank1->getHeight() == 0);
    CHECK(s.bank2->getHeight() == 0);
    const auto [patchX2, patchY2] = Shell::origin(*s.patch);
    CHECK(patchY2 < patchY); // The items after move up
    s.session->toggle();
    s.layout();
    CHECK(s.session->expanded());
    CHECK(s.bank1->getHeight() == s.theme.metrics.pointer.rowSm);

    // A click on the chevron expands or collapses without making the parent current
    Div &chevron = *s.session->at(s.session->childCount() - 1);
    REQUIRE(chevron.classNames().contains("disclosure"));
    s.click(chevron);
    CHECK_FALSE(s.session->expanded());
    CHECK(sidebar.current() == nullptr);
    s.click(chevron);
    CHECK(s.session->expanded());
    CHECK(sidebar.current() == nullptr);
    s.click(*s.session); // On the rest of the row: current
    CHECK(sidebar.current() == s.session);
    CHECK(s.session->expanded());

    // A collapsed parent keeps its current child current, and is the Tab stop in its place
    sidebar.select(s.bank2);
    s.layout();
    s.session->expanded(false);
    s.layout();
    CHECK(sidebar.current() == s.bank2);
    CHECK(s.session->classNames().contains("has-current"));
    CHECK(s.session->focusEnabled());
    CHECK_FALSE(s.bank2->focusEnabled());

    // Collapsing the parent of the focused item focuses the parent
    s.session->expanded(true);
    s.layout();
    s.window->requestFocus(s.bank1, true);
    s.session->expanded(false);
    s.layout();
    CHECK(s.session->hasFocus());
}

TEST_CASE("Sidebar: the keyboard walks the nested items", "[components][appshell][sidebar]") {
    Nested s;
    Sidebar &sidebar = s.shell.sidebar();
    s.window->requestFocus(s.fixtures, true);

    // Collapsed: the arrows skip the banks
    s.key(Key::DOWN);
    CHECK(s.session->hasFocus());
    s.key(Key::DOWN);
    CHECK(s.patch->hasFocus());
    s.key(Key::UP);
    CHECK(s.session->hasFocus());

    // Right expands, then goes down into the banks with Down
    s.key(Key::RIGHT);
    CHECK(s.session->expanded());
    CHECK(s.session->hasFocus());
    s.key(Key::DOWN);
    CHECK(s.bank1->hasFocus());
    s.key(Key::DOWN);
    CHECK(s.bank2->hasFocus());
    s.key(Key::DOWN);
    CHECK(s.patch->hasFocus());
    s.key(Key::UP);
    CHECK(s.bank2->hasFocus());
    s.key(Key::END);
    CHECK(s.patch->hasFocus());
    s.key(Key::HOME);
    CHECK(s.inbox->hasFocus());

    // Enter makes a nested item current
    s.window->requestFocus(s.bank2, true);
    s.key(Key::ENTER);
    CHECK(sidebar.current() == s.bank2);

    // Left on a nested item goes to its parent, then collapses it
    s.key(Key::LEFT);
    CHECK(s.session->hasFocus());
    CHECK(s.session->expanded());
    s.key(Key::LEFT);
    CHECK_FALSE(s.session->expanded());
    s.key(Key::DOWN);
    CHECK(s.patch->hasFocus());
    CHECK(sidebar.current() == s.bank2);
}

TEST_CASE("Sidebar: a header without window controls, with the app's mark and lines under its name", "[components][appshell][sidebar][drag]") {
    Shell s;
    SidebarHeader &header = s.shell.sidebar().header();
    CHECK(header.windowControls());
    CHECK(header.controls().parent() == &header);

    header.windowControls(false);
    s.layout();
    CHECK_FALSE(header.windowControls());
    CHECK(header.controls().parent() == nullptr);
    // Nothing of it is left to Tab to: the first Tab lands on the first item
    s.key(Key::TAB);
    CHECK(s.inbox->hasFocus());

    // The app's own mark in place of the letters, and lines under the name
    auto &mark = header.mark().add<Div>().size(20, 20);
    auto &show = header.details().add<Label>("Summer Tour");
    auto &venue = header.details().add<Label>("Main Hall");
    s.layout();
    const auto [markX, markY]   = Shell::origin(mark);
    const auto [titleX, titleY] = Shell::origin(*header.brand().at(1));
    const auto [showX, showY]   = Shell::origin(show);
    const auto [venueX, venueY] = Shell::origin(venue);
    CHECK(mark.getWidth() == 20);
    CHECK(markX < titleX);
    CHECK(showY > titleY);
    CHECK(venueY > showY);
    CHECK(header.getHeight() >= header.brand().getHeight() + show.getHeight() + venue.getHeight());

    // Still the drag area, over the lines too; the brand made a menu trigger isn't
    const auto [lineX, lineY] = Shell::center(venue);
    CHECK(s.window->isDragArea(lineX, lineY));
    int menus = 0;
    header.brand().onClick([&]() { ++menus; });
    const auto [brandX, brandY] = Shell::center(header.brand());
    CHECK_FALSE(s.window->isDragArea(brandX, brandY));
    s.click(header.brand());
    CHECK(menus == 1);

    // Back on: first again
    header.windowControls(true);
    s.layout();
    CHECK(header.controls().parent() == &header);
    CHECK(header.at(0) == &header.controls());
}
