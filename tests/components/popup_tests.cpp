#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/SearchField.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/MenuBar.hpp>
#include <psychic-ui/components/MenuButton.hpp>
#include <psychic-ui/components/Select.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/Toast.hpp>
#include <psychic-ui/components/Tooltip.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;
using namespace std::chrono_literals;

// region Placement

TEST_CASE("A popup goes on its side of the anchor, aligned and offset", "[popup][placement]") {
    const SkRect bounds = SkRect::MakeWH(800, 600);
    const SkRect anchor = SkRect::MakeXYWH(100, 100, 80, 20);

    auto below = placePopup(anchor, 200, 100, bounds, Side::Bottom, PopupAlign::Start, 6);
    CHECK(below.side == Side::Bottom);
    CHECK(below.x == 100);
    CHECK(below.y == 126);

    auto above = placePopup(SkRect::MakeXYWH(100, 300, 80, 20), 200, 100, bounds, Side::Top, PopupAlign::Center, 6);
    CHECK(above.side == Side::Top);
    CHECK(above.x == 40); // 140 - 100
    CHECK(above.y == 194); // 300 - 6 - 100

    auto end = placePopup(SkRect::MakeXYWH(300, 100, 80, 20), 200, 100, bounds, Side::Bottom, PopupAlign::End, 0, 10);
    CHECK(end.x == 170); // 380 - 200 - 10
}

TEST_CASE("A popup flips to the other side when it doesn't fit", "[popup][placement]") {
    const SkRect bounds = SkRect::MakeXYWH(12, 12, 776, 576);

    SECTION("below, at the bottom: above") {
        auto p = placePopup(SkRect::MakeXYWH(100, 540, 80, 20), 200, 100, bounds, Side::Bottom, PopupAlign::Start, 6);
        CHECK(p.side == Side::Top);
        CHECK(p.y == 434); // 540 - 6 - 100
    }

    SECTION("above, at the top: below") {
        auto p = placePopup(SkRect::MakeXYWH(100, 20, 80, 20), 200, 30, bounds, Side::Top, PopupAlign::Start, 6);
        CHECK(p.side == Side::Bottom);
        CHECK(p.y == 46);
    }

    SECTION("right, at the right edge: left (a submenu)") {
        auto p = placePopup(SkRect::MakeXYWH(600, 100, 180, 32), 200, 100, bounds, Side::Right, PopupAlign::Start, 4, -5);
        CHECK(p.side == Side::Left);
        CHECK(p.x == 396); // 600 - 4 - 200
        CHECK(p.y == 95);
    }

    SECTION("neither side fits: the one with more room, shifted inside") {
        auto p = placePopup(SkRect::MakeXYWH(100, 200, 80, 20), 200, 500, bounds, Side::Bottom, PopupAlign::Start, 6);
        CHECK(p.side == Side::Bottom); // 362 below, 182 above
        CHECK(p.y == 88);               // 588 - 500
    }
}

TEST_CASE("A popup shifts along its side to stay inside", "[popup][placement]") {
    const SkRect bounds = SkRect::MakeXYWH(12, 12, 776, 576);

    auto right = placePopup(SkRect::MakeXYWH(700, 100, 80, 20), 200, 100, bounds, Side::Bottom, PopupAlign::Start, 6);
    CHECK(right.side == Side::Bottom);
    CHECK(right.x == 588); // 788 - 200

    auto left = placePopup(SkRect::MakeXYWH(0, 100, 20, 20), 200, 100, bounds, Side::Bottom, PopupAlign::Center, 6);
    CHECK(left.x == 12);

    auto wide = placePopup(SkRect::MakeXYWH(100, 100, 20, 20), 1000, 100, bounds, Side::Bottom, PopupAlign::Start, 6);
    CHECK(wide.x == 12); // Larger than the bounds: at their start
}

// endregion

namespace {
    /**
     * A window with a row of buttons at the top of the app container
     */
    struct Popups {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 800, 600};
        Div                         &bar{window->appContainer().add<Div>()};
        Button                      &first{bar.add<Button>("First")};
        Button                      &second{bar.add<Button>("Second")};
        int                         firstClicks{0};
        int                         secondClicks{0};

        Popups() {
            bar.direction(Direction::Row).padding(20).gap(10);
            first.onClick([this]() { ++firstClicks; });
            second.onClick([this]() { ++secondClicks; });
            layout();
        }

        void layout() { systemWindow.layout(); }

        void press(int x, int y, MouseButton button = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = button, .down = true});
            layout();
        }

        void release(int x, int y, MouseButton button = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = button, .down = false});
            layout();
        }

        void clickAt(int x, int y) {
            press(x, y);
            release(x, y);
        }

        void click(const Div &div) {
            const auto [x, y] = center(div);
            clickAt(x, y);
        }

        void moveTo(int x, int y) {
            window->handleMouseMove({.x = static_cast<float>(x), .y = static_cast<float>(y)}, false);
            layout();
        }

        void hover(const Div &div) {
            const auto [x, y] = center(div);
            moveTo(x, y);
        }

        void key(Key key, Mod mod = Mod{}) {
            window->handleKeyPress(key, mod);
            layout();
        }

        static std::pair<int, int> origin(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y);
            return {x - div.scrollX(), y - div.scrollY()};
        }

        static std::pair<int, int> center(const Div &div) {
            const auto [x, y] = origin(div);
            return {x + (div.getWidth() / 2), y + (div.getHeight() / 2)};
        }

        std::size_t popupCount() const { return window->overlay().children().size(); }

        template<class T>
        T *top() const { return window->overlay().top<T>(); }
    };

    bool hasClass(const Div &div, const std::string &name) {
        return div.classNames().contains(name);
    }
}

// region Overlay

