#include <memory>
#include <string>
#include <typeinfo>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <unicode/unistr.h>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/ColorSwatches.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/SearchField.hpp>
#include <psychic-ui/components/SegmentedControl.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/skins/base/ColorSwatchesSkinBase.hpp>
#include <psychic-ui/skins/base/NumberStepperSkinBase.hpp>
#include <psychic-ui/skins/base/SearchFieldSkinBase.hpp>
#include <psychic-ui/skins/base/SegmentedControlSkinBase.hpp>
#include <psychic-ui/skins/base/SwitchSkinBase.hpp>
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
            int wx = 0;
            int wy = 0;
            div.localToGlobal(wx, wy, x, y);
            return {wx, wy};
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
            for (const char c: text) {
                window->handleType(icu::UnicodeString(static_cast<UChar32>(c)));
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
    Inputs f;
    auto &input = f.column.add<TextInput>();
    f.layout();

    f.tab();
    CHECK(input.hasFocus());
    CHECK(f.window->focusedDiv() == &input); // The field itself takes the keys
    f.type("abc");
    CHECK(input.text() == "abc");

    // Tab goes on from the field, not into its text
    auto &other = f.column.add<TextInput>();
    f.layout();
    f.tab();
    CHECK(f.window->focusedDiv() == &other);
    CHECK_FALSE(input.hasFocus());
}

TEST_CASE("TextInput: a placeholder while it is empty", "[components][inputs]") {
    Inputs f;
    auto &input = f.column.add<TextInput>().placeholder("Name");
    f.layout();
    const Label *placeholder = part<Label>(input, "placeholder");
    REQUIRE(placeholder);
    CHECK(placeholder->text() == "Name");
    CHECK(placeholder->visible());
    CHECK(placeholder->computedStyle().color() == f.theme.palette.textMuted);

    input.text("Ada");
    f.layout();
    CHECK_FALSE(placeholder->visible());

    f.tab();
    f.key(Key::END);
    f.key(Key::BACKSPACE);
    f.key(Key::BACKSPACE);
    f.key(Key::BACKSPACE);
    CHECK(input.text().empty());
    CHECK(placeholder->visible());

    // None by default
    auto &bare = f.column.add<TextInput>();
    f.layout();
    CHECK_FALSE(part<Label>(bare, "placeholder")->visible());
}

// endregion

// region Search field

TEST_CASE("SearchField: icon, text, and a clear button while not empty", "[components][inputs][search]") {
    Inputs f;
    auto &search = f.column.add<SearchField>().width(240);
    std::vector<std::string> changes{};
    std::vector<std::string> submits{};
    search.onChange([&](const std::string &text) { changes.push_back(text); });
    search.onSubmit([&](const std::string &text) { submits.push_back(text); });
    f.layout();

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
    CHECK(placeholder->computedStyle().color() == f.theme.palette.textMuted);
    CHECK(search.getHeight() == f.theme.metrics.pointer.controlMd);
    CHECK(search.computedStyle().backgroundColor() == f.theme.palette.raised);
    CHECK(text->computedStyle().border() == 0.0f);
    CHECK(icon->x() < text->parent()->x()); // Before the text, in the skin
    CHECK(text->x() == placeholder->x());    // The placeholder where the text starts
    CHECK_FALSE(clear->visible());
    CHECK(clear->computedStyle().cursor() == Cursor::Arrow);

    // Tab: the field is focused (its accent border) and typing goes to its text
    f.tab();
    CHECK(f.window->focusedDiv() == &search); // The field itself takes the keys
    CHECK(search.computedStyle().borderColor() == f.theme.palette.accent);
    f.type("cue");
    CHECK(search.text() == "cue");
    CHECK(text->text() == "cue");
    CHECK(changes == std::vector<std::string>{"c", "cu", "cue"});
    CHECK(clear->visible());
    CHECK_FALSE(placeholder->visible());

    f.key(Key::ENTER);
    CHECK(submits == std::vector<std::string>{"cue"});
    CHECK(search.text() == "cue");
}

