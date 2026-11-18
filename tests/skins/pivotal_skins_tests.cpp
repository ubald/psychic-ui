#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/ToolBar.hpp>
#include <psychic-ui/components/WindowControls.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Fixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window};
        const Theme                 &theme{themes::pivotalDark};

        void layout() { systemWindow.layout(); }

        void click(Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y, div.getWidth() / 2, div.getHeight() / 2);
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true});
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false});
            layout();
        }

        void tab(bool shift = false) {
            window->handleKeyPress(Key::TAB, Mod{.shift = shift});
            layout();
        }
    };

    /**
     * A skin's part, by its class
     */
    Div *part(Div &root, const std::string &className) {
        if (root.classNames().contains(className)) {
            return &root;
        }
        for (const auto &child: root.children()) {
            // NOLINTNEXTLINE(misc-const-correctness): returned as a mutable pointer, which the check misses
            if (Div *found = part(*child, className)) {
                return found;
            }
        }
        return nullptr;
    }

    float centerX(const Div &div) {
        int x = 0;
        int y = 0;
        div.localToGlobal(x, y);
        return static_cast<float>(x) + (static_cast<float>(div.getWidth()) / 2.0f);
    }

    float left(const Div &div) {
        int x = 0;
        int y = 0;
        div.localToGlobal(x, y);
        return static_cast<float>(x);
    }
}

TEST_CASE("Pivotal skins: the slider's track, fill and thumb follow the value", "[skins][pivotal][range]") {
    Fixture f;
    auto &range = f.window->appContainer().add<Range<float>>(0.0f, 1.0f, 0.25f, nullptr).width(300);
    f.layout();
    REQUIRE(range.skin());
    const Div *track = part(*range.skin(), "track");
    const Div *fill  = part(*range.skin(), "fill");
    const Div *thumb = part(*range.skin(), "thumb");
    const Div *value = part(*range.skin(), "value");
    REQUIRE(track);
    REQUIRE(fill);
    REQUIRE(thumb);
    REQUIRE(value);

    // Pivotal's: a 4 px track in the strong line color, the accent's fill, a 14 px thumb, the value after them
    CHECK(track->getHeight() == 4);
    CHECK(track->computedStyle().backgroundColor() == f.theme.palette.lineStrong);
    CHECK(fill->computedStyle().backgroundColor() == f.theme.palette.accent);
    CHECK(thumb->getWidth() == 14);
    CHECK(thumb->computedStyle().shadowColor() == f.theme.elevation.thumb.color);
    CHECK(left(*value) > left(*track) + track->getWidth() - 1);

    CHECK(fill->getWidth() == Catch::Approx(track->getWidth() * 0.25f).margin(0.5f));
    CHECK(left(*fill) == Catch::Approx(left(*track)).margin(0.5f));
    CHECK(centerX(*thumb) == Catch::Approx(left(*track) + (track->getWidth() * 0.25f)).margin(1.0f));

    range.value(0.75f);
    f.layout();
    CHECK(fill->getWidth() == Catch::Approx(track->getWidth() * 0.75f).margin(0.5f));
    CHECK(centerX(*thumb) == Catch::Approx(left(*track) + (track->getWidth() * 0.75f)).margin(1.0f));

    // A press picks the value under it along the track
    int x = 0;
    int y = 0;
    track->localToGlobal(x, y, track->getWidth() / 2, 2);
    f.window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true});
    f.window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false});
    CHECK(range.value() == Catch::Approx(0.5f).margin(0.01f));
}

TEST_CASE("Pivotal skins: a vertical slider fills from the bottom", "[skins][pivotal][range]") {
    Fixture f;
    auto &range = f.window->appContainer().add<Range<float>>(0.0f, 1.0f, 0.25f, nullptr)
                   .orientation(Orientation::Vertical)
                   .height(200);
    f.layout();
    const Div *track = part(*range.skin(), "track");
    const Div *fill  = part(*range.skin(), "fill");
    REQUIRE(track);
    REQUIRE(fill);
    CHECK(range.skin()->classNames().contains("vertical"));
    CHECK(track->getWidth() == 4);
    CHECK(fill->getHeight() == Catch::Approx(track->getHeight() * 0.25f).margin(0.5f));
    CHECK(fill->y() + fill->getHeight() == Catch::Approx(track->getHeight()).margin(0.5f));
}