TEST_CASE("A menu opens under its anchor, and above it at the window's bottom", "[popup][menu]") {
    Popups f{};
    const Menu &menu = f.window->openMenu({Menu::item("One", []() {}), Menu::item("Two", []() {})}, f.first);
    f.layout();
    const auto [ax, ay] = Popups::origin(f.first);
    const auto [mx, my] = Popups::origin(menu);
    CHECK(mx == ax);
    CHECK(my == ay + f.first.getHeight() + 6);
    CHECK(menu.placedSide() == Side::Bottom);

    // An anchor at the bottom of the window
    f.bar.marginTop(560);
    f.layout();
    f.layout();
    CHECK(menu.placedSide() == Side::Top);
    const auto [bx, by] = Popups::origin(f.first);
    CHECK(Popups::origin(menu).second == by - 6 - menu.getHeight());
}

TEST_CASE("A popup whose anchor leaves the window closes", "[popup]") {
    Popups f{};
    f.window->openMenu({Menu::item("One", []() {})}, f.second);
    f.layout();
    REQUIRE(f.popupCount() == 1);
    f.bar.remove(f.second);
    f.layout();
    CHECK(f.popupCount() == 0);
}

TEST_CASE("A press outside a menu closes it and goes no further", "[popup][menu][dismiss]") {
    Popups f{};
    int closed = 0;
    f.window->onMenuClose([&]() { ++closed; });
    f.window->openMenu({Menu::item("One", []() {})}, f.first);
    f.layout();

    // On the second button, under the modal menu's reach
    f.click(f.second);
    CHECK(f.popupCount() == 0);
    CHECK(closed == 1);
    CHECK(f.secondClicks == 0); // Swallowed with its release

    // The next press goes through
    f.click(f.second);
    CHECK(f.secondClicks == 1);
}

TEST_CASE("A press inside a menu doesn't close it, Escape does", "[popup][menu][dismiss]") {
    Popups f{};
    const Menu &menu = f.window->openMenu({Menu::check("Check", false)}, f.first);
    f.layout();
    f.click(*menu.rows()[0]); // A check item keeps the menu open
    CHECK(f.popupCount() == 1);
    f.key(Key::ESCAPE);
    CHECK(f.popupCount() == 0);
}

TEST_CASE("A modal menu takes the mouse from the content, except its pass-through divs", "[popup][menu]") {
    Popups f{};
    Menu &menu = f.window->openMenu({Menu::item("One", []() {})}, f.first);
    menu.passThrough(f.bar);
    f.layout();
    CHECK_FALSE(f.window->isDragArea(5, 5));
    f.hover(f.second);
    CHECK(f.second.mouseOver());
    f.click(f.second); // Not outside: it reaches the button
    CHECK(f.secondClicks == 1);
}

TEST_CASE("Non-modal popups let the mouse through around them", "[popup]") {
    Popups f{};
    auto &tooltip = f.window->overlay().open<Tooltip>("Tip");
    tooltip.anchor(f.second);
    f.layout();
    REQUIRE(f.popupCount() == 1);
    // A press anywhere closes a tooltip, and goes on
    f.click(f.first);
    CHECK(f.firstClicks == 1);
    CHECK(f.popupCount() == 0);
}

// endregion

// region Menus

TEST_CASE("The keyboard moves along a menu's rows and chooses one", "[popup][menu][keyboard]") {
    Popups f{};
    int  chosen = 0;
    auto disabled = Menu::item("Disabled", []() {});
    disabled->enabled = false;
    Menu &menu = f.window->openMenu(
        {
            Menu::group("Group"),
            Menu::item("One", [&]() { chosen = 1; }),
            disabled,
            Menu::separator(),
            Menu::item("Three", [&]() { chosen = 3; }),
        }, f.first
    );
    f.layout();
    REQUIRE(menu.rows().size() == 3); // Not the group label or the separator
    CHECK(f.window->focusedDiv() == &menu);
    CHECK(menu.highlighted() == -1); // Opened with the mouse

    f.key(Key::DOWN);
    CHECK(menu.highlighted() == 0);
    CHECK(menu.rows()[0]->selected());
    f.key(Key::DOWN); // Over the disabled one
    CHECK(menu.highlighted() == 2);
    f.key(Key::DOWN); // Wraps
    CHECK(menu.highlighted() == 0);
    f.key(Key::END);
    CHECK(menu.highlighted() == 2);
    f.key(Key::ENTER);
    CHECK(chosen == 3);
    CHECK(f.popupCount() == 0);
}

TEST_CASE("A menu opened from the keyboard highlights its first row; closing gives the focus back", "[popup][menu][focus]") {
    Popups f{};
    f.first.onClick([&]() { f.window->openMenu({Menu::item("One", []() {}), Menu::item("Two", []() {})}, f.first); });
    f.key(Key::TAB); // The first button, from the keyboard
    REQUIRE(f.window->focusedDiv() == &f.first);
    f.key(Key::ENTER);
    auto *menu = f.top<Menu>();
    REQUIRE(menu != nullptr);
    CHECK(menu->highlighted() == 0);
    f.key(Key::ESCAPE);
    CHECK(f.popupCount() == 0);
    CHECK(f.window->focusedDiv() == &f.first);
    CHECK(f.window->focusVisible());
}

