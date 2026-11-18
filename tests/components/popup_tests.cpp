#include <algorithm>
#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <unicode/unistr.h>
#include "include/core/SkRect.h"
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
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Overlay.hpp>
#include <psychic-ui/Popup.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>
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
        auto placement = placePopup(SkRect::MakeXYWH(100, 540, 80, 20), 200, 100, bounds, Side::Bottom, PopupAlign::Start, 6);
        CHECK(placement.side == Side::Top);
        CHECK(placement.y == 434); // 540 - 6 - 100
    }

    SECTION("above, at the top: below") {
        auto placement = placePopup(SkRect::MakeXYWH(100, 20, 80, 20), 200, 30, bounds, Side::Top, PopupAlign::Start, 6);
        CHECK(placement.side == Side::Bottom);
        CHECK(placement.y == 46);
    }

    SECTION("right, at the right edge: left (a submenu)") {
        auto placement = placePopup(SkRect::MakeXYWH(600, 100, 180, 32), 200, 100, bounds, Side::Right, PopupAlign::Start, 4, -5);
        CHECK(placement.side == Side::Left);
        CHECK(placement.x == 396); // 600 - 4 - 200
        CHECK(placement.y == 95);
    }

    SECTION("neither side fits: the one with more room, shifted inside") {
        auto placement = placePopup(SkRect::MakeXYWH(100, 200, 80, 20), 200, 500, bounds, Side::Bottom, PopupAlign::Start, 6);
        CHECK(placement.side == Side::Bottom); // 362 below, 182 above
        CHECK(placement.y == 88);               // 588 - 500
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

        [[nodiscard]] std::size_t popupCount() const { return window->overlay().children().size(); }

        template<class T>
        [[nodiscard]] T *top() const { return window->overlay().top<T>(); }
    };

    bool hasClass(const Div &div, const std::string &name) {
        return div.classNames().contains(name);
    }
}

// region Overlay

TEST_CASE("A menu opens under its anchor, and above it at the window's bottom", "[popup][menu]") {
    Popups fixture{};
    const Menu &menu = fixture.window->openMenu({Menu::item("One", []() {}), Menu::item("Two", []() {})}, fixture.first);
    fixture.layout();
    const auto [areaX, areaY] = Popups::origin(fixture.first);
    const auto [menuX, menuY] = Popups::origin(menu);
    CHECK(menuX == areaX);
    CHECK(menuY == areaY + fixture.first.getHeight() + 6);
    CHECK(menu.placedSide() == Side::Bottom);

    // An anchor at the bottom of the window
    fixture.bar.marginTop(560);
    fixture.layout();
    fixture.layout();
    CHECK(menu.placedSide() == Side::Top);
    const auto [firstX, firstY] = Popups::origin(fixture.first);
    CHECK(Popups::origin(menu).second == firstY - 6 - menu.getHeight());
}

TEST_CASE("A popup whose anchor leaves the window closes", "[popup]") {
    Popups fixture{};
    fixture.window->openMenu({Menu::item("One", []() {})}, fixture.second);
    fixture.layout();
    REQUIRE(fixture.popupCount() == 1);
    fixture.bar.remove(fixture.second);
    fixture.layout();
    CHECK(fixture.popupCount() == 0);
}

TEST_CASE("A press outside a menu closes it and goes no further", "[popup][menu][dismiss]") {
    Popups fixture{};
    int closed = 0;
    fixture.window->onMenuClose([&]() { ++closed; });
    fixture.window->openMenu({Menu::item("One", []() {})}, fixture.first);
    fixture.layout();

    // On the second button, under the modal menu's reach
    fixture.click(fixture.second);
    CHECK(fixture.popupCount() == 0);
    CHECK(closed == 1);
    CHECK(fixture.secondClicks == 0); // Swallowed with its release

    // The next press goes through
    fixture.click(fixture.second);
    CHECK(fixture.secondClicks == 1);
}

TEST_CASE("A press inside a menu doesn't close it, Escape does", "[popup][menu][dismiss]") {
    Popups fixture{};
    const Menu &menu = fixture.window->openMenu({Menu::check("Check", false)}, fixture.first);
    fixture.layout();
    fixture.click(*menu.rows()[0]); // A check item keeps the menu open
    CHECK(fixture.popupCount() == 1);
    fixture.key(Key::ESCAPE);
    CHECK(fixture.popupCount() == 0);
}

TEST_CASE("A modal menu takes the mouse from the content, except its pass-through divs", "[popup][menu]") {
    Popups fixture{};
    Menu &menu = fixture.window->openMenu({Menu::item("One", []() {})}, fixture.first);
    menu.passThrough(fixture.bar);
    fixture.layout();
    CHECK_FALSE(fixture.window->isDragArea(5, 5));
    fixture.hover(fixture.second);
    CHECK(fixture.second.mouseOver());
    fixture.click(fixture.second); // Not outside: it reaches the button
    CHECK(fixture.secondClicks == 1);
}

TEST_CASE("Non-modal popups let the mouse through around them", "[popup]") {
    Popups fixture{};
    auto &tooltip = fixture.window->overlay().open<Tooltip>("Tip");
    tooltip.anchor(fixture.second);
    fixture.layout();
    REQUIRE(fixture.popupCount() == 1);
    // A press anywhere closes a tooltip, and goes on
    fixture.click(fixture.first);
    CHECK(fixture.firstClicks == 1);
    CHECK(fixture.popupCount() == 0);
}

// endregion

// region Menus