TEST_CASE("Pivotal skins: the keyboard's focus shows the focus ring, a click's doesn't", "[skins][pivotal][focus]") {
    Fixture f;
    auto &first  = f.window->appContainer().add<Button>("First");
    auto &check  = f.window->appContainer().add<CheckBox>("Check");
    auto &range  = f.window->appContainer().add<Range<float>>();
    auto &hidden = f.window->appContainer().add<Button>("Hidden").visible(false);
    auto &last   = f.window->appContainer().add<Button>("Last").enabled(false);
    f.layout();
    CHECK(first.computedStyle().outlineWidth() == 0.0f);

    // Tab goes through what takes the focus, in order, skipping what is hidden or disabled, and wraps around
    f.tab();
    CHECK(first.hasFocus());
    CHECK(f.window->focusVisible());
    CHECK(first.computedStyle().outlineWidth() == f.theme.metrics.focusRing);
    CHECK(first.computedStyle().outlineOffset() == f.theme.metrics.focusRingOffset);
    CHECK(first.computedStyle().outlineColor() == f.theme.palette.accent);

    f.tab();
    CHECK(check.hasFocus());
    CHECK(first.computedStyle().outlineWidth() == 0.0f);
    const Div *box = part(*check.skin(), "box");
    REQUIRE(box);
    CHECK(box->computedStyle().outlineWidth() == f.theme.metrics.focusRing); // On the box

    f.tab();
    CHECK(range.hasFocus());
    const Div *thumb = part(*range.skin(), "thumb");
    REQUIRE(thumb);
    CHECK(thumb->computedStyle().outlineWidth() == f.theme.metrics.focusRing); // Around the thumb

    f.tab();
    CHECK(first.hasFocus()); // Wrapped, past the hidden and the disabled ones
    CHECK_FALSE(hidden.hasFocus());
    CHECK_FALSE(last.hasFocus());

    f.tab(true);
    CHECK(range.hasFocus());

    // The keyboard works them
    const float before = range.getLinearPercentage();
    f.window->handleKeyPress(Key::RIGHT, Mod{});
    CHECK(range.getLinearPercentage() == Catch::Approx(before + 0.01f));
    f.tab(true);
    CHECK(check.hasFocus());
    f.window->handleKeyPress(Key::SPACE, Mod{});
    CHECK(check.checked());
    int clicks = 0;
    first.onClick([&]() { ++clicks; });
    f.tab(true);
    f.window->handleKeyPress(Key::ENTER, Mod{});
    CHECK(clicks == 1);

    // A click focuses without the ring
    f.click(first);
    CHECK(first.hasFocus());
    CHECK_FALSE(f.window->focusVisible());
    CHECK(first.computedStyle().outlineWidth() == 0.0f);
}

TEST_CASE("Pivotal skins: a text field shows its focus with the accent's border", "[skins][pivotal][focus]") {
    Fixture f;
    auto &input = f.window->appContainer().add<TextInput>("Text");
    f.layout();
    CHECK(input.computedStyle().borderColor() == f.theme.palette.line);
    f.click(input);
    CHECK(input.hasFocus());
    CHECK(input.computedStyle().borderColor() == f.theme.palette.accent);
    CHECK(input.computedStyle().outlineWidth() == 0.0f);
}