TEST_CASE("Submenus open by hover, Right and Enter, and close by Left and Escape", "[popup][menu][keyboard]") {
    Popups f{};
    int  chosen = 0;
    Menu &menu = f.window->openMenu(
        {
            Menu::item("One", []() {}),
            Menu::item("More", {Menu::item("Deep", [&]() { chosen = 1; }), Menu::item("Deeper", []() {})}),
        }, f.first
    );
    f.layout();

    SECTION("hover") {
        f.hover(*menu.rows()[1]);
        const Menu *sub = menu.submenu();
        REQUIRE(sub != nullptr);
        CHECK(sub->owner() == &menu);
        CHECK(f.window->focusedDiv() == &menu); // Hovering leaves the focus
        CHECK(sub->placedSide() == Side::Right);
        // Level with its row: its first row's top on the row's top
        f.layout();
        CHECK(Popups::origin(*sub->rows()[0]).second == Popups::origin(*menu.rows()[1]).second);
        // Hovering another row closes it
        f.hover(*menu.rows()[0]);
        CHECK(menu.submenu() == nullptr);
        CHECK(f.popupCount() == 1);
    }

    SECTION("keyboard") {
        f.key(Key::DOWN);
        f.key(Key::DOWN);
        f.key(Key::RIGHT);
        const Menu *sub = menu.submenu();
        REQUIRE(sub != nullptr);
        CHECK(f.window->focusedDiv() == sub);
        CHECK(sub->highlighted() == 0);
        f.key(Key::LEFT);
        CHECK(menu.submenu() == nullptr);
        CHECK(f.window->focusedDiv() == &menu);
        CHECK(f.popupCount() == 1);

        f.key(Key::ENTER); // On "More" still
        sub = menu.submenu();
        REQUIRE(sub != nullptr);
        f.key(Key::ESCAPE); // Only the submenu
        CHECK(f.popupCount() == 1);
        CHECK(f.window->focusedDiv() == &menu);

        f.key(Key::RIGHT);
        f.key(Key::ENTER); // "Deep"
        CHECK(chosen == 1);
        CHECK(f.popupCount() == 0);
    }

    SECTION("a press outside closes the whole chain") {
        f.hover(*menu.rows()[1]);
        REQUIRE(f.popupCount() == 2);
        f.clickAt(790, 590);
        CHECK(f.popupCount() == 0);
    }
}

TEST_CASE("A submenu flips to the left at the window's right edge", "[popup][menu]") {
    Popups f{};
    f.bar.justify(Justify::End);
    f.layout();
    const Menu &menu = f.window->openMenu({Menu::item("More", {Menu::item("Deep", []() {})})}, f.second, Side::Bottom, PopupAlign::End);
    f.layout();
    f.hover(*menu.rows()[0]);
    const Menu *sub = menu.submenu();
    REQUIRE(sub != nullptr);
    f.layout();
    CHECK(sub->placedSide() == Side::Left);
    CHECK(Popups::origin(*sub).first + sub->getWidth() == Popups::origin(*menu.rows()[0]).first - 4);
}

TEST_CASE("Check and radio items keep their state and the menu open", "[popup][menu]") {
    Popups f{};
    std::vector<bool> toggles{};
    std::string       picked{};
    auto group = std::make_shared<MenuRadioGroup>("a", [&](const std::string &value) { picked = value; });
    const MenuItems items{
        Menu::check("Check", false, [&](bool on) { toggles.push_back(on); }),
        Menu::radio(group, "A", "a"),
        Menu::radio(group, "B", "b"),
        Menu::item("Plain", []() {}),
    };
    const Menu &menu = f.window->openMenu(items, f.first);
    f.layout();
    CHECK(menu.rows()[1]->checked());
    CHECK(hasClass(*menu.rows()[1], "checked"));

    f.click(*menu.rows()[0]);
    f.click(*menu.rows()[2]);
    CHECK(f.popupCount() == 1);
    CHECK(toggles == std::vector<bool>{true});
    CHECK(picked == "b");
    CHECK(group->value == "b");
    CHECK(menu.rows()[0]->checked());
    CHECK_FALSE(menu.rows()[1]->checked());
    CHECK(menu.rows()[2]->checked());

    // Every row keeps the check column
    for (const MenuButton *button: menu.rows()) {
        CHECK(button->checkColumn());
    }

    // The items remember, the next menu shows it
    f.key(Key::ESCAPE);
    const Menu &again = f.window->openMenu(items, f.first);
    f.layout();
    CHECK(again.rows()[0]->checked());
    CHECK(again.rows()[2]->checked());
}

TEST_CASE("Menu bars switch menus with Left and Right", "[popup][menu][keyboard]") {
    Popups f{};
    auto &bar = f.window->appContainer().add<MenuBar>(
        MenuItems{
            Menu::item("File", {Menu::item("New", []() {})}),
            Menu::item("Edit", {Menu::item("Copy", []() {})}),
        }
    );
    f.layout();
    auto *file = static_cast<Button *>(bar.children()[0].get());
    f.click(*file);
    REQUIRE(f.top<Menu>() != nullptr);
    CHECK(f.top<Menu>()->rows()[0]->menuItem()->label == "New");
    f.key(Key::RIGHT);
    REQUIRE(f.top<Menu>() != nullptr);
    CHECK(f.top<Menu>()->rows()[0]->menuItem()->label == "Copy");
    f.key(Key::RIGHT); // Wraps
    CHECK(f.top<Menu>()->rows()[0]->menuItem()->label == "New");
    CHECK(f.popupCount() == 1);
}

TEST_CASE("A context menu opens at a point, inside the window", "[popup][menu]") {
    Popups f{};
    const Menu &menu = f.window->openContextMenu({Menu::item("One", []() {}), Menu::item("Two", []() {})}, 790, 590);
    f.layout();
    CHECK(hasClass(menu, "context"));
    const auto [x, y] = Popups::origin(menu);
    CHECK(x + menu.getWidth() <= 800 - 12);
    CHECK(y + menu.getHeight() <= 590); // Flipped above the point
}

// endregion

// region Context menus