TEST_CASE("The keyboard moves along a menu's rows and chooses one", "[popup][menu][keyboard]") {
    Popups fixture{};
    int  chosen = 0;
    auto disabled = Menu::item("Disabled", []() {});
    disabled->enabled = false;
    Menu &menu = fixture.window->openMenu(
        {
            Menu::group("Group"),
            Menu::item("One", [&]() { chosen = 1; }),
            disabled,
            Menu::separator(),
            Menu::item("Three", [&]() { chosen = 3; }),
        }, fixture.first
    );
    fixture.layout();
    REQUIRE(menu.rows().size() == 3); // Not the group label or the separator
    CHECK(fixture.window->focusedDiv() == &menu);
    CHECK(menu.highlighted() == -1); // Opened with the mouse

    fixture.key(Key::DOWN);
    CHECK(menu.highlighted() == 0);
    CHECK(menu.rows()[0]->selected());
    fixture.key(Key::DOWN); // Over the disabled one
    CHECK(menu.highlighted() == 2);
    fixture.key(Key::DOWN); // Wraps
    CHECK(menu.highlighted() == 0);
    fixture.key(Key::END);
    CHECK(menu.highlighted() == 2);
    fixture.key(Key::ENTER);
    CHECK(chosen == 3);
    CHECK(fixture.popupCount() == 0);
}

TEST_CASE("A menu opened from the keyboard highlights its first row; closing gives the focus back", "[popup][menu][focus]") {
    Popups fixture{};
    fixture.first.onClick([&]() { fixture.window->openMenu({Menu::item("One", []() {}), Menu::item("Two", []() {})}, fixture.first); });
    fixture.key(Key::TAB); // The first button, from the keyboard
    REQUIRE(fixture.window->focusedDiv() == &fixture.first);
    fixture.key(Key::ENTER);
    auto *menu = fixture.top<Menu>();
    REQUIRE(menu != nullptr);
    CHECK(menu->highlighted() == 0);
    fixture.key(Key::ESCAPE);
    CHECK(fixture.popupCount() == 0);
    CHECK(fixture.window->focusedDiv() == &fixture.first);
    CHECK(fixture.window->focusVisible());
}

TEST_CASE("Submenus open by hover, Right and Enter, and close by Left and Escape", "[popup][menu][keyboard]") {
    Popups fixture{};
    int  chosen = 0;
    Menu &menu = fixture.window->openMenu(
        {
            Menu::item("One", []() {}),
            Menu::item("More", {Menu::item("Deep", [&]() { chosen = 1; }), Menu::item("Deeper", []() {})}),
        }, fixture.first
    );
    fixture.layout();

    SECTION("hover") {
        fixture.hover(*menu.rows()[1]);
        const Menu *sub = menu.submenu();
        REQUIRE(sub != nullptr);
        CHECK(sub->owner() == &menu);
        CHECK(fixture.window->focusedDiv() == &menu); // Hovering leaves the focus
        CHECK(sub->placedSide() == Side::Right);
        // Level with its row: its first row's top on the row's top
        fixture.layout();
        CHECK(Popups::origin(*sub->rows()[0]).second == Popups::origin(*menu.rows()[1]).second);
        // Hovering another row closes it
        fixture.hover(*menu.rows()[0]);
        CHECK(menu.submenu() == nullptr);
        CHECK(fixture.popupCount() == 1);
    }

    SECTION("keyboard") {
        fixture.key(Key::DOWN);
        fixture.key(Key::DOWN);
        fixture.key(Key::RIGHT);
        const Menu *sub = menu.submenu();
        REQUIRE(sub != nullptr);
        CHECK(fixture.window->focusedDiv() == sub);
        CHECK(sub->highlighted() == 0);
        fixture.key(Key::LEFT);
        CHECK(menu.submenu() == nullptr);
        CHECK(fixture.window->focusedDiv() == &menu);
        CHECK(fixture.popupCount() == 1);

        fixture.key(Key::ENTER); // On "More" still
        sub = menu.submenu();
        REQUIRE(sub != nullptr);
        fixture.key(Key::ESCAPE); // Only the submenu
        CHECK(fixture.popupCount() == 1);
        CHECK(fixture.window->focusedDiv() == &menu);

        fixture.key(Key::RIGHT);
        fixture.key(Key::ENTER); // "Deep"
        CHECK(chosen == 1);
        CHECK(fixture.popupCount() == 0);
    }

    SECTION("a press outside closes the whole chain") {
        fixture.hover(*menu.rows()[1]);
        REQUIRE(fixture.popupCount() == 2);
        fixture.clickAt(790, 590);
        CHECK(fixture.popupCount() == 0);
    }
}

TEST_CASE("A submenu flips to the left at the window's right edge", "[popup][menu]") {
    Popups fixture{};
    fixture.bar.justify(Justify::End);
    fixture.layout();
    const Menu &menu = fixture.window->openMenu({Menu::item("More", {Menu::item("Deep", []() {})})}, fixture.second, Side::Bottom, PopupAlign::End);
    fixture.layout();
    fixture.hover(*menu.rows()[0]);
    const Menu *sub = menu.submenu();
    REQUIRE(sub != nullptr);
    fixture.layout();
    CHECK(sub->placedSide() == Side::Left);
    CHECK(Popups::origin(*sub).first + sub->getWidth() == Popups::origin(*menu.rows()[0]).first - 4);
}

TEST_CASE("Check and radio items keep their state and the menu open", "[popup][menu]") {
    Popups fixture{};
    std::vector<bool> toggles{};
    std::string       picked{};
    auto group = std::make_shared<MenuRadioGroup>("a", [&](const std::string &value) { picked = value; });
    const MenuItems items{
        Menu::check("Check", false, [&](bool checked) { toggles.push_back(checked); }),
        Menu::radio(group, "A", "a"),
        Menu::radio(group, "B", "b"),
        Menu::item("Plain", []() {}),
    };
    const Menu &menu = fixture.window->openMenu(items, fixture.first);
    fixture.layout();
    CHECK(menu.rows()[1]->checked());
    CHECK(hasClass(*menu.rows()[1], "checked"));

    fixture.click(*menu.rows()[0]);
    fixture.click(*menu.rows()[2]);
    CHECK(fixture.popupCount() == 1);
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
    fixture.key(Key::ESCAPE);
    const Menu &again = fixture.window->openMenu(items, fixture.first);
    fixture.layout();
    CHECK(again.rows()[0]->checked());
    CHECK(again.rows()[2]->checked());
}

