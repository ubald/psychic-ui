#include <memory>
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Skin.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/ColorSwatches.hpp>
#include <psychic-ui/components/Icon.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/SearchField.hpp>
#include <psychic-ui/components/SegmentedControl.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/skins/base/ColorSwatchesSkinBase.hpp>
#include <psychic-ui/skins/base/NumberStepperSkinBase.hpp>
#include <psychic-ui/skins/base/SearchFieldSkinBase.hpp>
#include <psychic-ui/skins/base/SegmentedControlSkinBase.hpp>
#include <psychic-ui/skins/base/SwitchSkinBase.hpp>
#include <psychic-ui/style/Color.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    /**
     * Whether the skin is a T, not a subclass of it
     */
    template<class T>
    bool is(const std::shared_ptr<internal::SkinBase> &skin) {
        if (!skin) {
            return false;
        }
        // Dereferenced outside typeid, which would otherwise evaluate a call in its operand
        const internal::SkinBase &base = *skin;
        return typeid(base) == typeid(T);
    }

    struct Inputs {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 800, 600};
        const Theme                 &theme{themes::pivotalDark};
        Div                         &column{window->appContainer().add<Div>()};

        Inputs() {
            column.alignItems(Align::Start).gap(8).padding(8);
        }

        void layout() { systemWindow.layout(); }

        static std::pair<int, int> at(const Div &div, int x, int y) {
            int windowX = 0;
            int windowY = 0;
            div.localToGlobal(windowX, windowY, x, y);
            return {windowX, windowY};
        }

        static std::pair<int, int> center(const Div &div) {
            return at(div, div.getWidth() / 2, div.getHeight() / 2);
        }

        void press(int x, int y) {
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true});
        }

        void move(int x, int y) {
            window->handleMouseMove({.x = static_cast<float>(x), .y = static_cast<float>(y)}, false);
        }

        void release(int x, int y) {
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false});
            layout();
        }

        void click(const Div &div) {
            const auto [x, y] = center(div);
            press(x, y);
            release(x, y);
        }

        void key(Key key, Mod mod = Mod{}) {
            window->handleKeyPress(key, mod);
            layout();
        }

        void type(const std::string &text) {
            for (const char character: text) {
                window->handleType(icu::UnicodeString(static_cast<UChar32>(character)));
            }
            layout();
        }

        void tab() { key(Key::TAB); }
    };

    template<class T>
    T *part(Div &root, const std::string &className) {
        for (const auto &child: root.children()) {
            if (child->classNames().contains(className)) {
                if (auto *found = dynamic_cast<T *>(child.get())) {
                    return found;
                }
            }
            // NOLINTNEXTLINE(misc-const-correctness): returned as a mutable pointer, which the check misses
            if (T *found = part<T>(*child, className)) {
                return found;
            }
        }
        return nullptr;
    }

    std::vector<Div *> parts(Div &root, const std::string &className) {
        std::vector<Div *> found{};
        for (const auto &child: root.children()) {
            if (child->classNames().contains(className)) {
                found.push_back(child.get());
            }
            auto deeper = parts(*child, className);
            found.insert(found.end(), deeper.begin(), deeper.end());
        }
        return found;
    }
}

// region Text fields

TEST_CASE("TextInput: Tab focuses the field itself, so one can type", "[components][inputs][focus]") {
    Inputs fixture;
    auto &input = fixture.column.add<TextInput>();
    fixture.layout();

    fixture.tab();
    CHECK(input.hasFocus());
    CHECK(fixture.window->focusedDiv() == &input); // The field itself takes the keys
    fixture.type("abc");
    CHECK(input.text() == "abc");

    // Tab goes on from the field, not into its text
    auto &other = fixture.column.add<TextInput>();
    fixture.layout();
    fixture.tab();
    CHECK(fixture.window->focusedDiv() == &other);
    CHECK_FALSE(input.hasFocus());
}