namespace {
    /**
     * An area with a context menu of three items (the first disabled), holding a button with one of its own
     */
    struct ContextMenus : Popups {
        Div    &area{window->appContainer().add<Div>()};
        Button &inner{area.add<Button>("Inner")};
        int    chosen{0};

        ContextMenus() {
            auto disabled = Menu::item("Undo", []() {});
            disabled->enabled = false;
            area.size(300, 200).padding(20).focusEnabled(true);
            area.contextMenu({disabled, Menu::item("Copy", [this]() { ++chosen; }), Menu::item("Paste", []() {})});
            layout();
        }

        Menu *menu() const { return top<Menu>(); }
    };
}

TEST_CASE("A right press opens a div's context menu where it pressed", "[popup][menu][context]") {
    ContextMenus f{};
    const auto [ax, ay] = Popups::origin(f.area);
    const int x = ax + 250;
    const int y = ay + 150;
    f.press(x, y, MouseButton::RIGHT);
    REQUIRE(f.menu() != nullptr);
    CHECK(hasClass(*f.menu(), "context"));
    CHECK(f.menu()->rows().size() == 3);
    CHECK(f.menu()->highlighted() == -1); // From the mouse: nothing highlighted
    const auto [mx, my] = Popups::origin(*f.menu());
    CHECK(mx == x);
    CHECK(my == y);
    f.release(x, y, MouseButton::RIGHT);
    CHECK(f.menu() != nullptr); // The release doesn't choose or close

    // A left press does nothing of the kind, and no right press outside opens one
    f.key(Key::ESCAPE);
    REQUIRE(f.menu() == nullptr);
    f.clickAt(x, y);
    CHECK(f.menu() == nullptr);
    const auto [bx, by] = Popups::center(f.first);
    f.press(bx, by, MouseButton::RIGHT);
    CHECK(f.menu() == nullptr);
}

TEST_CASE("The deepest div with a context menu opens its own", "[popup][menu][context]") {
    ContextMenus f{};
    f.inner.contextMenu({Menu::item("Rename", []() {})});
    const auto [x, y] = Popups::center(f.inner);
    f.press(x, y, MouseButton::RIGHT);
    REQUIRE(f.menu() != nullptr);
    REQUIRE(f.menu()->rows().size() == 1);
    CHECK(f.menu()->rows()[0]->menuItem()->label == "Rename");
}

TEST_CASE("A right press elsewhere while a context menu is open opens it there", "[popup][menu][context]") {
    ContextMenus f{};
    f.inner.contextMenu({Menu::item("Rename", []() {})});
    const auto [ax, ay] = Popups::origin(f.area);
    f.press(ax + 250, ay + 150, MouseButton::RIGHT);
    f.release(ax + 250, ay + 150, MouseButton::RIGHT);
    REQUIRE(f.menu() != nullptr);

    // Elsewhere in the same div: closed, and open again at the new point
    f.press(ax + 150, ay + 170, MouseButton::RIGHT);
    REQUIRE(f.menu() != nullptr);
    CHECK(f.window->overlay().popups().size() == 1);
    const auto [mx, my] = Popups::origin(*f.menu());
    CHECK(mx == ax + 150);
    CHECK(my == ay + 170);
    f.release(ax + 150, ay + 170, MouseButton::RIGHT);
    CHECK(f.menu() != nullptr); // Its release doesn't close it

    // On another div with a menu of its own: that one opens
    const auto [ix, iy] = Popups::center(f.inner);
    f.press(ix, iy, MouseButton::RIGHT);
    REQUIRE(f.menu() != nullptr);
    REQUIRE(f.menu()->rows().size() == 1);
    CHECK(f.menu()->rows()[0]->menuItem()->label == "Rename");
    f.release(ix, iy, MouseButton::RIGHT);

    // Where nothing has one: only closed, and the press reaches what is there
    const auto [bx, by] = Popups::center(f.first);
    int pressed = 0;
    f.first.onMousePress([&]() { ++pressed; });
    f.press(bx, by, MouseButton::RIGHT);
    CHECK(f.menu() == nullptr);
    CHECK(pressed == 1);
    f.release(bx, by, MouseButton::RIGHT);

    // A left press outside still only closes it
    f.press(ax + 250, ay + 150, MouseButton::RIGHT);
    REQUIRE(f.menu() != nullptr);
    f.clickAt(bx, by);
    CHECK(f.menu() == nullptr);
    CHECK(f.firstClicks == 0);
}

TEST_CASE("On the right button's press by default, on its release as on Windows", "[popup][menu][context]") {
    #ifdef _WIN32
    CHECK(std::make_shared<Window>("default")->contextMenuOnRelease());
    #else
    CHECK_FALSE(std::make_shared<Window>("default")->contextMenuOnRelease());
    #endif
    ContextMenus f{};
    f.window->contextMenuOnRelease(true);
    const auto [ax, ay] = Popups::origin(f.area);
    int requests = 0;
    int released = 0;
    f.area.onMouseRelease([&]() { ++released; });
    f.area.onContextMenuRequest([&]() {
        ++requests;
        if (requests == 1) {
            CHECK(released == 1); // After the release's signals
        }
    });
    f.press(ax + 250, ay + 150, MouseButton::RIGHT);
    CHECK(f.menu() == nullptr);
    f.release(ax + 260, ay + 160, MouseButton::RIGHT);
    REQUIRE(f.menu() != nullptr);
    CHECK(requests == 1);
    const auto [mx, my] = Popups::origin(*f.menu());
    CHECK(mx == ax + 260); // Where it was released
    CHECK(my == ay + 160);

    // Elsewhere: the press closes it, the release opens it again there
    f.press(ax + 150, ay + 170, MouseButton::RIGHT);
    CHECK(f.menu() == nullptr);
    f.release(ax + 150, ay + 170, MouseButton::RIGHT);
    REQUIRE(f.menu() != nullptr);
    CHECK(requests == 2);
    f.key(Key::ESCAPE);

    // A touch long press still opens it while held, the keyboard on its key
    f.window->handleTouchPress(static_cast<float>(ax + 250), static_cast<float>(ay + 150), 0ms);
    f.window->updateGestures(Window::longPressDelay);
    f.layout();
    CHECK(f.menu() != nullptr);
    f.window->handleTouchRelease(static_cast<float>(ax + 250), static_cast<float>(ay + 150), Window::longPressDelay + 100ms);
    f.layout();
    CHECK(f.menu() != nullptr);
    CHECK(requests == 3);
    f.key(Key::ESCAPE);
    f.window->requestFocus(&f.area, true);
    f.key(Key::MENU);
    CHECK(f.menu() != nullptr);
    CHECK(requests == 4);
}