TEST_CASE("Menu bars switch menus with Left and Right", "[popup][menu][keyboard]") {
    Popups fixture{};
    auto &bar = fixture.window->appContainer().add<MenuBar>(
        MenuItems{
            Menu::item("File", {Menu::item("New", []() {})}),
            Menu::item("Edit", {Menu::item("Copy", []() {})}),
        }
    );
    fixture.layout();
    auto *file = dynamic_cast<Button *>(bar.children()[0].get()); // The bar's first trigger
    REQUIRE(file);
    fixture.click(*file);
    REQUIRE(fixture.top<Menu>() != nullptr);
    CHECK(fixture.top<Menu>()->rows()[0]->menuItem()->label == "New");
    fixture.key(Key::RIGHT);
    REQUIRE(fixture.top<Menu>() != nullptr);
    CHECK(fixture.top<Menu>()->rows()[0]->menuItem()->label == "Copy");
    fixture.key(Key::RIGHT); // Wraps
    CHECK(fixture.top<Menu>()->rows()[0]->menuItem()->label == "New");
    CHECK(fixture.popupCount() == 1);
}

TEST_CASE("A context menu opens at a point, inside the window", "[popup][menu]") {
    Popups fixture{};
    const Menu &menu = fixture.window->openContextMenu({Menu::item("One", []() {}), Menu::item("Two", []() {})}, 790, 590);
    fixture.layout();
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

        [[nodiscard]] Menu *menu() const { return top<Menu>(); }
    };
}

TEST_CASE("A right press opens a div's context menu where it pressed", "[popup][menu][context]") {
    ContextMenus fixture{};
    const auto [areaX, areaY] = Popups::origin(fixture.area);
    const int x = areaX + 250;
    const int y = areaY + 150;
    fixture.press(x, y, MouseButton::RIGHT);
    REQUIRE(fixture.menu() != nullptr);
    CHECK(hasClass(*fixture.menu(), "context"));
    CHECK(fixture.menu()->rows().size() == 3);
    CHECK(fixture.menu()->highlighted() == -1); // From the mouse: nothing highlighted
    const auto [menuX, menuY] = Popups::origin(*fixture.menu());
    CHECK(menuX == x);
    CHECK(menuY == y);
    fixture.release(x, y, MouseButton::RIGHT);
    CHECK(fixture.menu() != nullptr); // The release doesn't choose or close

    // A left press does nothing of the kind, and no right press outside opens one
    fixture.key(Key::ESCAPE);
    REQUIRE(fixture.menu() == nullptr);
    fixture.clickAt(x, y);
    CHECK(fixture.menu() == nullptr);
    const auto [firstX, firstY] = Popups::center(fixture.first);
    fixture.press(firstX, firstY, MouseButton::RIGHT);
    CHECK(fixture.menu() == nullptr);
}

TEST_CASE("The deepest div with a context menu opens its own", "[popup][menu][context]") {
    ContextMenus fixture{};
    fixture.inner.contextMenu({Menu::item("Rename", []() {})});
    const auto [x, y] = Popups::center(fixture.inner);
    fixture.press(x, y, MouseButton::RIGHT);
    REQUIRE(fixture.menu() != nullptr);
    REQUIRE(fixture.menu()->rows().size() == 1);
    CHECK(fixture.menu()->rows()[0]->menuItem()->label == "Rename");
}

TEST_CASE("A right press elsewhere while a context menu is open opens it there", "[popup][menu][context]") {
    ContextMenus fixture{};
    fixture.inner.contextMenu({Menu::item("Rename", []() {})});
    const auto [areaX, areaY] = Popups::origin(fixture.area);
    fixture.press(areaX + 250, areaY + 150, MouseButton::RIGHT);
    fixture.release(areaX + 250, areaY + 150, MouseButton::RIGHT);
    REQUIRE(fixture.menu() != nullptr);

    // Elsewhere in the same div: closed, and open again at the new point
    fixture.press(areaX + 150, areaY + 170, MouseButton::RIGHT);
    REQUIRE(fixture.menu() != nullptr);
    CHECK(fixture.window->overlay().popups().size() == 1);
    const auto [menuX, menuY] = Popups::origin(*fixture.menu());
    CHECK(menuX == areaX + 150);
    CHECK(menuY == areaY + 170);
    fixture.release(areaX + 150, areaY + 170, MouseButton::RIGHT);
    CHECK(fixture.menu() != nullptr); // Its release doesn't close it

    // On another div with a menu of its own: that one opens
    const auto [innerX, innerY] = Popups::center(fixture.inner);
    fixture.press(innerX, innerY, MouseButton::RIGHT);
    REQUIRE(fixture.menu() != nullptr);
    REQUIRE(fixture.menu()->rows().size() == 1);
    CHECK(fixture.menu()->rows()[0]->menuItem()->label == "Rename");
    fixture.release(innerX, innerY, MouseButton::RIGHT);

    // Where nothing has one: only closed, and the press reaches what is there
    const auto [firstX, firstY] = Popups::center(fixture.first);
    int pressed = 0;
    fixture.first.onMousePress([&]() { ++pressed; });
    fixture.press(firstX, firstY, MouseButton::RIGHT);
    CHECK(fixture.menu() == nullptr);
    CHECK(pressed == 1);
    fixture.release(firstX, firstY, MouseButton::RIGHT);

    // A left press outside still only closes it
    fixture.press(areaX + 250, areaY + 150, MouseButton::RIGHT);
    REQUIRE(fixture.menu() != nullptr);
    fixture.clickAt(firstX, firstY);
    CHECK(fixture.menu() == nullptr);
    CHECK(fixture.firstClicks == 0);
}