TEST_CASE("TextInput: a placeholder while it is empty", "[components][inputs]") {
    Inputs fixture;
    auto &input = fixture.column.add<TextInput>().placeholder("Name");
    fixture.layout();
    const Label *placeholder = part<Label>(input, "placeholder");
    REQUIRE(placeholder);
    CHECK(placeholder->text() == "Name");
    CHECK(placeholder->visible());
    CHECK(placeholder->computedStyle().color() == fixture.theme.palette.textMuted);

    input.text("Ada");
    fixture.layout();
    CHECK_FALSE(placeholder->visible());

    fixture.tab();
    fixture.key(Key::END);
    fixture.key(Key::BACKSPACE);
    fixture.key(Key::BACKSPACE);
    fixture.key(Key::BACKSPACE);
    CHECK(input.text().empty());
    CHECK(placeholder->visible());

    // None by default
    auto &bare = fixture.column.add<TextInput>();
    fixture.layout();
    CHECK_FALSE(part<Label>(bare, "placeholder")->visible());
}

// endregion

// region Search field

TEST_CASE("SearchField: icon, text, and a clear button while not empty", "[components][inputs][search]") {
    Inputs fixture;
    auto &search = fixture.column.add<SearchField>().width(240);
    std::vector<std::string> changes{};
    std::vector<std::string> submits{};
    search.onChange([&](const std::string &text) { changes.push_back(text); });
    search.onSubmit([&](const std::string &text) { submits.push_back(text); });
    fixture.layout();

    REQUIRE(is<SearchFieldSkinBase>(search.skin()));
    const Div  *icon        = part<Icon>(search, "icon");
    Text       *text        = part<Text>(search, "text");
    const Div  *clear       = part<Div>(search, "clear");
    const auto *placeholder = part<Label>(search, "placeholder");
    REQUIRE(icon);
    REQUIRE(text);
    REQUIRE(clear);
    REQUIRE(placeholder);
    CHECK(text == search.skin()->textView());
    CHECK(search.placeholder() == "Search");
    CHECK(placeholder->text() == "Search");
    CHECK(placeholder->visible());
    CHECK(placeholder->computedStyle().color() == fixture.theme.palette.textMuted);
    CHECK(search.getHeight() == fixture.theme.metrics.pointer.controlMd);
    CHECK(search.computedStyle().backgroundColor() == fixture.theme.palette.raised);
    CHECK(text->computedStyle().border() == 0.0f);
    CHECK(icon->x() < text->parent()->x()); // Before the text, in the skin
    CHECK(text->x() == placeholder->x());    // The placeholder where the text starts
    CHECK_FALSE(clear->visible());
    CHECK(clear->computedStyle().cursor() == Cursor::Arrow);

    // Tab: the field is focused (its accent border) and typing goes to its text
    fixture.tab();
    CHECK(fixture.window->focusedDiv() == &search); // The field itself takes the keys
    CHECK(search.computedStyle().borderColor() == fixture.theme.palette.accent);
    fixture.type("cue");
    CHECK(search.text() == "cue");
    CHECK(text->text() == "cue");
    CHECK(changes == std::vector<std::string>{"c", "cu", "cue"});
    CHECK(clear->visible());
    CHECK_FALSE(placeholder->visible());

    fixture.key(Key::ENTER);
    CHECK(submits == std::vector<std::string>{"cue"});
    CHECK(search.text() == "cue");
}

TEST_CASE("SearchField: Escape clears it, once, then leaves the key alone", "[components][inputs][search]") {
    Inputs fixture;
    auto &search = fixture.column.add<SearchField>("cue").width(240);
    std::vector<std::string> changes{};
    search.onChange([&](const std::string &text) { changes.push_back(text); });
    fixture.layout();
    const Div *clear = part<Div>(search, "clear");
    CHECK(clear->visible());

    fixture.tab();
    CHECK(search.takesEscape());
    fixture.key(Key::ESCAPE);
    CHECK(search.text().empty());
    CHECK(changes == std::vector<std::string>{""});
    CHECK_FALSE(clear->visible());
    CHECK(part<Label>(search, "placeholder")->visible());
    CHECK_FALSE(search.takesEscape());
    fixture.key(Key::ESCAPE);
    CHECK(changes.size() == 1);
}