TEST_CASE("Control and the left button is the right one on macOS", "[popup][menu][context]") {
    ContextMenus f{};
    #ifdef __APPLE__
    CHECK(f.window->ctrlClickIsRightClick());
    #else
    CHECK_FALSE(f.window->ctrlClickIsRightClick());
    #endif
    const auto [ax, ay] = Popups::origin(f.area);
    const auto [bx, by] = Popups::center(f.first);
    std::vector<MouseButton> buttons{};
    f.first.onMouseButtonChange([&](const MouseEvent &e) { buttons.push_back(e.button); });
    const auto ctrlClick = [&](int x, int y, Mod mod) {
        f.window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true, .modifiers = mod});
        f.layout();
        // Control may go up first: the release is still the right button's
        f.window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false});
        f.layout();
    };

    // Off: a left click with Control
    f.window->ctrlClickIsRightClick(false);
    ctrlClick(bx, by, Mod{.ctrl = true});
    CHECK(f.firstClicks == 1);
    CHECK(buttons == std::vector{MouseButton::LEFT, MouseButton::LEFT});

    // On: a right press and release, no click, the context menu
    f.window->ctrlClickIsRightClick(true);
    buttons.clear();
    ctrlClick(bx, by, Mod{.ctrl = true});
    CHECK(f.firstClicks == 1);
    CHECK(buttons == std::vector{MouseButton::RIGHT, MouseButton::RIGHT});
    ctrlClick(ax + 250, ay + 150, Mod{.ctrl = true});
    REQUIRE(f.menu() != nullptr);
    const auto [mx, my] = Popups::origin(*f.menu());
    CHECK(mx == ax + 250);
    CHECK(my == ay + 150);
    f.key(Key::ESCAPE);

    // With another modifier, it stays the left button
    buttons.clear();
    ctrlClick(bx, by, Mod{.shift = true, .ctrl = true});
    CHECK(f.firstClicks == 2);
    CHECK(buttons == std::vector{MouseButton::LEFT, MouseButton::LEFT});
    CHECK(f.menu() == nullptr);
}

TEST_CASE("An empty context menu removes it", "[popup][menu][context]") {
    ContextMenus f{};
    f.area.contextMenu({});
    CHECK(!f.area.contextMenuRequested.hasSubscriptions());
    const auto [x, y] = Popups::center(f.area);
    f.press(x + 100, y, MouseButton::RIGHT);
    CHECK(f.menu() == nullptr);
}

TEST_CASE("The Menu key and Shift+F10 open the focused div's context menu at its first item", "[popup][menu][context][keyboard]") {
    ContextMenus f{};
    f.window->requestFocus(&f.area, false); // Even after a click: the keyboard asked for it
    f.key(Key::MENU);
    REQUIRE(f.menu() != nullptr);
    CHECK(f.menu()->highlighted() == 1); // The first that can be chosen: Undo is disabled
    CHECK(f.menu()->hasFocus());
    CHECK(f.window->focusVisible());
    const auto [mx, my] = Popups::origin(*f.menu());
    const auto [cx, cy] = Popups::center(f.area);
    CHECK(mx == cx);
    CHECK(my == cy);

    f.key(Key::ENTER);
    CHECK(f.chosen == 1);
    CHECK(f.menu() == nullptr);
    CHECK(f.area.hasFocus()); // The focus came back

    f.key(Key::F10, Mod{.shift = true});
    CHECK(f.menu() != nullptr);
}

TEST_CASE("A touch long press opens the context menu instead of the tooltip", "[popup][menu][context][touch]") {
    ContextMenus f{};
    f.area.tooltip("Tip");
    const auto [ax, ay] = Popups::origin(f.area);
    const auto x = static_cast<float>(ax + 250);
    const auto y = static_cast<float>(ay + 150);
    f.window->handleTouchPress(x, y, 0ms);
    f.window->updateGestures(Window::longPressDelay);
    f.layout();
    REQUIRE(f.menu() != nullptr);
    CHECK(f.window->tooltip() == nullptr);
    f.window->handleTouchRelease(x, y, Window::longPressDelay + 100ms);
    f.layout();
    CHECK(f.menu() != nullptr); // Lifting the finger leaves it open
    CHECK(f.chosen == 0);
}

// endregion

// region Select