TEST_CASE("On the right button's press by default, on its release as on Windows", "[popup][menu][context]") {
    #ifdef _WIN32
    CHECK(std::make_shared<Window>("default")->contextMenuOnRelease());
    #else
    CHECK_FALSE(std::make_shared<Window>("default")->contextMenuOnRelease());
    #endif
    ContextMenus fixture{};
    fixture.window->contextMenuOnRelease(true);
    const auto [areaX, areaY] = Popups::origin(fixture.area);
    int requests = 0;
    int released = 0;
    fixture.area.onMouseRelease([&]() { ++released; });
    fixture.area.onContextMenuRequest([&]() {
        ++requests;
        if (requests == 1) {
            CHECK(released == 1); // After the release's signals
        }
    });
    fixture.press(areaX + 250, areaY + 150, MouseButton::RIGHT);
    CHECK(fixture.menu() == nullptr);
    fixture.release(areaX + 260, areaY + 160, MouseButton::RIGHT);
    REQUIRE(fixture.menu() != nullptr);
    CHECK(requests == 1);
    const auto [menuX, menuY] = Popups::origin(*fixture.menu());
    CHECK(menuX == areaX + 260); // Where it was released
    CHECK(menuY == areaY + 160);

    // Elsewhere: the press closes it, the release opens it again there
    fixture.press(areaX + 150, areaY + 170, MouseButton::RIGHT);
    CHECK(fixture.menu() == nullptr);
    fixture.release(areaX + 150, areaY + 170, MouseButton::RIGHT);
    REQUIRE(fixture.menu() != nullptr);
    CHECK(requests == 2);
    fixture.key(Key::ESCAPE);

    // A touch long press still opens it while held, the keyboard on its key
    fixture.window->handleTouchPress(static_cast<float>(areaX + 250), static_cast<float>(areaY + 150), 0ms);
    fixture.window->updateGestures(Window::longPressDelay);
    fixture.layout();
    CHECK(fixture.menu() != nullptr);
    fixture.window->handleTouchRelease(static_cast<float>(areaX + 250), static_cast<float>(areaY + 150), Window::longPressDelay + 100ms);
    fixture.layout();
    CHECK(fixture.menu() != nullptr);
    CHECK(requests == 3);
    fixture.key(Key::ESCAPE);
    fixture.window->requestFocus(&fixture.area, true);
    fixture.key(Key::MENU);
    CHECK(fixture.menu() != nullptr);
    CHECK(requests == 4);
}

TEST_CASE("Control and the left button is the right one on macOS", "[popup][menu][context]") {
    ContextMenus fixture{};
    #ifdef __APPLE__
    CHECK(f.window->ctrlClickIsRightClick());
    #else
    CHECK_FALSE(fixture.window->ctrlClickIsRightClick());
    #endif
    const auto [areaX, areaY]   = Popups::origin(fixture.area);
    const auto [firstX, firstY] = Popups::center(fixture.first);
    std::vector<MouseButton> buttons{};
    fixture.first.onMouseButtonChange([&](const MouseEvent &event) { buttons.push_back(event.button); });
    const auto ctrlClick = [&](int x, int y, Mod mod) {
        fixture.window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true, .modifiers = mod});
        fixture.layout();
        // Control may go up first: the release is still the right button's
        fixture.window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false});
        fixture.layout();
    };

    // Off: a left click with Control
    fixture.window->ctrlClickIsRightClick(false);
    ctrlClick(firstX, firstY, Mod{.ctrl = true});
    CHECK(fixture.firstClicks == 1);
    CHECK(buttons == std::vector{MouseButton::LEFT, MouseButton::LEFT});

    // On: a right press and release, no click, the context menu
    fixture.window->ctrlClickIsRightClick(true);
    buttons.clear();
    ctrlClick(firstX, firstY, Mod{.ctrl = true});
    CHECK(fixture.firstClicks == 1);
    CHECK(buttons == std::vector{MouseButton::RIGHT, MouseButton::RIGHT});
    ctrlClick(areaX + 250, areaY + 150, Mod{.ctrl = true});
    REQUIRE(fixture.menu() != nullptr);
    const auto [menuX, menuY] = Popups::origin(*fixture.menu());
    CHECK(menuX == areaX + 250);
    CHECK(menuY == areaY + 150);
    fixture.key(Key::ESCAPE);

    // With another modifier, it stays the left button
    buttons.clear();
    ctrlClick(firstX, firstY, Mod{.shift = true, .ctrl = true});
    CHECK(fixture.firstClicks == 2);
    CHECK(buttons == std::vector{MouseButton::LEFT, MouseButton::LEFT});
    CHECK(fixture.menu() == nullptr);
}

TEST_CASE("An empty context menu removes it", "[popup][menu][context]") {
    ContextMenus fixture{};
    fixture.area.contextMenu({});
    CHECK(!fixture.area.contextMenuRequested.hasSubscriptions());
    const auto [x, y] = Popups::center(fixture.area);
    fixture.press(x + 100, y, MouseButton::RIGHT);
    CHECK(fixture.menu() == nullptr);
}

TEST_CASE("The Menu key and Shift+F10 open the focused div's context menu at its first item", "[popup][menu][context][keyboard]") {
    ContextMenus fixture{};
    fixture.window->requestFocus(&fixture.area, false); // Even after a click: the keyboard asked for it
    fixture.key(Key::MENU);
    REQUIRE(fixture.menu() != nullptr);
    CHECK(fixture.menu()->highlighted() == 1); // The first that can be chosen: Undo is disabled
    CHECK(fixture.menu()->hasFocus());
    CHECK(fixture.window->focusVisible());
    const auto [menuX, menuY]     = Popups::origin(*fixture.menu());
    const auto [centerX, centerY] = Popups::center(fixture.area);
    CHECK(menuX == centerX);
    CHECK(menuY == centerY);

    fixture.key(Key::ENTER);
    CHECK(fixture.chosen == 1);
    CHECK(fixture.menu() == nullptr);
    CHECK(fixture.area.hasFocus()); // The focus came back

    fixture.key(Key::F10, Mod{.shift = true});
    CHECK(fixture.menu() != nullptr);
}

