#include <memory>
#include <type_traits>
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

        void press(int x, int y) const {
            window->handleMouseButton(x, y, MouseButton::LEFT, true, Mod{});
            window->updateStylesAndLayout();
        }

        void release(int x, int y) const {
            window->handleMouseButton(x, y, MouseButton::LEFT, false, Mod{});
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
    t.box->onMouseUp([&](int, int, MouseButton, Mod) { ++ups; })
          .onMouseUpOutside([&](int, int, MouseButton, Mod) { ++outside; })
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
