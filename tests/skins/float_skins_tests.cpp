#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/AppShell.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/skins/default/DefaultButtonSkin.hpp>
#include <psychic-ui/skins/float/FloatSkins.hpp>
#include <psychic-ui/themes/desk.hpp>
#include <psychic-ui/themes/float.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/themes/psychic.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct Fixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 1000, 700};

        void layout() { systemWindow.layout(); }

        std::string_view theme() const { return window->theme().name; }

        ~Fixture() {
            window->skinSet<DefaultSkins>(); // The style manager is shared with the other tests
        }
    };
}

TEST_CASE("Float's themes", "[skins][float]") {
    CHECK(themes::floatLight.scheme == ColorScheme::Light);
    CHECK(themes::floatDark.scheme == ColorScheme::Dark);
    CHECK(themes::floatLight.palette.accent == rgb(0x7357FF));
    CHECK(themes::floatLight.palette.frame == rgb(0xEDEDF0)); // The canvas
    CHECK(themes::floatLight.metrics.radius.button == 7);     // No pills
    CHECK(themes::floatDark.elevation.panel.color.alpha() > 0);

    FloatSkins set;
    CHECK(&set.theme(ColorScheme::Light) == &themes::floatLight);
    CHECK(&set.theme(ColorScheme::Dark) == &themes::floatDark);
    CHECK(&set.theme(ColorScheme::Unknown) == &themes::floatLight); // Light first
    REQUIRE(set.themes().size() == 2);
    CHECK(set.hasTheme(themes::floatDark));
    CHECK_FALSE(set.hasTheme(themes::pivotalDark));

    // Every named theme has its canvas's dots; Float's are its hairlines, strong in light
    for (const Theme *named: {&themes::pivotalDark, &themes::pivotalLight, &themes::oneDark, &themes::oneLight,
                              &themes::psychic, &themes::floatLight, &themes::floatDark, &themes::deskDark,
                              &themes::deskLight}) {
        INFO(named->name);
        CHECK(named->palette.canvasDot.alpha() > 0);
    }
    CHECK(themes::floatLight.palette.canvasDot == themes::floatLight.palette.lineStrong);
    CHECK(themes::floatDark.palette.canvasDot == themes::floatDark.palette.line);

    // The default set keeps Pivotal Dark when the system doesn't say, and lists its other themes
    DefaultSkins defaults;
    CHECK(&defaults.theme(ColorScheme::Unknown) == &themes::pivotalDark);
    CHECK(&defaults.theme(ColorScheme::Dark) == &themes::pivotalDark);
    CHECK(defaults.hasTheme(themes::oneLight));
}

TEST_CASE("Float installs over the default set", "[skins][float]") {
    Fixture f;
    auto &shell  = f.window->appContainer().add<AppShell>();
    auto &button = shell.page().add<Button>("Go").skin("primary");
    f.window->skinSet<FloatSkins>();
    f.layout();

    REQUIRE(f.window->styleManager()->skinSet()->name() == "Float");
    CHECK(f.theme() == themes::floatLight.name);

    // The default set's skins, Float's rules over the default set's
    CHECK(std::dynamic_pointer_cast<DefaultButtonSkin>(button.skin()) != nullptr);
    CHECK(button.computedStyle().borderRadius() == 7);
    CHECK(button.computedStyle().height() == Length::px(30));

    // The canvas: dotted, the sidebar and the page floating over it
    const Theme &theme = themes::floatLight;
    CHECK(shell.computedStyle().backgroundColor() == theme.palette.frame);
    CHECK(shell.computedStyle().dotGridColor() == theme.palette.canvasDot);
    CHECK(shell.sidebar().computedStyle().shadowColor() == theme.elevation.panel.color);
    CHECK(shell.sidebar().computedStyle().shadow2Spread() == 1);
    CHECK(shell.page().computedStyle().shadowColor() == theme.elevation.panel.color);
    CHECK(shell.page().computedStyle().borderRadius() == theme.metrics.radius.page);
    CHECK(shell.sidebar().getWidth() == theme.metrics.sidebarWidth);
    CHECK(shell.sidebar().x() == theme.metrics.frameInset);

    // The dots are the theme's, dark too
    f.window->theme(themes::floatDark);
    f.layout();
    CHECK(shell.computedStyle().dotGridColor() == themes::floatDark.palette.canvasDot);
}