TEST_CASE("SearchField: Escape clears it, once, then leaves the key alone", "[components][inputs][search]") {
    Inputs f;
    auto &search = f.column.add<SearchField>("cue").width(240);
    std::vector<std::string> changes{};
    search.onChange([&](const std::string &text) { changes.push_back(text); });
    f.layout();
    const Div *clear = part<Div>(search, "clear");
    CHECK(clear->visible());

    f.tab();
    CHECK(search.takesEscape());
    f.key(Key::ESCAPE);
    CHECK(search.text().empty());
    CHECK(changes == std::vector<std::string>{""});
    CHECK_FALSE(clear->visible());
    CHECK(part<Label>(search, "placeholder")->visible());
    CHECK_FALSE(search.takesEscape());
    f.key(Key::ESCAPE);
    CHECK(changes.size() == 1);
}

TEST_CASE("SearchField: a press on the clear button clears it, and the field keeps the focus", "[components][inputs][search]") {
    Inputs f;
    auto &search = f.column.add<SearchField>().width(240);
    std::vector<std::string> changes{};
    search.onChange([&](const std::string &text) { changes.push_back(text); });
    f.layout();
    f.tab();
    f.type("fixtures");
    changes.clear();

    const Div *clear = part<Div>(search, "clear");
    REQUIRE(clear->visible());
    const auto [cx, cy] = Inputs::center(*clear);
    int x = 0;
    int y = 0;
    search.skin()->globalToLocal(x, y, cx, cy);
    CHECK(search.skin()->clearAt(static_cast<float>(x), static_cast<float>(y)));
    CHECK_FALSE(search.skin()->clearAt(0, 0));

    f.click(*clear);
    CHECK(search.text().empty());
    CHECK(changes == std::vector<std::string>{""});
    CHECK(search.hasFocus());
    CHECK_FALSE(clear->visible());
    CHECK_FALSE(search.skin()->clearAt(static_cast<float>(x), static_cast<float>(y))); // Hidden: nothing to clear

    // A press elsewhere on the text places the caret, and clears nothing
    search.text("cue");
    f.layout();
    f.click(*part<Text>(search, "text"));
    CHECK(search.text() == "cue");
}

TEST_CASE("SearchField: a press anywhere on it lets one type", "[components][inputs][search]") {
    Inputs f;
    auto &search = f.column.add<SearchField>().width(240);
    f.layout();
    const Div *icon = part<Icon>(search, "icon");
    f.click(*icon);
    CHECK(search.hasFocus());
    CHECK_FALSE(f.window->focusVisible());
    f.type("x");
    CHECK(search.text() == "x");
}

TEST_CASE("SearchField: disabled, it refuses input", "[components][inputs][search]") {
    Inputs f;
    auto &search = f.column.add<SearchField>("cue").width(240).enabled(false);
    std::vector<std::string> changes{};
    std::vector<std::string> submits{};
    search.onChange([&](const std::string &text) { changes.push_back(text); });
    search.onSubmit([&](const std::string &text) { submits.push_back(text); });
    f.layout();

    // A press focuses nothing that takes typing, nor clears it
    f.click(*part<Div>(search, "clear"));
    f.click(search);
    f.type("x");
    CHECK(search.text() == "cue");

    // Even given the focus, the keys do nothing
    f.window->requestFocus(&search, true);
    f.type("x");
    f.key(Key::BACKSPACE);
    f.key(Key::ENTER);
    CHECK_FALSE(search.takesEscape());
    f.key(Key::ESCAPE);
    CHECK(search.text() == "cue");
    CHECK(changes.empty());
    CHECK(submits.empty());
}

// endregion

// region Switch

TEST_CASE("Switch: a click, Space or Enter flips it", "[components][inputs][switch]") {
    Inputs f;
    auto &toggle = f.column.add<Switch>("Follow system");
    std::vector<bool> changes{};
    toggle.onChange([&](bool on) { changes.push_back(on); });
    f.layout();

    REQUIRE(is<SwitchSkinBase>(toggle.skin()));
    const Div *track = part<Div>(toggle, "track");
    const Div *thumb = part<Div>(toggle, "thumb");
    REQUIRE(track);
    REQUIRE(thumb);
    CHECK(track->getWidth() == 30);
    CHECK(track->getHeight() == 18);
    CHECK(thumb->getWidth() == 12);
    CHECK(track->computedStyle().backgroundColor() == f.theme.palette.switchOff);
    CHECK(thumb->x() == 3); // The border and 2 px in

    f.click(toggle);
    CHECK(toggle.checked());
    CHECK(toggle.classNames().contains("checked"));
    CHECK(track->computedStyle().backgroundColor() == f.theme.palette.accent);
    CHECK(thumb->x() == 15); // 12 px along: the end of the travel

    f.tab();
    CHECK(toggle.hasFocus());
    CHECK(track->computedStyle().outlineColor() == f.theme.palette.accent);
    f.key(Key::SPACE);
    CHECK_FALSE(toggle.checked());
    f.key(Key::ENTER);
    CHECK(toggle.checked());
    CHECK(changes == std::vector{true, false, true});

    toggle.enabled(false);
    f.click(toggle);
    f.key(Key::SPACE);
    CHECK(toggle.checked());
}