TEST_CASE("SearchField: a press on the clear button clears it, and the field keeps the focus", "[components][inputs][search]") {
    Inputs fixture;
    auto &search = fixture.column.add<SearchField>().width(240);
    std::vector<std::string> changes{};
    search.onChange([&](const std::string &text) { changes.push_back(text); });
    fixture.layout();
    fixture.tab();
    fixture.type("fixtures");
    changes.clear();

    const Div *clear = part<Div>(search, "clear");
    REQUIRE(clear->visible());
    const auto [clearX, clearY] = Inputs::center(*clear);
    int x = 0;
    int y = 0;
    search.skin()->globalToLocal(x, y, clearX, clearY);
    CHECK(search.skin()->clearAt(static_cast<float>(x), static_cast<float>(y)));
    CHECK_FALSE(search.skin()->clearAt(0, 0));

    fixture.click(*clear);
    CHECK(search.text().empty());
    CHECK(changes == std::vector<std::string>{""});
    CHECK(search.hasFocus());
    CHECK_FALSE(clear->visible());
    CHECK_FALSE(search.skin()->clearAt(static_cast<float>(x), static_cast<float>(y))); // Hidden: nothing to clear

    // A press elsewhere on the text places the caret, and clears nothing
    search.text("cue");
    fixture.layout();
    fixture.click(*part<Text>(search, "text"));
    CHECK(search.text() == "cue");
}

TEST_CASE("SearchField: a press anywhere on it lets one type", "[components][inputs][search]") {
    Inputs fixture;
    auto &search = fixture.column.add<SearchField>().width(240);
    fixture.layout();
    const Div *icon = part<Icon>(search, "icon");
    fixture.click(*icon);
    CHECK(search.hasFocus());
    CHECK_FALSE(fixture.window->focusVisible());
    fixture.type("x");
    CHECK(search.text() == "x");
}

TEST_CASE("SearchField: disabled, it refuses input", "[components][inputs][search]") {
    Inputs fixture;
    auto &search = fixture.column.add<SearchField>("cue").width(240).enabled(false);
    std::vector<std::string> changes{};
    std::vector<std::string> submits{};
    search.onChange([&](const std::string &text) { changes.push_back(text); });
    search.onSubmit([&](const std::string &text) { submits.push_back(text); });
    fixture.layout();

    // A press focuses nothing that takes typing, nor clears it
    fixture.click(*part<Div>(search, "clear"));
    fixture.click(search);
    fixture.type("x");
    CHECK(search.text() == "cue");

    // Even given the focus, the keys do nothing
    fixture.window->requestFocus(&search, true);
    fixture.type("x");
    fixture.key(Key::BACKSPACE);
    fixture.key(Key::ENTER);
    CHECK_FALSE(search.takesEscape());
    fixture.key(Key::ESCAPE);
    CHECK(search.text() == "cue");
    CHECK(changes.empty());
    CHECK(submits.empty());
}

// endregion

// region Switch

