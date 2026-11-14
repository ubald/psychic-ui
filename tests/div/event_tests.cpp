#include <memory>
#include <type_traits>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/TextInput.hpp>

using namespace psychic_ui;

namespace {
    struct Tree {
        std::shared_ptr<Window> window = std::make_shared<Window>("events");
        Div                     *box{nullptr};

        Tree() {
            window->setStyleManager(std::make_shared<StyleManager>());
            window->setWindowSize(800, 600);
            box = &window->appContainer().add<Div>().size(100, 100);
            window->updateStylesAndLayout();
        }

        void press(int x, int y, MouseButton button = MouseButton::LEFT) const {
            window->handleMouseButtonChange(x, y, button, true, Mod{});
            window->updateStylesAndLayout();
        }

        void release(int x, int y, MouseButton button = MouseButton::LEFT) const {
            window->handleMouseButtonChange(x, y, button, false, Mod{});
            window->updateStylesAndLayout();
        }
    };
}

static_assert(std::is_same_v<decltype(std::declval<Button &>().onClick([] {})), Button &>);
static_assert(std::is_same_v<decltype(std::declval<Button &>().onChange([](bool) {}).toggle(true)), Button &>);
static_assert(std::is_same_v<decltype(std::declval<Range<int> &>().onChange([](int) {}).range(0, 1)), Range<int> &>);

TEST_CASE("mouse up fires once per release", "[div][events]") {
    Tree t;
    int  ups     = 0;
    int  outside = 0;
    int  clicks  = 0;
    t.box->onMouseRelease([&](int, int, MouseButton, Mod) { ++ups; })
          .onMouseReleaseOutside([&](int, int, MouseButton, Mod) { ++outside; })
          .onClick([&] { ++clicks; });

    SECTION("pressed and released over the div") {
        t.press(50, 50);
        t.release(50, 50);
        REQUIRE(ups == 1);
        REQUIRE(outside == 0);
        REQUIRE(clicks == 1);
    }

    SECTION("pressed over the div, released outside: the press still ends there") {
        t.press(50, 50);
        t.release(500, 500);
        REQUIRE(ups == 1);
        REQUIRE(outside == 1);
        REQUIRE(clicks == 0);
    }

    SECTION("released over the div without a press there") {
        t.press(500, 500);
        t.release(50, 50);
        REQUIRE(ups == 1);
        REQUIRE(outside == 0);
        REQUIRE(clicks == 0);
    }

    SECTION("released elsewhere without a press") {
        t.press(500, 500);
        t.release(600, 500);
        REQUIRE(ups == 0);
        REQUIRE(outside == 0);
    }
}

TEST_CASE("a connector's callback lives as long as its div", "[div][events]") {
    auto marker = std::make_shared<int>(0);
    auto parent = std::make_shared<Div>();
    auto &child = parent->add<Div>().onClick([marker] { ++*marker; });
    REQUIRE(marker.use_count() == 2);

    child.clicked.emit();
    REQUIRE(*marker == 1);

    parent->removeAll();
    REQUIRE(marker.use_count() == 1);
}

TEST_CASE("components report changes the same way", "[div][events]") {
    auto root = std::make_shared<Div>();

    SECTION("Button") {
        std::vector<bool> changes;
        auto &button = root->add<Button>("b").toggle(true).onChange([&](bool selected) { changes.push_back(selected); });
        button.selected(true).selected(true).selected(false);
        REQUIRE(changes == std::vector<bool>{true, false});
        button.clicked.emit(); // A toggle button selects itself when clicked
        REQUIRE(changes == std::vector<bool>{true, false, true});
    }

    SECTION("Button, several listeners") {
        int a = 0;
        int b = 0;
        root->add<Button>("b").onChange([&](bool) { ++a; }).onChange([&](bool) { ++b; }).selected(true);
        REQUIRE(a == 1);
        REQUIRE(b == 1);
    }

    SECTION("CheckBox") {
        std::vector<bool> changes;
        root->add<CheckBox>("c").onChange([&](bool checked) { changes.push_back(checked); }).checked(true);
        REQUIRE(changes == std::vector<bool>{true});
    }

    SECTION("Range") {
        std::vector<int> changes;
        root->add<Range<int>>().range(0, 10).onChange([&](int value) { changes.push_back(value); }).value(4).value(40);
        REQUIRE(changes == std::vector<int>{4, 10});
    }

    SECTION("Button's constructor callback runs after the selection changed") {
        bool seen = false;
        auto &button = root->add<Button>("b", [&] { seen = true; }).toggle(true);
        bool selectedInCallback = false;
        button.onClick([&] { selectedInCallback = button.selected(); });
        button.clicked.emit();
        REQUIRE(seen);
        REQUIRE(selectedInCallback);
    }
}