TEST_CASE("Switch: the thumb drags, and lands on the nearer side", "[components][inputs][switch]") {
    Inputs f;
    auto &toggle = f.column.add<Switch>();
    std::vector<bool> changes{};
    toggle.onChange([&](bool on) { changes.push_back(on); });
    f.layout();
    const Div *thumb = part<Div>(toggle, "thumb");
    auto [x, y] = Inputs::center(*thumb);

    // A little way: back off, no change and no click
    f.press(x, y);
    f.move(x + 4, y);
    CHECK(toggle.dragging());
    CHECK(toggle.thumbPosition() > 0.0f);
    CHECK(toggle.thumbPosition() < 0.5f);
    f.release(x + 4, y);
    CHECK_FALSE(toggle.checked());
    CHECK(changes.empty());
    CHECK(thumb->x() == 3);

    // Past the middle, released outside: on
    f.press(x, y);
    f.move(x + 9, y);
    f.layout();
    CHECK(thumb->x() > 3);
    CHECK(thumb->x() < 15);
    f.move(x + 40, y + 30);
    CHECK(toggle.thumbPosition() == 1.0f); // Clamped
    f.release(x + 40, y + 30);
    CHECK(toggle.checked());
    CHECK(changes == std::vector{true});
    CHECK(thumb->x() == 15);

    // And back from on
    std::tie(x, y) = Inputs::center(*thumb);
    f.press(x, y);
    f.move(x - 12, y);
    f.release(x - 12, y);
    CHECK_FALSE(toggle.checked());
    CHECK(changes == std::vector{true, false});
}

// endregion

// region Segmented control

TEST_CASE("SegmentedControl: one segment selected, by click or arrow", "[components][inputs][segmented]") {
    Inputs f;
    auto &control = f.column.add<SegmentedControl>(
        std::vector<Segment>{{"Day"}, {"Week", "calendar"}, {"", "list"}}
    );
    std::vector<int> changes{};
    control.onChange([&](int index) { changes.push_back(index); });
    f.layout();

    REQUIRE(is<SegmentedControlSkinBase>(control.skin()));
    const auto segments = parts(control, "segment");
    REQUIRE(segments.size() == 3);
    CHECK(control.selected() == 0);
    CHECK(segments[0]->classNames().contains("selected"));
    CHECK(control.getHeight() == f.theme.metrics.pointer.controlMd);
    CHECK(segments[0]->getHeight() == control.getHeight() - 4); // In its border and 1 px padding
    CHECK(segments[0]->computedStyle().backgroundColor() == f.theme.palette.raised);
    CHECK(segments[1]->computedStyle().color() == f.theme.palette.textMuted);
    // Label, icon and label, icon alone
    CHECK(parts(*segments[0], "icon").empty());
    CHECK(parts(*segments[1], "icon").size() == 1);
    CHECK(parts(*segments[2], "label").empty());

    f.click(*segments[1]);
    CHECK(control.selected() == 1);
    CHECK(segments[1]->classNames().contains("selected"));
    CHECK_FALSE(segments[0]->classNames().contains("selected"));
    CHECK(segments[1]->computedStyle().color() == f.theme.palette.text);

    f.tab();
    CHECK(control.hasFocus());
    CHECK(segments[1]->computedStyle().outlineColor() == f.theme.palette.accent);
    f.key(Key::RIGHT);
    CHECK(control.selected() == 2);
    f.key(Key::RIGHT); // At the end
    CHECK(control.selected() == 2);
    f.key(Key::HOME);
    CHECK(control.selected() == 0);
    f.key(Key::LEFT);
    CHECK(control.selected() == 0);
    f.key(Key::END);
    CHECK(changes == std::vector{1, 2, 0, 2});

    control.selected(7);
    CHECK(control.selected() == 2);
    control.segments({{"A"}, {"B"}});
    f.layout();
    CHECK(control.selected() == 0); // 2 is gone
    CHECK(parts(control, "segment").size() == 2);
}