TEST_CASE("A touch long press opens the context menu instead of the tooltip", "[popup][menu][context][touch]") {
    ContextMenus fixture{};
    fixture.area.tooltip("Tip");
    const auto [areaX, areaY] = Popups::origin(fixture.area);
    const auto x = static_cast<float>(areaX + 250);
    const auto y = static_cast<float>(areaY + 150);
    fixture.window->handleTouchPress(x, y, 0ms);
    fixture.window->updateGestures(Window::longPressDelay);
    fixture.layout();
    REQUIRE(fixture.menu() != nullptr);
    CHECK(fixture.window->tooltip() == nullptr);
    fixture.window->handleTouchRelease(x, y, Window::longPressDelay + 100ms);
    fixture.layout();
    CHECK(fixture.menu() != nullptr); // Lifting the finger leaves it open
    CHECK(fixture.chosen == 0);
}

// endregion

// region Select

TEST_CASE("A select opens its options under it and changes by the keyboard", "[popup][select]") {
    Popups fixture{};
    auto &select = fixture.window->appContainer().add<Select>(std::vector<std::string>{"Low", "Medium", "High"}, 1);
    select.width(160);
    std::vector<int> changes{};
    select.onChange([&](int index) { changes.push_back(index); });
    fixture.layout();
    CHECK(select.value() == "Medium");

    fixture.click(select);
    const Menu *list = select.menu();
    REQUIRE(list != nullptr);
    CHECK(hasClass(select, "open"));
    CHECK(hasClass(*list, "listbox"));
    CHECK(list->highlighted() == 1);
    CHECK(list->getWidth() >= select.getWidth());
    CHECK(list->rows()[1]->checked());
    CHECK(list->rows()[1]->checkTrailing());

    fixture.key(Key::DOWN);
    fixture.key(Key::ENTER);
    CHECK_FALSE(select.isOpen());
    CHECK_FALSE(hasClass(select, "open"));
    CHECK(select.selectedIndex() == 2);
    CHECK(changes == std::vector<int>{2});
    CHECK(fixture.window->focusedDiv() == &select);

    // From the keyboard, then Escape changes nothing
    fixture.key(Key::UP);
    REQUIRE(select.isOpen());
    fixture.key(Key::UP);
    fixture.key(Key::ESCAPE);
    CHECK_FALSE(select.isOpen());
    CHECK(select.selectedIndex() == 2);

    // Pressing the field again closes it
    fixture.click(select);
    REQUIRE(select.isOpen());
    fixture.click(select);
    CHECK_FALSE(select.isOpen());
    CHECK(changes.size() == 1);
}

TEST_CASE("A select shows its placeholder until something is chosen", "[popup][select]") {
    Popups fixture{};
    auto &select = fixture.window->appContainer().add<Select>(std::vector<std::string>{"A", "B"});
    select.placeholder("Pick one");
    fixture.layout();
    CHECK(select.selectedIndex() == -1);
    CHECK(select.value().empty());
    select.selectedIndex(0);
    CHECK(select.value() == "A");
}

// endregion

// region Tooltips

TEST_CASE("A tooltip shows after the mouse rests, at once for the next one", "[popup][tooltip]") {
    Popups fixture{};
    fixture.first.tooltip("First");
    fixture.second.tooltip("Second");
    fixture.layout();

    fixture.hover(fixture.first);
    fixture.window->updatePopups(1000ms);
    CHECK(fixture.window->tooltip() == nullptr);
    fixture.window->updatePopups(1000ms + Window::tooltipDelay - 1ms);
    CHECK(fixture.window->tooltip() == nullptr);
    fixture.window->updatePopups(1000ms + Window::tooltipDelay);
    REQUIRE(fixture.window->tooltip() != nullptr);
    CHECK(fixture.window->tooltip()->text() == "First");
    fixture.layout();
    // Above its anchor, unless there is no room: the buttons are at the top, so below
    CHECK(fixture.window->tooltip()->placedSide() == Side::Bottom);

    // The next one, at once
    fixture.hover(fixture.second);
    CHECK(fixture.window->tooltip() == nullptr);
    fixture.window->updatePopups(1400ms);
    REQUIRE(fixture.window->tooltip() != nullptr);
    CHECK(fixture.window->tooltip()->text() == "Second");

    // A press hides it, and it doesn't come back until the mouse moves to another
    fixture.click(fixture.second);
    CHECK(fixture.window->tooltip() == nullptr);
    fixture.window->updatePopups(5000ms);
    CHECK(fixture.window->tooltip() == nullptr);

    // Away, then long enough for the warmth to pass
    fixture.moveTo(400, 500);
    fixture.window->updatePopups(6000ms);
    fixture.window->updatePopups(6000ms + Window::tooltipWarmTime);
    fixture.hover(fixture.first);
    fixture.window->updatePopups(7000ms);
    CHECK(fixture.window->tooltip() == nullptr);
}

TEST_CASE("A tooltip sits above its anchor when there is room", "[popup][tooltip]") {
    Popups fixture{};
    fixture.bar.marginTop(200);
    fixture.first.tooltip("Tip");
    fixture.layout();
    fixture.hover(fixture.first);
    fixture.window->updatePopups(0ms);
    fixture.window->updatePopups(Window::tooltipDelay);
    REQUIRE(fixture.window->tooltip() != nullptr);
    fixture.layout();
    const Tooltip &tooltip = *fixture.window->tooltip();
    CHECK(tooltip.placedSide() == Side::Top);
    CHECK(Popups::origin(tooltip).second + tooltip.getHeight() == Popups::origin(fixture.first).second - 6);
    // It takes no mouse
    CHECK(fixture.window->overlay().popupAt(Popups::center(tooltip).first, Popups::center(tooltip).second) == nullptr);
}