TEST_CASE("Pivotal skins: buttons are pills, their variants skins", "[skins][pivotal][button]") {
    Fixture f;
    auto &secondary = f.window->appContainer().add<Button>("Secondary");
    auto &primary   = f.window->appContainer().add<Button>("Primary").skin("primary");
    auto &ghost     = f.window->appContainer().add<Button>("Ghost").skin("ghost");
    auto &withIcon  = f.window->appContainer().add<Button>("Add").icon("plus");
    f.layout();

    CHECK(secondary.getHeight() == f.theme.metrics.pointer.controlMd);
    CHECK(secondary.skin()->getHeight() == secondary.getHeight()); // The skin fills its button
    CHECK(secondary.computedStyle().borderRadius() >= secondary.getHeight() / 2);
    CHECK(secondary.skin()->computedStyle().backgroundColor() == f.theme.palette.raised);
    CHECK(primary.skin()->classNames().contains("primary"));
    CHECK(primary.skin()->computedStyle().backgroundColor() == f.theme.palette.accent);
    CHECK(primary.skin()->computedStyle().color() == f.theme.palette.onAccent);
    CHECK(ghost.skin()->computedStyle().backgroundColor().alpha() == 0);

    // Toggled (`:active`, as pressed): held as a hover, filled with the strong hairline's color in its outline
    secondary.toggle(true).selected(true);
    primary.toggle(true).selected(true);
    f.layout();
    CHECK(secondary.skin()->computedStyle().backgroundColor() == f.theme.palette.lineStrong);
    CHECK(secondary.skin()->computedStyle().borderColor() == f.theme.palette.lineStrong);
    CHECK(primary.skin()->computedStyle().backgroundColor() == f.theme.palette.accentHover); // Its variant's own
    secondary.selected(false);
    f.layout();
    CHECK(secondary.skin()->computedStyle().backgroundColor() == f.theme.palette.raised);
    CHECK(secondary.skin()->computedStyle().borderColor() == f.theme.palette.line);

    const Div *icon = part(*withIcon.skin(), "icon");
    REQUIRE(icon);
    CHECK(icon->computedStyle().display() == Display::Flex);
    CHECK(icon->getWidth() == f.theme.metrics.iconSm);
    CHECK(part(*secondary.skin(), "icon")->computedStyle().display() == Display::None);

    // An icon alone, on a square
    auto &iconButton = f.window->appContainer().add<Button>().icon("more").addClassName("icon-button");
    f.layout();
    CHECK(iconButton.getWidth() == f.theme.metrics.pointer.controlSm);
    CHECK(iconButton.getHeight() == f.theme.metrics.pointer.controlSm);
    const Div *alone = part(*iconButton.skin(), "icon");
    REQUIRE(alone);
    CHECK(alone->getWidth() == f.theme.metrics.iconMd);
    CHECK(alone->getHeight() == f.theme.metrics.iconMd);
    CHECK(part(*iconButton.skin(), "label")->computedStyle().display() == Display::None);
}

TEST_CASE("Pivotal skins: a checked check box fills its box with the accent", "[skins][pivotal][checkbox]") {
    Fixture f;
    auto &check = f.window->appContainer().add<CheckBox>("Check");
    f.layout();
    const Div *box = part(*check.skin(), "box");
    REQUIRE(box);
    CHECK(box->getWidth() == 14);
    CHECK(box->computedStyle().backgroundColor() == f.theme.palette.page);
    f.click(check);
    CHECK(check.checked());
    CHECK(check.classNames().contains("checked"));
    CHECK(box->computedStyle().backgroundColor() == f.theme.palette.accent);
}

TEST_CASE("Pivotal skins: menus float on the theme's two-layer shadow", "[skins][pivotal][menu]") {
    Fixture f;
    f.window->openMenu({Menu::item("Copy", []() {}, "Ctrl+C", "copy")}, 10, 10);
    f.layout();
    REQUIRE(f.window->overlay().children().size() == 1);
    const Style &menu = f.window->overlay().children().front()->computedStyle();
    CHECK(menu.backgroundColor() == f.theme.palette.overlay);
    CHECK(menu.borderRadius() == f.theme.metrics.radius.popup);
    CHECK(menu.shadowColor() == f.theme.elevation.overlay.color);
    CHECK(menu.shadowSpread() == f.theme.elevation.overlay.spread);
    CHECK(menu.shadow2Color() == f.theme.elevation.overlayContact.color);
    CHECK(menu.shadow2Blur() == f.theme.elevation.overlayContact.blur);
}