TEST_CASE("Switch: a click, Space or Enter flips it", "[components][inputs][switch]") {
    Inputs fixture;
    auto &toggle = fixture.column.add<Switch>("Follow system");
    std::vector<bool> changes{};
    toggle.onChange([&](bool checked) { changes.push_back(checked); });
    fixture.layout();

    REQUIRE(is<SwitchSkinBase>(toggle.skin()));
    const Div *track = part<Div>(toggle, "track");
    const Div *thumb = part<Div>(toggle, "thumb");
    REQUIRE(track);
    REQUIRE(thumb);
    CHECK(track->getWidth() == 30);
    CHECK(track->getHeight() == 18);
    CHECK(thumb->getWidth() == 12);
    CHECK(track->computedStyle().backgroundColor() == fixture.theme.palette.switchOff);
    CHECK(thumb->x() == 3); // The border and 2 px in

    fixture.click(toggle);
    CHECK(toggle.checked());
    CHECK(toggle.classNames().contains("checked"));
    CHECK(track->computedStyle().backgroundColor() == fixture.theme.palette.accent);
    CHECK(thumb->x() == 15); // 12 px along: the end of the travel

    fixture.tab();
    CHECK(toggle.hasFocus());
    CHECK(track->computedStyle().outlineColor() == fixture.theme.palette.accent);
    fixture.key(Key::SPACE);
    CHECK_FALSE(toggle.checked());
    fixture.key(Key::ENTER);
    CHECK(toggle.checked());
    CHECK(changes == std::vector{true, false, true});

    toggle.enabled(false);
    fixture.click(toggle);
    fixture.key(Key::SPACE);
    CHECK(toggle.checked());
}

TEST_CASE("Switch: the thumb drags, and lands on the nearer side", "[components][inputs][switch]") {
    Inputs fixture;
    auto &toggle = fixture.column.add<Switch>();
    std::vector<bool> changes{};
    toggle.onChange([&](bool checked) { changes.push_back(checked); });
    fixture.layout();
    const Div *thumb = part<Div>(toggle, "thumb");
    auto [x, y] = Inputs::center(*thumb);

    // A little way: back off, no change and no click
    fixture.press(x, y);
    fixture.move(x + 4, y);
    CHECK(toggle.dragging());
    CHECK(toggle.thumbPosition() > 0.0f);
    CHECK(toggle.thumbPosition() < 0.5f);
    fixture.release(x + 4, y);
    CHECK_FALSE(toggle.checked());
    CHECK(changes.empty());
    CHECK(thumb->x() == 3);

    // Past the middle, released outside: on
    fixture.press(x, y);
    fixture.move(x + 9, y);
    fixture.layout();
    CHECK(thumb->x() > 3);
    CHECK(thumb->x() < 15);
    fixture.move(x + 40, y + 30);
    CHECK(toggle.thumbPosition() == 1.0f); // Clamped
    fixture.release(x + 40, y + 30);
    CHECK(toggle.checked());
    CHECK(changes == std::vector{true});
    CHECK(thumb->x() == 15);

    // And back from on
    std::tie(x, y) = Inputs::center(*thumb);
    fixture.press(x, y);
    fixture.move(x - 12, y);
    fixture.release(x - 12, y);
    CHECK_FALSE(toggle.checked());
    CHECK(changes == std::vector{true, false});
}

// endregion

// region Segmented control