// endregion

// region Number stepper

TEST_CASE("NumberStepper: the buttons step it within its range", "[components][inputs][stepper]") {
    Inputs f;
    auto &stepper = f.column.add<NumberStepper>(2).range(0, 3);
    std::vector<double> changes{};
    stepper.onChange([&](double value) { changes.push_back(value); });
    f.layout();

    REQUIRE(is<NumberStepperSkinBase>(stepper.skin()));
    const Div &minus = *part<Div>(stepper, "decrement");
    const Div &plus  = *part<Div>(stepper, "increment");
    Text &value = *part<Text>(stepper, "value");
    CHECK(&value == stepper.skin()->textView());
    CHECK(stepper.getWidth() == f.theme.metrics.pointer.controlMd * 4);
    CHECK(stepper.getHeight() == f.theme.metrics.pointer.controlMd);
    CHECK(minus.x() < value.x());
    CHECK(value.x() < plus.x());
    CHECK(value.text() == "2");
    CHECK(stepper.text() == "2");
    CHECK(minus.computedStyle().cursor() == Cursor::Arrow);
    CHECK(value.computedStyle().cursor() == Cursor::IBeam);

    // The skin says which button a point is on
    const auto skinPoint = [&](const Div &div) {
        const auto [wx, wy] = Inputs::center(div);
        int x = 0;
        int y = 0;
        stepper.skin()->globalToLocal(x, y, wx, wy);
        return std::pair{static_cast<float>(x), static_cast<float>(y)};
    };
    CHECK(stepper.skin()->partAt(skinPoint(minus).first, skinPoint(minus).second) == NumberStepperSkin::Part::Decrement);
    CHECK(stepper.skin()->partAt(skinPoint(plus).first, skinPoint(plus).second) == NumberStepperSkin::Part::Increment);
    CHECK(stepper.skin()->partAt(skinPoint(value).first, skinPoint(value).second) == NumberStepperSkin::Part::None);

    // Hovered and pressed, through rules
    const auto [px, py] = Inputs::center(plus);
    f.move(px, py);
    f.layout();
    CHECK(plus.computedStyle().backgroundColor() == f.theme.palette.line);

    f.click(plus);
    CHECK(stepper.value() == 3);
    CHECK_FALSE(plus.enabled()); // At the maximum: `:disabled`
    CHECK(plus.computedStyle().color() == f.theme.palette.textMuted);
    f.click(plus);
    CHECK(stepper.value() == 3);
    f.click(minus);
    f.click(minus);
    f.click(minus);
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
    f.layout();
    CHECK_FALSE(minus.enabled());
    CHECK_FALSE(plus.enabled());
    f.click(plus);
    CHECK(stepper.value() == 5);
}

TEST_CASE("NumberStepper: typed, and stepped by the keys", "[components][inputs][stepper]") {
    Inputs f;
    auto &stepper = f.column.add<NumberStepper>(1.5).range(0, 100).step(0.5);
    f.column.add<Switch>(); // Somewhere for Tab to go next
    f.layout();
    CHECK(stepper.text() == "1.5");

    f.tab();
    CHECK(f.window->focusedDiv() == &stepper); // The stepper itself takes the keys
    CHECK(stepper.computedStyle().borderColor() == f.theme.palette.accent);

    f.key(Key::UP);
    CHECK(stepper.value() == 2.0);
    CHECK(stepper.text() == "2.0");
    CHECK(stepper.caret() == 3); // At the end
    f.key(Key::DOWN, Mod{.shift = true});
    CHECK(stepper.value() == 0.0); // Ten steps down, clamped

    // Typing: Enter sets it, clamped, and rounded to the decimals shown
    stepper.text("");
    f.type("42.25");
    CHECK(stepper.value() == 0.0); // Not yet
    f.key(Key::ENTER);
    CHECK(stepper.value() == 42.3);
    CHECK(stepper.text() == "42.3");

    stepper.text("");
    f.type("500");
    f.key(Key::ENTER);
    CHECK(stepper.value() == 100);

    // Not a number: back to the value; Escape gives the typing up
    stepper.text("");
    f.type("abc");
    f.key(Key::ENTER);
    CHECK(stepper.value() == 100);
    CHECK(stepper.text() == "100.0");
    stepper.text("");
    f.type("7");
    CHECK(stepper.takesEscape());
    f.key(Key::ESCAPE);
    CHECK(stepper.text() == "100.0");
    CHECK_FALSE(stepper.takesEscape());

    // The text is edited as any field's: the caret moves, Backspace erases
    f.key(Key::HOME);
    f.key(Key::DELETE);
    CHECK(stepper.text() == "00.0");
    f.key(Key::ESCAPE);

    // Leaving the field sets what was typed
    stepper.text("");
    f.type("8");
    f.tab();
    CHECK(stepper.value() == 8);
    CHECK_FALSE(stepper.hasFocus());
}

