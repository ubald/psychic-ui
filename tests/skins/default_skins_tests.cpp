#include <memory>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/TextInput.hpp>
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
            window->handleMouseButtonChange(x, y, MouseButton::LEFT, true, Mod{});
            window->handleMouseButtonChange(x, y, MouseButton::LEFT, false, Mod{});
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
        for (auto &child: root.children()) {
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
        return static_cast<float>(x) + div.getWidth() / 2.0f;
    }

    float left(const Div &div) {
        int x = 0;
        int y = 0;
        div.localToGlobal(x, y);
        return static_cast<float>(x);
    }
}

TEST_CASE("Default skins: the slider's track, fill and thumb follow the value", "[skins][default][range]") {
    Fixture f;
    auto &range = f.window->appContainer().add<Range<float>>(0.0f, 1.0f, 0.25f, nullptr).width(300);
    f.layout();
    REQUIRE(range.skin());
    Div *track = part(*range.skin(), "track");
    Div *fill  = part(*range.skin(), "fill");
    Div *thumb = part(*range.skin(), "thumb");
    Div *value = part(*range.skin(), "value");
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
    CHECK(centerX(*thumb) == Catch::Approx(left(*track) + track->getWidth() * 0.25f).margin(1.0f));

    range.value(0.75f);
    f.layout();
    CHECK(fill->getWidth() == Catch::Approx(track->getWidth() * 0.75f).margin(0.5f));
    CHECK(centerX(*thumb) == Catch::Approx(left(*track) + track->getWidth() * 0.75f).margin(1.0f));

    // A press picks the value under it along the track
    int x = 0;
    int y = 0;
    track->localToGlobal(x, y, static_cast<int>(track->getWidth() / 2), 2);
    f.window->handleMouseButtonChange(x, y, MouseButton::LEFT, true, Mod{});
    f.window->handleMouseButtonChange(x, y, MouseButton::LEFT, false, Mod{});
    CHECK(range.value() == Catch::Approx(0.5f).margin(0.01f));
}

TEST_CASE("Default skins: a vertical slider fills from the bottom", "[skins][default][range]") {
    Fixture f;
    auto &range = f.window->appContainer().add<Range<float>>(0.0f, 1.0f, 0.25f, nullptr)
                   .orientation(Orientation::Vertical)
                   .height(200);
    f.layout();
    Div *track = part(*range.skin(), "track");
    Div *fill  = part(*range.skin(), "fill");
    REQUIRE(track);
    REQUIRE(fill);
    CHECK(range.skin()->classNames().contains("vertical"));
    CHECK(track->getWidth() == 4);
    CHECK(fill->getHeight() == Catch::Approx(track->getHeight() * 0.25f).margin(0.5f));
    CHECK(fill->y() + fill->getHeight() == Catch::Approx(track->getHeight()).margin(0.5f));
}

TEST_CASE("Default skins: the keyboard's focus shows the focus ring, a click's doesn't", "[skins][default][focus]") {
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
    Div *box = part(*check.skin(), "box");
    REQUIRE(box);
    CHECK(box->computedStyle().outlineWidth() == f.theme.metrics.focusRing); // On the box

    f.tab();
    CHECK(range.hasFocus());
    Div *thumb = part(*range.skin(), "thumb");
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

TEST_CASE("Default skins: a text field shows its focus with the accent's border", "[skins][default][focus]") {
    Fixture f;
    auto &input = f.window->appContainer().add<TextInput>("Text");
    f.layout();
    CHECK(input.computedStyle().borderColor() == f.theme.palette.line);
    f.click(input);
    CHECK(input.hasFocus());
    CHECK(input.computedStyle().borderColor() == f.theme.palette.accent);
    CHECK(input.computedStyle().outlineWidth() == 0.0f);
}

TEST_CASE("Default skins: buttons are pills, their variants skins", "[skins][default][button]") {
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

    Div *icon = part(*withIcon.skin(), "icon");
    REQUIRE(icon);
    CHECK(icon->computedStyle().display() == Display::Flex);
    CHECK(icon->getWidth() == f.theme.metrics.iconSm);
    CHECK(part(*secondary.skin(), "icon")->computedStyle().display() == Display::None);

    // An icon alone, on a square
    auto &iconButton = f.window->appContainer().add<Button>().icon("more").addClassName("icon-button");
    f.layout();
    CHECK(iconButton.getWidth() == f.theme.metrics.pointer.controlSm);
    CHECK(iconButton.getHeight() == f.theme.metrics.pointer.controlSm);
    Div *alone = part(*iconButton.skin(), "icon");
    REQUIRE(alone);
    CHECK(alone->getWidth() == f.theme.metrics.iconMd);
    CHECK(alone->getHeight() == f.theme.metrics.iconMd);
    CHECK(part(*iconButton.skin(), "label")->computedStyle().display() == Display::None);
}

TEST_CASE("Default skins: a checked check box fills its box with the accent", "[skins][default][checkbox]") {
    Fixture f;
    auto &check = f.window->appContainer().add<CheckBox>("Check");
    f.layout();
    Div *box = part(*check.skin(), "box");
    REQUIRE(box);
    CHECK(box->getWidth() == 14);
    CHECK(box->computedStyle().backgroundColor() == f.theme.palette.page);
    f.click(check);
    CHECK(check.checked());
    CHECK(check.classNames().contains("checked"));
    CHECK(box->computedStyle().backgroundColor() == f.theme.palette.accent);
}

TEST_CASE("Default skins: menus float on the theme's two-layer shadow", "[skins][default][menu]") {
    Fixture f;
    f.window->openMenu({Menu::item("Copy", []() {}, "Ctrl+C", "copy")}, 10, 10);
    f.layout();
    REQUIRE(f.window->menuContainer().children().size() == 1);
    const Style &menu = f.window->menuContainer().children().front()->computedStyle();
    CHECK(menu.backgroundColor() == f.theme.palette.overlay);
    CHECK(menu.borderRadius() == f.theme.metrics.radius.popup);
    CHECK(menu.shadowColor() == f.theme.elevation.overlay.color);
    CHECK(menu.shadowSpread() == f.theme.elevation.overlay.spread);
    CHECK(menu.shadow2Color() == f.theme.elevation.overlayContact.color);
    CHECK(menu.shadow2Blur() == f.theme.elevation.overlayContact.blur);
}