TEST_CASE("A select opens its options under it and changes by the keyboard", "[popup][select]") {
    Popups f{};
    auto &select = f.window->appContainer().add<Select>(std::vector<std::string>{"Low", "Medium", "High"}, 1);
    select.width(160);
    std::vector<int> changes{};
    select.onChange([&](int index) { changes.push_back(index); });
    f.layout();
    CHECK(select.value() == "Medium");

    f.click(select);
    const Menu *list = select.menu();
    REQUIRE(list != nullptr);
    CHECK(hasClass(select, "open"));
    CHECK(hasClass(*list, "listbox"));
    CHECK(list->highlighted() == 1);
    CHECK(list->getWidth() >= select.getWidth());
    CHECK(list->rows()[1]->checked());
    CHECK(list->rows()[1]->checkTrailing());

    f.key(Key::DOWN);
    f.key(Key::ENTER);
    CHECK_FALSE(select.isOpen());
    CHECK_FALSE(hasClass(select, "open"));
    CHECK(select.selectedIndex() == 2);
    CHECK(changes == std::vector<int>{2});
    CHECK(f.window->focusedDiv() == &select);

    // From the keyboard, then Escape changes nothing
    f.key(Key::UP);
    REQUIRE(select.isOpen());
    f.key(Key::UP);
    f.key(Key::ESCAPE);
    CHECK_FALSE(select.isOpen());
    CHECK(select.selectedIndex() == 2);

    // Pressing the field again closes it
    f.click(select);
    REQUIRE(select.isOpen());
    f.click(select);
    CHECK_FALSE(select.isOpen());
    CHECK(changes.size() == 1);
}

TEST_CASE("A select shows its placeholder until something is chosen", "[popup][select]") {
    Popups f{};
    auto &select = f.window->appContainer().add<Select>(std::vector<std::string>{"A", "B"});
    select.placeholder("Pick one");
    f.layout();
    CHECK(select.selectedIndex() == -1);
    CHECK(select.value().empty());
    select.selectedIndex(0);
    CHECK(select.value() == "A");
}

// endregion

// region Tooltips

TEST_CASE("A tooltip shows after the mouse rests, at once for the next one", "[popup][tooltip]") {
    Popups f{};
    f.first.tooltip("First");
    f.second.tooltip("Second");
    f.layout();

    f.hover(f.first);
    f.window->updatePopups(1000ms);
    CHECK(f.window->tooltip() == nullptr);
    f.window->updatePopups(1000ms + Window::tooltipDelay - 1ms);
    CHECK(f.window->tooltip() == nullptr);
    f.window->updatePopups(1000ms + Window::tooltipDelay);
    REQUIRE(f.window->tooltip() != nullptr);
    CHECK(f.window->tooltip()->text() == "First");
    f.layout();
    // Above its anchor, unless there is no room: the buttons are at the top, so below
    CHECK(f.window->tooltip()->placedSide() == Side::Bottom);

    // The next one, at once
    f.hover(f.second);
    CHECK(f.window->tooltip() == nullptr);
    f.window->updatePopups(1400ms);
    REQUIRE(f.window->tooltip() != nullptr);
    CHECK(f.window->tooltip()->text() == "Second");

    // A press hides it, and it doesn't come back until the mouse moves to another
    f.click(f.second);
    CHECK(f.window->tooltip() == nullptr);
    f.window->updatePopups(5000ms);
    CHECK(f.window->tooltip() == nullptr);

    // Away, then long enough for the warmth to pass
    f.moveTo(400, 500);
    f.window->updatePopups(6000ms);
    f.window->updatePopups(6000ms + Window::tooltipWarmTime);
    f.hover(f.first);
    f.window->updatePopups(7000ms);
    CHECK(f.window->tooltip() == nullptr);
}

TEST_CASE("A tooltip sits above its anchor when there is room", "[popup][tooltip]") {
    Popups f{};
    f.bar.marginTop(200);
    f.first.tooltip("Tip");
    f.layout();
    f.hover(f.first);
    f.window->updatePopups(0ms);
    f.window->updatePopups(Window::tooltipDelay);
    REQUIRE(f.window->tooltip() != nullptr);
    f.layout();
    const Tooltip &tooltip = *f.window->tooltip();
    CHECK(tooltip.placedSide() == Side::Top);
    CHECK(Popups::origin(tooltip).second + tooltip.getHeight() == Popups::origin(f.first).second - 6);
    // It takes no mouse
    CHECK(f.window->overlay().popupAt(Popups::center(tooltip).first, Popups::center(tooltip).second) == nullptr);
}

TEST_CASE("A touch long press shows the tooltip when it opens nothing", "[popup][tooltip][touch]") {
    Popups f{};
    f.first.tooltip("Tip");
    f.layout();
    const auto [x, y] = Popups::center(f.first);
    f.window->handleTouchPress(static_cast<float>(x), static_cast<float>(y), 0ms);
    f.window->updateGestures(Window::longPressDelay);
    REQUIRE(f.window->tooltip() != nullptr);
    CHECK(f.window->tooltip()->text() == "Tip");
    f.window->handleTouchRelease(static_cast<float>(x), static_cast<float>(y), Window::longPressDelay + 100ms);
    CHECK(f.window->tooltip() == nullptr);
    CHECK(f.firstClicks == 0); // A long press never clicks
}

// endregion

// region With the lists and inputs

TEST_CASE("In a dialog, Tab hands the focus to the text fields' text and stays inside", "[popup][dialog][focus][inputs]") {
    Popups f{};
    f.key(Key::TAB);
    REQUIRE(f.window->focusedDiv() == &f.first);

    auto &dialog  = f.window->overlay().open<Dialog>("Find");
    auto &search  = dialog.body().add<SearchField>();
    auto &stepper = dialog.body().add<NumberStepper>(4);
    auto &cancel  = dialog.action("Cancel");
    f.layout();
    CHECK(f.window->focusedDiv() == search.focusDelegate()); // Its first field

    std::vector<Div *> visited{};
    for (int i = 0; i < 4; ++i) {
        f.key(Key::TAB);
        visited.push_back(f.window->focusedDiv());
    }
    // The stepper's text (delegated), Cancel, the close button, the search field's text again
    CHECK(visited[0] == stepper.focusDelegate());
    CHECK(visited[1] == &cancel);
    CHECK(visited[3] == search.focusDelegate());

    f.key(Key::ESCAPE);
    CHECK(f.popupCount() == 0);
    CHECK(f.window->focusedDiv() == &f.first);
}