TEST_CASE("NumberStepper: the wheel steps it while it has the focus", "[components][inputs][stepper]") {
    Inputs f;
    auto &stepper = f.column.add<NumberStepper>(5).range(0, 10);
    f.layout();
    const auto [x, y] = Inputs::center(stepper);
    const auto scroll = [&](float dy) {
        f.window->handleMouseScroll({.x = static_cast<float>(x), .y = static_cast<float>(y), .scrollY = dy});
        f.layout();
    };
    scroll(1);
    CHECK(stepper.value() == 5); // Not focused: it scrolls what is around it
    f.tab();
    scroll(1);
    CHECK(stepper.value() == 6);
    scroll(-1);
    scroll(-1);
    CHECK(stepper.value() == 4);
}

TEST_CASE("NumberStepper: disabled, it refuses input", "[components][inputs][stepper]") {
    Inputs f;
    auto &stepper = f.column.add<NumberStepper>(5).enabled(false);
    std::vector<double> changes{};
    stepper.onChange([&](double value) { changes.push_back(value); });
    f.layout();
    f.window->requestFocus(&stepper, true);
    f.type("9");
    f.key(Key::UP);
    f.key(Key::ENTER);
    CHECK(stepper.text() == "5");
    CHECK(stepper.value() == 5);
    CHECK(changes.empty());
}