TEST_CASE("switching to Float live re-hatches skins and keeps the components' state", "[skins][float][switch]") {
    Fixture f;
    auto &app   = f.window->appContainer();
    auto &input = app.add<TextInput>("Typed before");
    auto &list  = app.add<List>();
    list.row("Ada");
    auto &grace = list.row("Grace");
    auto &on    = app.add<Switch>("On").checked(true);
    f.layout();
    list.select(&grace);
    f.layout();
    const auto inputSkin = input.skin();
    const auto switchSkin = on.skin();

    f.window->skinSet<FloatSkins>();
    f.layout();

    CHECK(input.skin() != inputSkin);
    CHECK(on.skin() != switchSkin);
    CHECK(input.text() == "Typed before");
    CHECK(input.textDisplay()->parent() == input.skin().get());
    CHECK(list.selection() == &grace);
    CHECK(grace.computedStyle().backgroundColor() == themes::floatLight.palette.accentSoft);
    CHECK(on.checked());

    // And back
    f.window->skinSet<DefaultSkins>();
    f.layout();
    CHECK(input.text() == "Typed before");
    CHECK(list.selection() == &grace);
    CHECK(on.checked());
}

TEST_CASE("which theme a skin set switch picks", "[skins][theme]") {
    Fixture f;

    SECTION("following the system, the new set's theme for the scheme") {
        f.window->skinSet<FloatSkins>();
        CHECK(f.theme() == "Float Light"); // The system doesn't say: the set's default
        f.window->handleColorSchemeChange(ColorScheme::Dark);
        CHECK(f.theme() == "Float Dark");
        f.window->skinSet<DefaultSkins>();
        CHECK(f.theme() == "Pivotal Dark");
        f.window->handleColorSchemeChange(ColorScheme::Light);
        CHECK(f.theme() == "Pivotal Light");
        f.window->skinSet<FloatSkins>();
        CHECK(f.theme() == "Float Light");
        CHECK(f.window->followsSystemTheme());
    }

    SECTION("a picked theme of another set gives way to the new set's of the same scheme, still picked") {
        f.window->theme(themes::oneLight);
        REQUIRE_FALSE(f.window->followsSystemTheme());
        f.window->handleColorSchemeChange(ColorScheme::Dark); // Ignored: the theme is picked
        f.window->skinSet<FloatSkins>();
        CHECK(f.theme() == "Float Light"); // One Light is light, whatever the system
        CHECK_FALSE(f.window->followsSystemTheme());

        f.window->theme(themes::floatDark);
        f.window->skinSet<DefaultSkins>();
        CHECK(f.theme() == "Pivotal Dark");
        CHECK_FALSE(f.window->followsSystemTheme());

        f.window->theme(themes::pivotalLight);
        f.window->skinSet<FloatSkins>();
        CHECK(f.theme() == "Float Light");
        f.window->handleColorSchemeChange(ColorScheme::Light);
        f.window->handleColorSchemeChange(ColorScheme::Dark);
        CHECK(f.theme() == "Float Light"); // Still picked
    }

    SECTION("a picked theme of the new set stays") {
        f.window->theme(themes::floatDark); // Any theme works with any set
        f.window->skinSet<FloatSkins>();
        CHECK(f.theme() == "Float Dark");
        CHECK_FALSE(f.window->followsSystemTheme());
        f.window->handleColorSchemeChange(ColorScheme::Light); // Still picked: the system doesn't change it
        CHECK(f.theme() == "Float Dark");

        // Installing the same set again keeps it too
        f.window->skinSet<FloatSkins>();
        CHECK(f.theme() == "Float Dark");

        // One Dark is the default set's: it stays across a switch back
        f.window->theme(themes::oneDark);
        f.window->skinSet<DefaultSkins>();
        CHECK(f.theme() == "One Dark");
        CHECK_FALSE(f.window->followsSystemTheme());
    }

    SECTION("following the system with a pair: it stays only if both are the new set's, else the new set's own") {
        f.window->followSystemTheme(themes::oneLight, themes::oneDark); // Both the default set's
        f.window->skinSet<DefaultSkins>();
        CHECK(f.theme() == "One Dark");
        f.window->skinSet<FloatSkins>();
        CHECK(f.theme() == "Float Light");
        f.window->handleColorSchemeChange(ColorScheme::Dark);
        CHECK(f.theme() == "Float Dark");
        CHECK(f.window->followsSystemTheme());

        // A pair of the new set's stays
        f.window->followSystemTheme(themes::floatLight, themes::floatDark);
        f.window->skinSet<FloatSkins>();
        CHECK(f.theme() == "Float Dark");
        f.window->handleColorSchemeChange(ColorScheme::Light);
        CHECK(f.theme() == "Float Light");
    }
}