TEST_CASE("only the left button clicks", "[div][events]") {
    Tree                     t;
    int                      clicks       = 0;
    int                      doubleClicks = 0;
    std::vector<MouseButton> pressed{};
    std::vector<MouseButton> released{};
    std::vector<MouseButton> changed{};
    t.box->onClick([&] { ++clicks; })
          .onDoubleClick([&](unsigned int) { ++doubleClicks; })
          .onMousePress([&](int, int, MouseButton button, Mod) { pressed.push_back(button); })
          .onMouseRelease([&](int, int, MouseButton button, Mod) { released.push_back(button); })
          .onMouseButtonChange([&](int, int, MouseButton button, bool, Mod) { changed.push_back(button); });

    t.press(50, 50, MouseButton::RIGHT);
    t.release(50, 50, MouseButton::RIGHT);
    t.press(50, 50, MouseButton::MIDDLE);
    t.release(50, 50, MouseButton::MIDDLE);
    t.press(50, 50, MouseButton::RIGHT); // Quickly again: still no double click
    t.release(50, 50, MouseButton::RIGHT);
    CHECK(clicks == 0);
    CHECK(doubleClicks == 0);
    // Every button still presses, releases and changes
    CHECK(pressed == std::vector{MouseButton::RIGHT, MouseButton::MIDDLE, MouseButton::RIGHT});
    CHECK(released == pressed);
    CHECK(changed.size() == 6);

    t.press(50, 50);
    t.release(50, 50);
    CHECK(clicks == 1);
    t.press(50, 50);
    t.release(50, 50);
    CHECK(clicks == 2);
    CHECK(doubleClicks == 1);
}

TEST_CASE("a right press on a clicked div reaches its parent", "[div][events]") {
    Tree t;
    t.box->add<Div>().size(50, 50).onClick([] {});
    t.window->updateStylesAndLayout();
    std::vector<MouseButton> parentPresses{};
    t.box->onMousePress([&](int, int, MouseButton button, Mod) { parentPresses.push_back(button); });

    t.press(10, 10);
    t.release(10, 10);
    t.press(10, 10, MouseButton::RIGHT);
    t.release(10, 10, MouseButton::RIGHT);
    CHECK(parentPresses == std::vector{MouseButton::RIGHT}); // The left one is the child's click
}

TEST_CASE("a button looks pressed and clicks for the left button only", "[div][events]") {
    Tree t;
    int  clicks = 0;
    auto &button = t.window->appContainer().add<Button>("Go").size(100, 40).onClick([&] { ++clicks; });
    t.window->updateStylesAndLayout();
    int x = 0;
    int y = 0;
    button.localToGlobal(x, y);

    t.press(x + 10, y + 10, MouseButton::RIGHT);
    CHECK_FALSE(button.selected());
    t.release(x + 10, y + 10, MouseButton::RIGHT);
    CHECK(clicks == 0);

    t.press(x + 10, y + 10);
    CHECK(button.selected());
    t.release(x + 10, y + 10);
    CHECK_FALSE(button.selected());
    CHECK(clicks == 1);
}