TEST_CASE("NumberStepper: decimal steps don't drift", "[components][inputs][stepper]") {
    Inputs f;
    auto &stepper = f.column.add<NumberStepper>().step(0.1);
    f.layout();
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
    Inputs f;
    auto &swatches = f.column.add<ColorSwatches>(std::vector{rgb(0xE5484D), rgb(0x4CB782), rgb(0x4B8BF5)});
    std::vector<Color> changes{};
    swatches.onChange([&](Color color) { changes.push_back(color); });
    f.layout();

    REQUIRE(is<ColorSwatchesSkinBase>(swatches.skin()));
    const auto chips = parts(swatches, "swatch");
    REQUIRE(chips.size() == 3);
    CHECK(swatches.selected() == -1);
    CHECK(swatches.value() == Color{});
    CHECK(chips[0]->getWidth() == f.theme.metrics.pointer.controlXs);
    CHECK(chips[1]->x() - chips[0]->x() == f.theme.metrics.pointer.controlXs + 4);
    CHECK(chips[2]->computedStyle().backgroundColor() == rgb(0x4B8BF5));
    CHECK(chips[0]->computedStyle().outlineWidth() == 0.0f);

    // Tab: the keyboard's focus on the first chip (none is selected), ringed in grey; nothing selected
    f.tab();
    REQUIRE(swatches.hasFocus());
    CHECK(swatches.focusedSwatch() == 0);
    CHECK(chips[0]->classNames().contains("focused"));
    CHECK(chips[0]->computedStyle().outlineColor() == f.theme.palette.textMuted);
    CHECK(chips[0]->computedStyle().outlineWidth() == f.theme.metrics.focusRing);
    CHECK(swatches.selected() == -1);

    // The arrows move the focus, not the selection; Space or Enter selects
    f.key(Key::RIGHT);
    CHECK(swatches.focusedSwatch() == 1);
    CHECK(swatches.selected() == -1);
    CHECK_FALSE(chips[0]->classNames().contains("focused"));
    CHECK(chips[0]->computedStyle().outlineWidth() == 0.0f);
    CHECK(chips[1]->computedStyle().outlineColor() == f.theme.palette.textMuted);
    f.key(Key::SPACE);
    CHECK(swatches.selected() == 1);
    CHECK(chips[1]->computedStyle().outlineColor() == f.theme.palette.accent); // Focused and selected: the accent
    f.key(Key::END);
    CHECK(swatches.focusedSwatch() == 2);
    CHECK(swatches.selected() == 1);
    // The selected chip keeps its accent ring, the focused one its grey one
    CHECK(chips[1]->computedStyle().outlineColor() == f.theme.palette.accent);
    CHECK(chips[1]->computedStyle().outlineWidth() == 2.0f);
    CHECK(chips[2]->computedStyle().outlineColor() == f.theme.palette.textMuted);
    f.key(Key::ENTER);
    CHECK(swatches.selected() == 2);
    f.key(Key::HOME);
    f.key(Key::UP);
    CHECK(swatches.focusedSwatch() == 0);
    CHECK(swatches.selected() == 2);

    // Leaving and coming back: the focus starts on the selection again
    f.column.add<Button>("After");
    f.layout();
    f.tab();
    CHECK_FALSE(swatches.hasFocus());
    CHECK(chips[0]->computedStyle().outlineWidth() == 0.0f); // No ring without the focus
    CHECK(chips[2]->computedStyle().outlineColor() == f.theme.palette.accent);
    f.key(Key::TAB, Mod{.shift = true});
    REQUIRE(swatches.hasFocus());
    CHECK(swatches.focusedSwatch() == 2);

    // A press selects at once, and the keyboard goes on from there
    f.click(*chips[1]);
    CHECK(swatches.selected() == 1);
    CHECK(swatches.value() == rgb(0x4CB782));
    CHECK(chips[1]->classNames().contains("selected"));
    CHECK(swatches.focusedSwatch() == 1);
    CHECK(chips[1]->computedStyle().outlineColor() == f.theme.palette.accent);
    CHECK(chips[1]->computedStyle().outlineWidth() == 2.0f);

    // Between two chips: the nearest
    const auto [x, y] = Inputs::at(*chips[2], -1, chips[2]->getHeight() / 2);
    f.press(x, y);
    f.release(x, y);
    CHECK(swatches.selected() == 2);
    CHECK(changes == std::vector{rgb(0x4CB782), rgb(0x4B8BF5), rgb(0x4CB782), rgb(0x4B8BF5)});

    swatches.value(rgb(0x123456));
    CHECK(swatches.selected() == -1);
    swatches.value(rgb(0x4CB782));
    CHECK(swatches.selected() == 1);
    swatches.colors({rgb(0x4CB782)});
    f.layout();
    CHECK(swatches.selected() == 0); // Still there
    swatches.colors({rgb(0x000000)});
    CHECK(swatches.selected() == -1);

    CHECK(ColorSwatches{}.colors().size() == ColorSwatches::pivotalPalette().size());
}

// endregion

TEST_CASE("Inputs: by touch, they look the same and take presses from around them", "[components][inputs][touch]") {
    Inputs f;
    auto                    &search = f.column.add<SearchField>();
    auto                    &toggle = f.column.add<Switch>("On");
    const std::vector<Div *> controls{
        &search,
        &toggle,
        &f.column.add<SegmentedControl>(std::vector<Segment>{{"A"}, {"B"}}),
        &f.column.add<NumberStepper>(),
        &f.column.add<ColorSwatches>(),
    };
    f.layout();
    std::vector<int> sizes{};
    for (const Div *control: controls) {
        CHECK(control->computedStyle().hitSlop() == 0.0f);
        sizes.push_back(control->getWidth());
        sizes.push_back(control->getHeight());
    }

    f.window->handleInputModeChange(InputMode::Touch);
    f.layout();
    std::vector<int> touchSizes{};
    for (const Div *control: controls) {
        CHECK(control->computedStyle().hitSlop() == f.theme.metrics.touch.hitSlop);
        touchSizes.push_back(control->getWidth());
        touchSizes.push_back(control->getHeight());
    }
    CHECK(touchSizes == sizes);

    // A press just outside the switch reaches it
    const auto [x, y] = Inputs::at(toggle, toggle.getWidth() / 2, toggle.getHeight() + 4);
    f.press(x, y);
    f.release(x, y);
    CHECK(toggle.checked());
}