TEST_CASE("In a dialog, Escape goes to a field that has a use for it before closing", "[popup][dialog][inputs]") {
    Popups f{};
    auto &dialog  = f.window->overlay().open<Dialog>("Find");
    auto &search  = dialog.body().add<SearchField>();
    auto &stepper = dialog.body().add<NumberStepper>(4);
    dialog.action("Cancel");
    f.layout();
    REQUIRE(f.window->focusedDiv() == search.focusDelegate());

    // With text, Escape clears the search field; empty, it closes the dialog
    search.text("cue");
    f.key(Key::ESCAPE);
    CHECK(search.text().empty());
    CHECK(f.popupCount() == 1);

    // A number being typed reverts; as typed, Escape closes
    f.key(Key::TAB);
    REQUIRE(f.window->focusedDiv() == stepper.focusDelegate());
    CHECK_FALSE(stepper.takesEscape());
    f.window->handleType(icu::UnicodeString("2"));
    f.layout();
    CHECK(stepper.takesEscape());
    f.key(Key::ESCAPE);
    CHECK(f.popupCount() == 1);
    CHECK(stepper.value() == 4);
    CHECK_FALSE(stepper.takesEscape());
    f.key(Key::ESCAPE);
    CHECK(f.popupCount() == 0);
}

TEST_CASE("A select in a dialog closes first, and gives the focus back to itself", "[popup][dialog][select][focus]") {
    Popups f{};
    auto &dialog = f.window->overlay().open<Dialog>("Curve");
    auto &select = dialog.body().add<Select>(std::vector<std::string>{"Linear", "Snap"}, 0);
    f.layout();
    f.click(select);
    REQUIRE(select.isOpen());
    f.key(Key::ESCAPE);
    CHECK(!select.isOpen());
    CHECK(f.popupCount() == 1); // The dialog stays
    CHECK(select.hasFocus());
}

TEST_CASE("A list scrolling in a dialog ends with its padding after the last row", "[popup][dialog][lists]") {
    Popups f{};
    auto &dialog = f.window->overlay().open<Dialog>("People");
    auto &list   = dialog.body().add<List>();
    list.height(120.0f).padding(8.0f).overflow(Overflow::Scroll);
    const ListRow *last = nullptr;
    for (int i = 0; i < 12; ++i) {
        last = &list.row("Row " + std::to_string(i));
    }
    f.layout();
    REQUIRE(list.contentHeight() > list.getHeight());
    list.scrollBy(0, -100000);
    f.layout();
    // Scrolled to the end, the last row's bottom sits the padding above the list's
    CHECK(last->y() + last->getHeight() + list.scrollY() == list.getHeight() - 8);
}

TEST_CASE("Tooltips show on the new controls", "[popup][tooltip][inputs]") {
    Popups f{};
    auto &toggle = f.window->appContainer().add<Switch>("Blackout");
    toggle.tooltip("Blackout on start");
    f.layout();
    f.hover(toggle);
    f.window->updatePopups(1000ms);
    f.window->updatePopups(1000ms + Window::tooltipDelay);
    REQUIRE(f.window->tooltip() != nullptr);
    CHECK(f.window->tooltip()->text() == "Blackout on start");
}

// endregion

// region Dialogs

TEST_CASE("A dialog traps the focus and gives it back", "[popup][dialog][focus]") {
    Popups f{};
    f.key(Key::TAB);
    REQUIRE(f.window->focusedDiv() == &f.first);

    int  renamed = 0;
    auto &dialog = f.window->overlay().open<Dialog>("Rename");
    dialog.description("Something");
    auto &cancel = dialog.action("Cancel");
    auto &ok     = dialog.action("Rename", [&]() { ++renamed; }, "primary");
    CHECK(f.window->focusedDiv() == &dialog.panel()); // Until it is laid out
    f.layout();
    CHECK(dialog.modal());
    CHECK(f.window->focusedDiv() == &cancel); // No field: the first button, ringed since Tab opened it
    CHECK(f.window->focusVisible());

    // Tab goes round the actions and the close button, never out
    std::vector<Div *> visited{};
    for (int i = 0; i < 3; ++i) {
        f.key(Key::TAB);
        visited.push_back(f.window->focusedDiv());
    }
    CHECK(visited[0] == &ok);
    CHECK(visited[2] == &cancel); // After the close button
    f.key(Key::TAB, Mod{.shift = true});
    CHECK(f.window->focusedDiv() == visited[1]); // The close button
    f.key(Key::TAB, Mod{.shift = true});
    CHECK(f.window->focusedDiv() == &ok);

    f.key(Key::ENTER);
    CHECK(renamed == 1);
    CHECK(f.popupCount() == 0);
    CHECK(f.window->focusedDiv() == &f.first);
}