TEST_CASE("A touch long press shows the tooltip when it opens nothing", "[popup][tooltip][touch]") {
    Popups fixture{};
    fixture.first.tooltip("Tip");
    fixture.layout();
    const auto [x, y] = Popups::center(fixture.first);
    fixture.window->handleTouchPress(static_cast<float>(x), static_cast<float>(y), 0ms);
    fixture.window->updateGestures(Window::longPressDelay);
    REQUIRE(fixture.window->tooltip() != nullptr);
    CHECK(fixture.window->tooltip()->text() == "Tip");
    fixture.window->handleTouchRelease(static_cast<float>(x), static_cast<float>(y), Window::longPressDelay + 100ms);
    CHECK(fixture.window->tooltip() == nullptr);
    CHECK(fixture.firstClicks == 0); // A long press never clicks
}

// endregion

// region With the lists and inputs

TEST_CASE("In a dialog, Tab goes through the text fields and stays inside", "[popup][dialog][focus][inputs]") {
    Popups fixture{};
    fixture.key(Key::TAB);
    REQUIRE(fixture.window->focusedDiv() == &fixture.first);

    auto &dialog  = fixture.window->overlay().open<Dialog>("Find");
    auto &search  = dialog.body().add<SearchField>();
    auto &stepper = dialog.body().add<NumberStepper>(4);
    auto &cancel  = dialog.action("Cancel");
    fixture.layout();
    CHECK(fixture.window->focusedDiv() == &search); // Its first field

    std::vector<Div *> visited{};
    for (int i = 0; i < 4; ++i) {
        fixture.key(Key::TAB);
        visited.push_back(fixture.window->focusedDiv());
    }
    // The stepper, Cancel, the close button, the search field's text again
    CHECK(visited[0] == &stepper);
    CHECK(visited[1] == &cancel);
    CHECK(visited[3] == &search);

    fixture.key(Key::ESCAPE);
    CHECK(fixture.popupCount() == 0);
    CHECK(fixture.window->focusedDiv() == &fixture.first);
}

TEST_CASE("In a dialog, Escape goes to a field that has a use for it before closing", "[popup][dialog][inputs]") {
    Popups fixture{};
    auto &dialog  = fixture.window->overlay().open<Dialog>("Find");
    auto &search  = dialog.body().add<SearchField>();
    auto &stepper = dialog.body().add<NumberStepper>(4);
    dialog.action("Cancel");
    fixture.layout();
    REQUIRE(fixture.window->focusedDiv() == &search);

    // With text, Escape clears the search field; empty, it closes the dialog
    search.text("cue");
    fixture.key(Key::ESCAPE);
    CHECK(search.text().empty());
    CHECK(fixture.popupCount() == 1);

    // A number being typed reverts; as typed, Escape closes
    fixture.key(Key::TAB);
    REQUIRE(fixture.window->focusedDiv() == &stepper);
    CHECK_FALSE(stepper.takesEscape());
    fixture.window->handleType(icu::UnicodeString("2"));
    fixture.layout();
    CHECK(stepper.takesEscape());
    fixture.key(Key::ESCAPE);
    CHECK(fixture.popupCount() == 1);
    CHECK(stepper.value() == 4);
    CHECK_FALSE(stepper.takesEscape());
    fixture.key(Key::ESCAPE);
    CHECK(fixture.popupCount() == 0);
}

TEST_CASE("A select in a dialog closes first, and gives the focus back to itself", "[popup][dialog][select][focus]") {
    Popups fixture{};
    auto &dialog = fixture.window->overlay().open<Dialog>("Curve");
    auto &select = dialog.body().add<Select>(std::vector<std::string>{"Linear", "Snap"}, 0);
    fixture.layout();
    fixture.click(select);
    REQUIRE(select.isOpen());
    fixture.key(Key::ESCAPE);
    CHECK(!select.isOpen());
    CHECK(fixture.popupCount() == 1); // The dialog stays
    CHECK(select.hasFocus());
}

TEST_CASE("A list scrolling in a dialog ends with its padding after the last row", "[popup][dialog][lists]") {
    Popups fixture{};
    auto &dialog = fixture.window->overlay().open<Dialog>("People");
    auto &list   = dialog.body().add<List>();
    list.height(120.0f).padding(8.0f).overflow(Overflow::Scroll);
    const ListRow *last = nullptr;
    for (int i = 0; i < 12; ++i) {
        last = &list.row("Row " + std::to_string(i));
    }
    fixture.layout();
    REQUIRE(list.contentHeight() > list.getHeight());
    list.scrollBy(0, -100000);
    fixture.layout();
    // Scrolled to the end, the last row's bottom sits the padding above the list's
    CHECK(last->y() + last->getHeight() + list.scrollY() == list.getHeight() - 8);
}

TEST_CASE("Tooltips show on the new controls", "[popup][tooltip][inputs]") {
    Popups fixture{};
    auto &toggle = fixture.window->appContainer().add<Switch>("Blackout");
    toggle.tooltip("Blackout on start");
    fixture.layout();
    fixture.hover(toggle);
    fixture.window->updatePopups(1000ms);
    fixture.window->updatePopups(1000ms + Window::tooltipDelay);
    REQUIRE(fixture.window->tooltip() != nullptr);
    CHECK(fixture.window->tooltip()->text() == "Blackout on start");
}

// endregion

// region Dialogs

