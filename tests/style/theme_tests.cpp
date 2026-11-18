#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/themes/desk.hpp>
#include <psychic-ui/themes/float.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/themes/psychic.hpp>
#include <psychic-ui/Div.hpp>

using namespace psychic_ui;

namespace {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables): the style manager constructs the sheets that record here
    std::vector<std::string> loads{};

    /**
     * A look: every `.thing` gets the control color, and a padding from the metrics
     */
    struct LookSheet : StyleSheet {
        void load(StyleManager *manager) override {
            loads.emplace_back("look");
            manager->style(".thing")
                   .backgroundColor(manager->theme().palette.raised)
                   .color(manager->theme().palette.text)
                   .padding(manager->theme().metrics.gutter);
        }
    };

    /**
     * An app's sheet, loaded after the look: same selector, so it wins on what it sets
     */
    struct AppSheet : StyleSheet {
        void load(StyleManager *manager) override {
            loads.emplace_back("app");
            manager->style(".thing").backgroundColor(manager->theme().palette.accent);
        }
    };
}

TEST_CASE("a style manager starts with Pivotal Dark", "[style][theme]") {
    const StyleManager manager;
    REQUIRE(manager.theme().name == themes::pivotalDark.name);
    REQUIRE(manager.theme().palette.accent == themes::pivotalDark.palette.accent);
}

TEST_CASE("switching themes loads the stylesheets again, in order, with the new values", "[style][theme]") {
    loads.clear();
    auto div = std::make_shared<Div>();
    div->addClassName("thing");

    StyleManager manager;
    manager.loadStyleSheet<LookSheet>();
    manager.loadStyleSheet<AppSheet>();
    manager.setValid();

    const Style dark = manager.computeStyle(div.get());
    REQUIRE(dark.backgroundColor() == themes::pivotalDark.palette.accent); // The app's rule wins
    REQUIRE(dark.color() == themes::pivotalDark.palette.text);

    manager.theme(themes::oneLight);
    REQUIRE(loads == std::vector<std::string>{"look", "app", "look", "app"});
    REQUIRE_FALSE(manager.valid());

    const Style light = manager.computeStyle(div.get());
    REQUIRE(light.backgroundColor() == themes::oneLight.palette.accent); // Still the app's rule
    REQUIRE(light.color() == themes::oneLight.palette.text);             // The look's, with the new values
    REQUIRE(light.padding() == Length{themes::oneLight.metrics.gutter});
}

TEST_CASE("a theme switch drops rules declared outside the stylesheets", "[style][theme]") {
    auto div = std::make_shared<Div>();
    div->addClassName("thing");

    StyleManager manager;
    manager.loadStyleSheet<LookSheet>();
    manager.style(".thing").margin(5.0f);
    REQUIRE(manager.computeStyle(div.get()).margin() == Length{5.0f});

    manager.theme(themes::oneLight);
    REQUIRE(manager.computeStyle(div.get()).margin() == Length{});
}

TEST_CASE("loading a stylesheet type again runs it in its place", "[style][theme]") {
    loads.clear();
    StyleManager manager;
    manager.loadStyleSheet<LookSheet>();
    manager.loadStyleSheet<AppSheet>();
    manager.loadStyleSheet<LookSheet>();
    REQUIRE(loads == std::vector<std::string>{"look", "app", "look"});

    loads.clear();
    manager.theme(themes::oneDark);
    REQUIRE(loads == std::vector<std::string>{"look", "app"});
}

TEST_CASE("theme groups mix", "[style][theme]") {
    constexpr Theme mixed{.name = "Mixed", .palette = themes::oneLight.palette, .metrics = {.border = 2}};
    StyleManager manager;
    manager.theme(mixed);
    REQUIRE(manager.theme().palette.accent == themes::oneLight.palette.accent);
    REQUIRE(manager.theme().metrics.border == 2.0f);
    REQUIRE(manager.theme().type.sm.size == 12.0f);
}

TEST_CASE("the Pivotal themes are Pivotal's tokens", "[style][theme]") {
    // tokens.css, dark (its default) and light
    CHECK(themes::pivotalDark.scheme == ColorScheme::Dark);
    CHECK(themes::pivotalDark.palette.base == rgb(0x08090A));
    CHECK(themes::pivotalDark.palette.accent == rgb(0x5E6AD2));
    CHECK(themes::pivotalDark.palette.line == argb(0x12FFFFFF)); // rgb(255 255 255 / 7%)
    CHECK(themes::pivotalDark.palette.frame == themes::pivotalDark.palette.surface);
    CHECK(themes::pivotalDark.palette.page == themes::pivotalDark.palette.base);
    CHECK(themes::pivotalLight.scheme == ColorScheme::Light);
    CHECK(themes::pivotalLight.palette.base == rgb(0xFBFBFC));
    CHECK(themes::pivotalLight.palette.overlay == rgb(0xFFFFFF));
    // --shadow-overlay: 0 12px 32px -8px, then 0 2px 6px
    CHECK(themes::pivotalDark.elevation.overlay.spread == -8.0f);
    CHECK(themes::pivotalDark.elevation.overlayContact.blur == 6.0f);
    // The scales: body text 12/18, rows 32, pill buttons
    const Theme &theme = themes::pivotalDark;
    CHECK(theme.type.regular.name == "Inter");
    CHECK(theme.type.sm.size == 12.0f);
    CHECK(theme.type.sm.leading == 18.0f);
    CHECK(theme.metrics.pointer.rowSm == 32.0f);
    CHECK(theme.metrics.space(2.5f) == 10.0f);
    CHECK(theme.metrics.radius.button > theme.metrics.pointer.controlMd);
}

TEST_CASE("every named theme has a panel elevation, lower than its overlays'", "[style][theme]") {
    for (const Theme *named: {&themes::pivotalDark, &themes::pivotalLight, &themes::oneDark, &themes::oneLight,
                              &themes::psychic, &themes::floatLight, &themes::floatDark,
                              &themes::deskDark, &themes::deskLight}) {
        INFO(named->name);
        const Elevation &e = named->elevation;
        CHECK(e.panel.color.alpha() > 0);
        CHECK(e.panelContact.color.alpha() > 0);
        CHECK(e.panel.blur <= e.overlay.blur);
        CHECK(e.panel.offsetY <= e.overlay.offsetY);
    }
}