TEST_CASE("SegmentedControl: one segment selected, by click or arrow", "[components][inputs][segmented]") {
    Inputs fixture;
    auto &control = fixture.column.add<SegmentedControl>(
        std::vector<Segment>{{"Day"}, {"Week", "calendar"}, {"", "list"}}
    );
    std::vector<int> changes{};
    control.onChange([&](int index) { changes.push_back(index); });
    fixture.layout();

    REQUIRE(is<SegmentedControlSkinBase>(control.skin()));
    const auto segments = parts(control, "segment");
    REQUIRE(segments.size() == 3);
    CHECK(control.selected() == 0);
    CHECK(segments[0]->classNames().contains("selected"));
    CHECK(control.getHeight() == fixture.theme.metrics.pointer.controlMd);
    CHECK(segments[0]->getHeight() == control.getHeight() - 4); // In its border and 1 px padding
    CHECK(segments[0]->computedStyle().backgroundColor() == fixture.theme.palette.raised);
    CHECK(segments[1]->computedStyle().color() == fixture.theme.palette.textMuted);
    // Label, icon and label, icon alone
    CHECK(parts(*segments[0], "icon").empty());
    CHECK(parts(*segments[1], "icon").size() == 1);
    CHECK(parts(*segments[2], "label").empty());

    fixture.click(*segments[1]);
    CHECK(control.selected() == 1);
    CHECK(segments[1]->classNames().contains("selected"));
    CHECK_FALSE(segments[0]->classNames().contains("selected"));
    CHECK(segments[1]->computedStyle().color() == fixture.theme.palette.text);

    fixture.tab();
    CHECK(control.hasFocus());
    CHECK(segments[1]->computedStyle().outlineColor() == fixture.theme.palette.accent);
    fixture.key(Key::RIGHT);
    CHECK(control.selected() == 2);
    fixture.key(Key::RIGHT); // At the end
    CHECK(control.selected() == 2);
    fixture.key(Key::HOME);
    CHECK(control.selected() == 0);
    fixture.key(Key::LEFT);
    CHECK(control.selected() == 0);
    fixture.key(Key::END);
    CHECK(changes == std::vector{1, 2, 0, 2});

    control.selected(7);
    CHECK(control.selected() == 2);
    control.segments({{"A"}, {"B"}});
    fixture.layout();
    CHECK(control.selected() == 0); // 2 is gone
    CHECK(parts(control, "segment").size() == 2);
}

// endregion

// region Number stepper

TEST_CASE("NumberStepper: the buttons step it within its range", "[components][inputs][stepper]") {
    Inputs fixture;
    auto &stepper = fixture.column.add<NumberStepper>(2).range(0, 3);
    std::vector<double> changes{};
    stepper.onChange([&](double value) { changes.push_back(value); });
    fixture.layout();

    REQUIRE(is<NumberStepperSkinBase>(stepper.skin()));
    const Div &minus = *part<Div>(stepper, "decrement");
    const Div &plus  = *part<Div>(stepper, "increment");
    Text &value = *part<Text>(stepper, "value");
    CHECK(&value == stepper.skin()->textView());
    CHECK(stepper.getWidth() == fixture.theme.metrics.pointer.controlMd * 4);
    CHECK(stepper.getHeight() == fixture.theme.metrics.pointer.controlMd);
    CHECK(minus.x() < value.x());
    CHECK(value.x() < plus.x());
    CHECK(value.text() == "2");
    CHECK(stepper.text() == "2");
    CHECK(minus.computedStyle().cursor() == Cursor::Arrow);
    CHECK(value.computedStyle().cursor() == Cursor::IBeam);

    // The skin says which button a point is on
    const auto skinPoint = [&](const Div &div) {
        const auto [windowX, windowY] = Inputs::center(div);
        int x = 0;
        int y = 0;
        stepper.skin()->globalToLocal(x, y, windowX, windowY);
        return std::pair{static_cast<float>(x), static_cast<float>(y)};
    };
    CHECK(stepper.skin()->partAt(skinPoint(minus).first, skinPoint(minus).second) == NumberStepperSkin::Part::Decrement);
    CHECK(stepper.skin()->partAt(skinPoint(plus).first, skinPoint(plus).second) == NumberStepperSkin::Part::Increment);
    CHECK(stepper.skin()->partAt(skinPoint(value).first, skinPoint(value).second) == NumberStepperSkin::Part::None);

    // Hovered and pressed, through rules
    const auto [plusX, plusY] = Inputs::center(plus);
    fixture.move(plusX, plusY);
    fixture.layout();
    CHECK(plus.computedStyle().backgroundColor() == fixture.theme.palette.line);

    fixture.click(plus);
    CHECK(stepper.value() == 3);
    CHECK_FALSE(plus.enabled()); // At the maximum: `:disabled`
    CHECK(plus.computedStyle().color() == fixture.theme.palette.textMuted);
    fixture.click(plus);
    CHECK(stepper.value() == 3);
    fixture.click(minus);
    fixture.click(minus);
    fixture.click(minus);
    CHECK(stepper.value() == 0);
    CHECK_FALSE(minus.enabled());
    CHECK(plus.enabled());
    CHECK(changes == std::vector<double>{3, 2, 1, 0});
    CHECK(value.text() == "0");
    CHECK(stepper.hasFocus()); // A press on a button focuses the stepper

    stepper.value(-5);
    CHECK(stepper.value() == 0);
    stepper.max(10).value(12);
    CHECK(stepper.value() == 10);

    // Disabled: both buttons are, and presses do nothing
    stepper.value(5).enabled(false);
    fixture.layout();
    CHECK_FALSE(minus.enabled());
    CHECK_FALSE(plus.enabled());
    fixture.click(plus);
    CHECK(stepper.value() == 5);
}