TEST_CASE("A dialog traps the focus and gives it back", "[popup][dialog][focus]") {
    Popups fixture{};
    fixture.key(Key::TAB);
    REQUIRE(fixture.window->focusedDiv() == &fixture.first);

    int  renamed = 0;
    auto &dialog = fixture.window->overlay().open<Dialog>("Rename");
    dialog.description("Something");
    auto &cancel = dialog.action("Cancel");
    auto &ok     = dialog.action("Rename", [&]() { ++renamed; }, "primary");
    CHECK(fixture.window->focusedDiv() == &dialog.panel()); // Until it is laid out
    fixture.layout();
    CHECK(dialog.modal());
    CHECK(fixture.window->focusedDiv() == &cancel); // No field: the first button, ringed since Tab opened it
    CHECK(fixture.window->focusVisible());

    // Tab goes round the actions and the close button, never out
    std::vector<Div *> visited{};
    for (int i = 0; i < 3; ++i) {
        fixture.key(Key::TAB);
        visited.push_back(fixture.window->focusedDiv());
    }
    CHECK(visited[0] == &ok);
    CHECK(visited[2] == &cancel); // After the close button
    fixture.key(Key::TAB, Mod{.shift = true});
    CHECK(fixture.window->focusedDiv() == visited[1]); // The close button
    fixture.key(Key::TAB, Mod{.shift = true});
    CHECK(fixture.window->focusedDiv() == &ok);

    fixture.key(Key::ENTER);
    CHECK(renamed == 1);
    CHECK(fixture.popupCount() == 0);
    CHECK(fixture.window->focusedDiv() == &fixture.first);
}

TEST_CASE("A dialog focuses its first field, else its first button, else its panel, or what it is told", "[popup][dialog][focus]") {
    Popups fixture{};

    SECTION("a field, after buttons in the body") {
        auto &dialog = fixture.window->overlay().open<Dialog>("Rename");
        auto &help   = dialog.body().add<Button>("Help");
        auto &name   = dialog.body().add<TextInput>();
        dialog.action("Cancel");
        fixture.layout();
        CHECK(dialog.initialFocus() == &name);
        CHECK(fixture.window->focusedDiv() == &name); // The field itself
        CHECK_FALSE(help.hasFocus());
        fixture.layout();
        CHECK(name.computedStyle().borderColor() == themes::pivotalDark.palette.accent);
    }

    SECTION("a button in the body, before the footer's") {
        auto &dialog = fixture.window->overlay().open<Dialog>("Pick");
        auto &help   = dialog.body().add<Button>("Help");
        dialog.action("Cancel");
        fixture.layout();
        CHECK(fixture.window->focusedDiv() == &help);
        CHECK_FALSE(fixture.window->focusVisible()); // Not opened from the keyboard: no ring
    }

    SECTION("nothing but the close button: the panel keeps it") {
        auto &dialog = fixture.window->overlay().open<Dialog>("Note").description("Read me");
        fixture.layout();
        CHECK(dialog.initialFocus() == nullptr);
        CHECK(fixture.window->focusedDiv() == &dialog.panel());
    }

    SECTION("initialFocus picks another") {
        auto &dialog = fixture.window->overlay().open<Dialog>("Delete");
        dialog.body().add<TextInput>();
        dialog.action("Cancel");
        auto &remove = dialog.action("Delete", nullptr, "danger");
        dialog.initialFocus(remove);
        fixture.layout();
        CHECK(fixture.window->focusedDiv() == &remove);
    }

    SECTION("only once: a focus moved after it stays") {
        auto &dialog = fixture.window->overlay().open<Dialog>("Twice");
        auto &name   = dialog.body().add<TextInput>();
        auto &cancel = dialog.action("Cancel");
        fixture.layout();
        REQUIRE(name.hasFocus());
        fixture.window->requestFocus(&cancel, true);
        fixture.layout();
        CHECK(cancel.hasFocus());
    }
}

TEST_CASE("A dialog closes by Escape and the backdrop, unless it isn't dismissible", "[popup][dialog][dismiss]") {
    Popups fixture{};
    auto &dialog = fixture.window->overlay().open<Dialog>("One");
    fixture.layout();
    // A press on the panel doesn't close it
    fixture.click(dialog.panel());
    CHECK(fixture.popupCount() == 1);
    fixture.clickAt(5, 5); // The backdrop
    CHECK(fixture.popupCount() == 0);
    CHECK(fixture.firstClicks == 0);

    fixture.window->overlay().open<Dialog>("Two").dismissible(false);
    fixture.layout();
    fixture.clickAt(5, 5);
    CHECK(fixture.popupCount() == 1);
    fixture.key(Key::ESCAPE);
    CHECK(fixture.popupCount() == 0);
}

TEST_CASE("A menu over a dialog closes first", "[popup][dialog][menu]") {
    Popups fixture{};
    auto &dialog = fixture.window->overlay().open<Dialog>("Dialog");
    auto &button = dialog.action("Menu", nullptr, "secondary", false);
    fixture.layout();
    fixture.window->openMenu({Menu::item("One", []() {})}, button);
    fixture.layout();
    REQUIRE(fixture.popupCount() == 2);
    fixture.key(Key::ESCAPE);
    CHECK(fixture.popupCount() == 1);
    CHECK(fixture.top<Dialog>() == &dialog);
}

// endregion

// region Typed children

namespace {
    /**
     * A popup whose removed() doesn't call the base: the overlay tracks it all the same
     */
    class SelfishPopup : public Popup {
    public:
        int removals{0};

    protected:
        void removed() override { ++removals; }
    };

    /**
     * A toast whose removed() doesn't call the base: the stack tracks it all the same
     */
    class SelfishToast : public Toast {
    public:
        explicit SelfishToast(const ToastOptions &options) : Toast(options) {}

        int removals{0};

    protected:
        void removed() override { ++removals; }
    };

    bool tracked(const Overlay &overlay, const Popup &popup) {
        return std::ranges::any_of(overlay.allPopups(), [&popup](const auto &other) { return other.get() == &popup; });
    }
}