TEST_CASE("Pivotal skins: the window controls are grey until the pointer is over them", "[skins][pivotal][window-controls]") {
    Fixture f;
    auto &controls = f.window->appContainer().add<WindowControls>();
    f.layout();
    Button *dots[] = {&controls.closeButton(), &controls.minimizeButton(), &controls.maximizeButton()};
    const Palette &p = f.theme.palette;

    const auto colors = [&]() {
        return std::vector{
            dots[0]->computedStyle().backgroundColor(),
            dots[1]->computedStyle().backgroundColor(),
            dots[2]->computedStyle().backgroundColor()
        };
    };
    const auto moveTo = [&](int x, int y) {
        f.window->handleMouseMove({.x = static_cast<float>(x), .y = static_cast<float>(y)}, false);
        f.layout();
    };
    const std::vector grey{p.lineStrong, p.lineStrong, p.lineStrong};
    const std::vector lit{p.danger, p.warning, p.success};

    CHECK(colors() == grey);

    // Over one of them, all three light up, like macOS's
    int x = 0;
    int y = 0;
    dots[2]->localToGlobal(x, y, dots[2]->getWidth() / 2, dots[2]->getHeight() / 2);
    moveTo(x, y);
    CHECK(colors() == lit);

    // Between two of them, still over the group
    dots[1]->localToGlobal(x, y, -2, dots[1]->getHeight() / 2);
    moveTo(x, y);
    CHECK_FALSE(dots[0]->mouseOver());
    CHECK_FALSE(dots[1]->mouseOver());
    CHECK(colors() == lit);

    // Away from them
    moveTo(x, y + 100);
    CHECK(colors() == grey);
}

TEST_CASE("Pivotal skins: a menu's labels line up when some rows have an icon", "[skins][pivotal][menu]") {
    Fixture f;

    const auto labelsLeft = [&](const std::vector<std::shared_ptr<MenuItem>> &items) {
        f.window->openMenu(items, 10, 10);
        f.layout();
        std::vector<float> lefts{};
        REQUIRE(f.window->overlay().children().size() == 1);
        const Div &menu = *f.window->overlay().children().front();
        for (const auto &row: menu.children()) {
            const Div *label = part(*row, "label");
            REQUIRE(label);
            lefts.push_back(left(*label) - left(*row));
        }
        f.window->closeMenu();
        f.layout();
        return lefts;
    };

    const std::vector<float> mixed = labelsLeft({
        Menu::item("Copy", []() {}, "Ctrl+C", "copy"),
        Menu::item("Save As…", []() {}, "Ctrl+Shift+S"),
        Menu::item("More", {Menu::item("Nothing", []() {})})
    });
    REQUIRE(mixed.size() == 3);
    CHECK(mixed[1] == mixed[0]);
    CHECK(mixed[2] == mixed[0]);

    // Without icons, no column: the labels start at the row's padding
    const std::vector<float> plain = labelsLeft({
        Menu::item("Save", []() {}, "Ctrl+S"),
        Menu::item("Save As…", []() {}, "Ctrl+Shift+S")
    });
    REQUIRE(plain.size() == 2);
    CHECK(plain[0] == plain[1]);
    CHECK(plain[0] == f.theme.metrics.space(2));
    CHECK(mixed[0] == plain[0] + f.theme.metrics.iconMd + f.theme.metrics.space(2));

    // Only a submenu's row shows its arrow
    f.window->openMenu({Menu::item("Copy", []() {}), Menu::item("More", {Menu::item("Nothing", []() {})})}, 10, 10);
    f.layout();
    const Div &menu = *f.window->overlay().children().front();
    REQUIRE(menu.children().size() == 2);
    CHECK(part(*menu.children()[0], "arrow")->computedStyle().display() == Display::None);
    CHECK(part(*menu.children()[1], "arrow")->computedStyle().display() == Display::Flex);
    f.window->closeMenu();
}

TEST_CASE("Pivotal skins: a tool bar's current tool is in the accent", "[skins][pivotal][toolbar]") {
    Fixture f;
    auto &bar     = f.window->appContainer().add<ToolBar>();
    auto &current = bar.add<Button>().icon("plus").toggle(true).autoToggle(false).selected(true);
    auto &other   = bar.add<Button>().icon("search").toggle(true).autoToggle(false);
    auto &action  = bar.add<Button>("Run");
    f.layout();
    CHECK(current.skin()->computedStyle().backgroundColor() == f.theme.palette.accent);
    CHECK(current.skin()->computedStyle().color() == f.theme.palette.onAccent);
    CHECK(other.skin()->computedStyle().backgroundColor().alpha() == 0); // Quiet at rest
    CHECK(action.skin()->computedStyle().backgroundColor().alpha() == 0);

    // Outside a tool bar, a toggled quiet button is only raised
    auto &quiet = f.window->appContainer().add<Button>().icon("plus").skin("quiet").toggle(true).selected(true);
    f.layout();
    CHECK(quiet.skin()->computedStyle().backgroundColor() == f.theme.palette.raised);
}