TEST_CASE("NumberStepper: typed, and stepped by the keys", "[components][inputs][stepper]") {
    Inputs fixture;
    auto &stepper = fixture.column.add<NumberStepper>(1.5).range(0, 100).step(0.5);
    fixture.column.add<Switch>(); // Somewhere for Tab to go next
    fixture.layout();
    CHECK(stepper.text() == "1.5");

    fixture.tab();
    CHECK(fixture.window->focusedDiv() == &stepper); // The stepper itself takes the keys
    CHECK(stepper.computedStyle().borderColor() == fixture.theme.palette.accent);

    fixture.key(Key::UP);
    CHECK(stepper.value() == 2.0);
    CHECK(stepper.text() == "2.0");
    CHECK(stepper.caret() == 3); // At the end
    fixture.key(Key::DOWN, Mod{.shift = true});
    CHECK(stepper.value() == 0.0); // Ten steps down, clamped

    // Typing: Enter sets it, clamped, and rounded to the decimals shown
    stepper.text("");
    fixture.type("42.25");
    CHECK(stepper.value() == 0.0); // Not yet
    fixture.key(Key::ENTER);
    CHECK(stepper.value() == 42.3);
    CHECK(stepper.text() == "42.3");

    stepper.text("");
    fixture.type("500");
    fixture.key(Key::ENTER);
    CHECK(stepper.value() == 100);

    // Not a number: back to the value; Escape gives the typing up
    stepper.text("");
    fixture.type("abc");
    fixture.key(Key::ENTER);
    CHECK(stepper.value() == 100);
    CHECK(stepper.text() == "100.0");
    stepper.text("");
    fixture.type("7");
    CHECK(stepper.takesEscape());
    fixture.key(Key::ESCAPE);
    CHECK(stepper.text() == "100.0");
    CHECK_FALSE(stepper.takesEscape());

    // The text is edited as any field's: the caret moves, Backspace erases
    fixture.key(Key::HOME);
    fixture.key(Key::DELETE);
    CHECK(stepper.text() == "00.0");
    fixture.key(Key::ESCAPE);

    // Leaving the field sets what was typed
    stepper.text("");
    fixture.type("8");
    fixture.tab();
    CHECK(stepper.value() == 8);
    CHECK_FALSE(stepper.hasFocus());
}

TEST_CASE("NumberStepper: the wheel steps it while it has the focus", "[components][inputs][stepper]") {
    Inputs fixture;
    auto &stepper = fixture.column.add<NumberStepper>(5).range(0, 10);
    fixture.layout();
    const auto [x, y] = Inputs::center(stepper);
    const auto scroll = [&](float dy) {
        fixture.window->handleMouseScroll({.x = static_cast<float>(x), .y = static_cast<float>(y), .scrollY = dy});
        fixture.layout();
    };
    scroll(1);
    CHECK(stepper.value() == 5); // Not focused: it scrolls what is around it
    fixture.tab();
    scroll(1);
    CHECK(stepper.value() == 6);
    scroll(-1);
    scroll(-1);
    CHECK(stepper.value() == 4);
}