TEST_CASE("The overlay tracks its popups however they come and go", "[popup][overlay]") {
    Popups  fixture{};
    Overlay &overlay = fixture.window->overlay();

    SECTION("removed by Div::remove") {
        auto &popup = overlay.open<Popup>();
        REQUIRE(tracked(overlay, popup));
        REQUIRE(popup.isOpen());
        overlay.remove(popup);
        CHECK_FALSE(tracked(overlay, popup));
    }

    SECTION("removed by removeAll") {
        const auto popup = std::make_shared<Popup>();
        overlay.open(popup);
        overlay.removeAll();
        CHECK(overlay.allPopups().empty());
        CHECK_FALSE(popup->isOpen());
    }

    SECTION("a subclass's removed() that doesn't call the base") {
        const auto popup = std::make_shared<SelfishPopup>();
        overlay.open(popup);
        REQUIRE(tracked(overlay, *popup));
        overlay.remove(popup);
        CHECK(popup->removals == 1);
        CHECK_FALSE(tracked(overlay, *popup));
        CHECK_FALSE(popup->isOpen());
    }

    SECTION("added with Div::add, then moved to another div") {
        const auto popup = std::make_shared<Popup>();
        overlay.add(popup);
        CHECK(tracked(overlay, *popup));
        CHECK(popup->isOpen());
        fixture.bar.add(popup);
        CHECK(popup->parent() == &fixture.bar);
        CHECK_FALSE(tracked(overlay, *popup));
        CHECK_FALSE(popup->isOpen());
    }

    SECTION("in the children's order") {
        auto &bottom = overlay.open<Popup>();
        auto &top    = overlay.open<Popup>();
        const auto middle = std::make_shared<Popup>();
        overlay.add(overlay.childIndex(&top), middle);
        std::vector<Popup *> order{};
        for (const auto &child: overlay.children()) {
            if (auto *popup = dynamic_cast<Popup *>(child.get())) {
                order.push_back(popup);
            }
        }
        std::vector<Popup *> popups{};
        for (const auto &popup: overlay.allPopups()) {
            popups.push_back(popup.get());
        }
        CHECK(popups == order);
        CHECK(std::ranges::find(popups, &bottom) < std::ranges::find(popups, middle.get()));
        CHECK(std::ranges::find(popups, middle.get()) < std::ranges::find(popups, &top));
    }
}

TEST_CASE("The toast stack tracks its toasts however they come and go", "[popup][toast]") {
    const Popups fixture{};
    ToastStack   &stack = fixture.window->toasts();
    Div          &asDiv = stack; // Its own add makes a toast from options

    SECTION("removed by Div::remove") {
        auto &toast = fixture.window->toast({.title = "One", .timeout = 0s});
        REQUIRE(stack.toasts().size() == 1);
        stack.remove(toast);
        CHECK(stack.toasts().empty());
    }

    SECTION("a subclass's removed() that doesn't call the base") {
        const auto toast = std::make_shared<SelfishToast>(ToastOptions{.title = "Two", .timeout = 0s});
        asDiv.add(toast);
        REQUIRE(stack.toasts() == std::vector<Toast *>{toast.get()});
        stack.removeAll();
        CHECK(toast->removals == 1);
        CHECK(stack.toasts().empty());
    }
}

// endregion

// region Toasts

TEST_CASE("Toasts time out, held while the mouse is over them", "[popup][toast]") {
    Popups fixture{};
    const ToastOptions options{.title = "Saved", .timeout = 5s};
    const Toast &toast = fixture.window->toast(options);
    fixture.layout();
    const ToastStack &stack = fixture.window->toasts();
    CHECK(hasClass(toast, "info"));
    // In the bottom right corner, inside the window's 1 px border
    const auto [x, y] = Popups::origin(toast);
    CHECK(x + toast.getWidth() == 800 - 1 - 16);
    CHECK(y + toast.getHeight() == 600 - 1 - 16);

    fixture.window->updatePopups(10s); // Shown from this frame
    fixture.window->updatePopups(12s);
    CHECK(toast.elapsed() == 2s);

    // Held while the mouse is over the stack
    fixture.hover(toast);
    fixture.window->updatePopups(20s);
    CHECK(toast.elapsed() == 2s);
    // Frames stop while nothing changes: the mouse leaving brings the next one, and the time until it didn't count
    fixture.window->updatePopups(40s);
    CHECK(toast.elapsed() == 2s);
    fixture.moveTo(10, 300);
    fixture.window->updatePopups(40s);
    CHECK(toast.elapsed() == 2s);
    fixture.window->updatePopups(42s);
    CHECK(toast.elapsed() == 4s);
    CHECK(stack.toasts().size() == 1);
    fixture.window->updatePopups(43s);
    CHECK(stack.toasts().empty());
}

TEST_CASE("A toast's actions and close button dismiss it; the stack keeps a few", "[popup][toast]") {
    Popups fixture{};
    int  retried = 0;
    auto &toast  = fixture.window->toast({.title = "Offline", .variant = ToastVariant::Warning, .timeout = 0s,
                                    .actions = {{"Retry", [&]() { ++retried; }, true}}});
    fixture.layout();
    CHECK(hasClass(toast, "warning"));
    fixture.window->updatePopups(0s);
    fixture.window->updatePopups(1000s);
    CHECK(fixture.window->toasts().toasts().size() == 1); // No timeout

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
    fixture.click(*retry);
    CHECK(retried == 1);
    CHECK(fixture.window->toasts().toasts().empty());

    for (int i = 0; i < 5; ++i) {
        fixture.window->toast({.title = "Toast " + std::to_string(i)});
    }
    fixture.layout();
    auto toasts = fixture.window->toasts().toasts();
    REQUIRE(toasts.size() == ToastStack::limit);
    CHECK(toasts.front()->options().title == "Toast 2");
    CHECK(toasts.back()->options().title == "Toast 4");
    // Toasts don't take the focus from the page, nor Escape
    CHECK_FALSE(fixture.window->overlay().dismissTop());
}

// endregion