TEST_CASE("A dialog focuses its first field, else its first button, else its panel, or what it is told", "[popup][dialog][focus]") {
    Popups f{};

    SECTION("a field, after buttons in the body") {
        auto &dialog = f.window->overlay().open<Dialog>("Rename");
        auto &help   = dialog.body().add<Button>("Help");
        auto &name   = dialog.body().add<TextInput>();
        dialog.action("Cancel");
        f.layout();
        CHECK(dialog.initialFocus() == &name);
        CHECK(f.window->focusedDiv() == &name); // The field itself
        CHECK_FALSE(help.hasFocus());
        f.layout();
        CHECK(name.computedStyle().borderColor() == themes::pivotalDark.palette.accent);
    }

    SECTION("a button in the body, before the footer's") {
        auto &dialog = f.window->overlay().open<Dialog>("Pick");
        auto &help   = dialog.body().add<Button>("Help");
        dialog.action("Cancel");
        f.layout();
        CHECK(f.window->focusedDiv() == &help);
        CHECK_FALSE(f.window->focusVisible()); // Not opened from the keyboard: no ring
    }

    SECTION("nothing but the close button: the panel keeps it") {
        auto &dialog = f.window->overlay().open<Dialog>("Note").description("Read me");
        f.layout();
        CHECK(dialog.initialFocus() == nullptr);
        CHECK(f.window->focusedDiv() == &dialog.panel());
    }

    SECTION("initialFocus picks another") {
        auto &dialog = f.window->overlay().open<Dialog>("Delete");
        dialog.body().add<TextInput>();
        dialog.action("Cancel");
        auto &remove = dialog.action("Delete", nullptr, "danger");
        dialog.initialFocus(remove);
        f.layout();
        CHECK(f.window->focusedDiv() == &remove);
    }

    SECTION("only once: a focus moved after it stays") {
        auto &dialog = f.window->overlay().open<Dialog>("Twice");
        auto &name   = dialog.body().add<TextInput>();
        auto &cancel = dialog.action("Cancel");
        f.layout();
        REQUIRE(name.hasFocus());
        f.window->requestFocus(&cancel, true);
        f.layout();
        CHECK(cancel.hasFocus());
    }
}

TEST_CASE("A dialog closes by Escape and the backdrop, unless it isn't dismissible", "[popup][dialog][dismiss]") {
    Popups f{};
    auto &dialog = f.window->overlay().open<Dialog>("One");
    f.layout();
    // A press on the panel doesn't close it
    f.click(dialog.panel());
    CHECK(f.popupCount() == 1);
    f.clickAt(5, 5); // The backdrop
    CHECK(f.popupCount() == 0);
    CHECK(f.firstClicks == 0);

    f.window->overlay().open<Dialog>("Two").dismissible(false);
    f.layout();
    f.clickAt(5, 5);
    CHECK(f.popupCount() == 1);
    f.key(Key::ESCAPE);
    CHECK(f.popupCount() == 0);
}

TEST_CASE("A menu over a dialog closes first", "[popup][dialog][menu]") {
    Popups f{};
    auto &dialog = f.window->overlay().open<Dialog>("Dialog");
    auto &button = dialog.action("Menu", nullptr, "secondary", false);
    f.layout();
    f.window->openMenu({Menu::item("One", []() {})}, button);
    f.layout();
    REQUIRE(f.popupCount() == 2);
    f.key(Key::ESCAPE);
    CHECK(f.popupCount() == 1);
    CHECK(f.top<Dialog>() == &dialog);
}

// endregion

// region Toasts

TEST_CASE("Toasts time out, held while the mouse is over them", "[popup][toast]") {
    Popups f{};
    const ToastOptions options{.title = "Saved", .timeout = 5s};
    const Toast &toast = f.window->toast(options);
    f.layout();
    const ToastStack &stack = f.window->toasts();
    CHECK(hasClass(toast, "info"));
    // In the bottom right corner, inside the window's 1 px border
    const auto [x, y] = Popups::origin(toast);
    CHECK(x + toast.getWidth() == 800 - 1 - 16);
    CHECK(y + toast.getHeight() == 600 - 1 - 16);

    f.window->updatePopups(10s); // Shown from this frame
    f.window->updatePopups(12s);
    CHECK(toast.elapsed() == 2s);

    // Held while the mouse is over the stack
    f.hover(toast);
    f.window->updatePopups(20s);
    CHECK(toast.elapsed() == 2s);
    // Frames stop while nothing changes: the mouse leaving brings the next one, and the time until it didn't count
    f.window->updatePopups(40s);
    CHECK(toast.elapsed() == 2s);
    f.moveTo(10, 300);
    f.window->updatePopups(40s);
    CHECK(toast.elapsed() == 2s);
    f.window->updatePopups(42s);
    CHECK(toast.elapsed() == 4s);
    CHECK(stack.toasts().size() == 1);
    f.window->updatePopups(43s);
    CHECK(stack.toasts().empty());
}

TEST_CASE("A toast's actions and close button dismiss it; the stack keeps a few", "[popup][toast]") {
    Popups f{};
    int  retried = 0;
    auto &toast  = f.window->toast({.title = "Offline", .variant = ToastVariant::Warning, .timeout = 0s,
                                    .actions = {{"Retry", [&]() { ++retried; }, true}}});
    f.layout();
    CHECK(hasClass(toast, "warning"));
    f.window->updatePopups(0s);
    f.window->updatePopups(1000s);
    CHECK(f.window->toasts().toasts().size() == 1); // No timeout

    Button *retry = nullptr;
    std::function<void(Div &)> find = [&](Div &div) {
        for (const auto &child: div.children()) {
            if (auto *button = dynamic_cast<Button *>(child.get()); button && button->label() == "Retry") {
                retry = button;
            }
            find(*child);
        }
    };
    find(toast);
    REQUIRE(retry != nullptr);
    f.click(*retry);
    CHECK(retried == 1);
    CHECK(f.window->toasts().toasts().empty());

    for (int i = 0; i < 5; ++i) {
        f.window->toast({.title = "Toast " + std::to_string(i)});
    }
    f.layout();
    auto toasts = f.window->toasts().toasts();
    REQUIRE(toasts.size() == ToastStack::limit);
    CHECK(toasts.front()->options().title == "Toast 2");
    CHECK(toasts.back()->options().title == "Toast 4");
    // Toasts don't take the focus from the page, nor Escape
    CHECK_FALSE(f.window->overlay().dismissTop());
}

// endregion