TEST_CASE("NumberStepper: disabled, it refuses input", "[components][inputs][stepper]") {
    Inputs fixture;
    auto &stepper = fixture.column.add<NumberStepper>(5).enabled(false);
    std::vector<double> changes{};
    stepper.onChange([&](double value) { changes.push_back(value); });
    fixture.layout();
    fixture.window->requestFocus(&stepper, true);
    fixture.type("9");
    fixture.key(Key::UP);
    fixture.key(Key::ENTER);
    CHECK(stepper.text() == "5");
    CHECK(stepper.value() == 5);
    CHECK(changes.empty());
}

TEST_CASE("NumberStepper: decimal steps don't drift", "[components][inputs][stepper]") {
    Inputs fixture;
    auto &stepper = fixture.column.add<NumberStepper>().step(0.1);
    fixture.layout();
    for (int i = 0; i < 3; ++i) {
        stepper.stepBy(1);
    }
    CHECK(stepper.value() == 0.3);
    CHECK(stepper.valueString() == "0.3");
    stepper.decimals(2);
    CHECK(stepper.valueString() == "0.30");
}

// endregion

// region Color swatches

TEST_CASE("ColorSwatches: a chip picked by a press, or by the keyboard's own focus", "[components][inputs][swatches]") {
    Inputs fixture;
    auto &swatches = fixture.column.add<ColorSwatches>(std::vector{rgb(0xE5484D), rgb(0x4CB782), rgb(0x4B8BF5)});
    std::vector<Color> changes{};
    swatches.onChange([&](Color color) { changes.push_back(color); });
    fixture.layout();

    REQUIRE(is<ColorSwatchesSkinBase>(swatches.skin()));
    const auto chips = parts(swatches, "swatch");
    REQUIRE(chips.size() == 3);
    CHECK(swatches.selected() == -1);
    CHECK(swatches.value() == Color{});
    CHECK(chips[0]->getWidth() == fixture.theme.metrics.pointer.controlXs);
    CHECK(chips[1]->x() - chips[0]->x() == fixture.theme.metrics.pointer.controlXs + 4);
    CHECK(chips[2]->computedStyle().backgroundColor() == rgb(0x4B8BF5));
    CHECK(chips[0]->computedStyle().outlineWidth() == 0.0f);

    // Tab: the keyboard's focus on the first chip (none is selected), ringed in grey; nothing selected
    fixture.tab();
    REQUIRE(swatches.hasFocus());
    CHECK(swatches.focusedSwatch() == 0);
    CHECK(chips[0]->classNames().contains("focused"));
    CHECK(chips[0]->computedStyle().outlineColor() == fixture.theme.palette.textMuted);
    CHECK(chips[0]->computedStyle().outlineWidth() == fixture.theme.metrics.focusRing);
    CHECK(swatches.selected() == -1);

    // The arrows move the focus, not the selection; Space or Enter selects
    fixture.key(Key::RIGHT);
    CHECK(swatches.focusedSwatch() == 1);
    CHECK(swatches.selected() == -1);
    CHECK_FALSE(chips[0]->classNames().contains("focused"));
    CHECK(chips[0]->computedStyle().outlineWidth() == 0.0f);
    CHECK(chips[1]->computedStyle().outlineColor() == fixture.theme.palette.textMuted);
    fixture.key(Key::SPACE);
    CHECK(swatches.selected() == 1);
    CHECK(chips[1]->computedStyle().outlineColor() == fixture.theme.palette.accent); // Focused and selected: the accent
    fixture.key(Key::END);
    CHECK(swatches.focusedSwatch() == 2);
    CHECK(swatches.selected() == 1);
    // The selected chip keeps its accent ring, the focused one its grey one
    CHECK(chips[1]->computedStyle().outlineColor() == fixture.theme.palette.accent);
    CHECK(chips[1]->computedStyle().outlineWidth() == 2.0f);
    CHECK(chips[2]->computedStyle().outlineColor() == fixture.theme.palette.textMuted);
    fixture.key(Key::ENTER);
    CHECK(swatches.selected() == 2);
    fixture.key(Key::HOME);
    fixture.key(Key::UP);
    CHECK(swatches.focusedSwatch() == 0);
    CHECK(swatches.selected() == 2);

    // Leaving and coming back: the focus starts on the selection again
    fixture.column.add<Button>("After");
    fixture.layout();
    fixture.tab();
    CHECK_FALSE(swatches.hasFocus());
    CHECK(chips[0]->computedStyle().outlineWidth() == 0.0f); // No ring without the focus
    CHECK(chips[2]->computedStyle().outlineColor() == fixture.theme.palette.accent);
    fixture.key(Key::TAB, Mod{.shift = true});
    REQUIRE(swatches.hasFocus());
    CHECK(swatches.focusedSwatch() == 2);

    // A press selects at once, and the keyboard goes on from there
    fixture.click(*chips[1]);
    CHECK(swatches.selected() == 1);
    CHECK(swatches.value() == rgb(0x4CB782));
    CHECK(chips[1]->classNames().contains("selected"));
    CHECK(swatches.focusedSwatch() == 1);
    CHECK(chips[1]->computedStyle().outlineColor() == fixture.theme.palette.accent);
    CHECK(chips[1]->computedStyle().outlineWidth() == 2.0f);

    // Between two chips: the nearest
    const auto [x, y] = Inputs::at(*chips[2], -1, chips[2]->getHeight() / 2);
    fixture.press(x, y);
    fixture.release(x, y);
    CHECK(swatches.selected() == 2);
    CHECK(changes == std::vector{rgb(0x4CB782), rgb(0x4B8BF5), rgb(0x4CB782), rgb(0x4B8BF5)});

    swatches.value(rgb(0x123456));
    CHECK(swatches.selected() == -1);
    swatches.value(rgb(0x4CB782));
    CHECK(swatches.selected() == 1);
    swatches.colors({rgb(0x4CB782)});
    fixture.layout();
    CHECK(swatches.selected() == 0); // Still there
    swatches.colors({rgb(0x000000)});
    CHECK(swatches.selected() == -1);

    CHECK(ColorSwatches{}.colors().size() == ColorSwatches::pivotalPalette().size());
}

// endregion

TEST_CASE("Inputs: by touch, they look the same and take presses from around them", "[components][inputs][touch]") {
    Inputs fixture;
    auto                    &search = fixture.column.add<SearchField>();
    auto                    &toggle = fixture.column.add<Switch>("On");
    const std::vector<Div *> controls{
        &search,
        &toggle,
        &fixture.column.add<SegmentedControl>(std::vector<Segment>{{"A"}, {"B"}}),
        &fixture.column.add<NumberStepper>(),
        &fixture.column.add<ColorSwatches>(),
    };
    fixture.layout();
    std::vector<int> sizes{};
    for (const Div *control: controls) {
        CHECK(control->computedStyle().hitSlop() == 0.0f);
        sizes.push_back(control->getWidth());
        sizes.push_back(control->getHeight());
    }

    fixture.window->handleInputModeChange(InputMode::Touch);
    fixture.layout();
    std::vector<int> touchSizes{};
    for (const Div *control: controls) {
        CHECK(control->computedStyle().hitSlop() == fixture.theme.metrics.touch.hitSlop);
        touchSizes.push_back(control->getWidth());
        touchSizes.push_back(control->getHeight());
    }
    CHECK(touchSizes == sizes);

    // A press just outside the switch reaches it
    const auto [x, y] = Inputs::at(toggle, toggle.getWidth() / 2, toggle.getHeight() + 4);
    fixture.press(x, y);
    fixture.release(x, y);
    CHECK(toggle.checked());
}
