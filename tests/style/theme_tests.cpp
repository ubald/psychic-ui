#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include <psychic-ui/Div.hpp>

using namespace psychic_ui;

namespace {
    std::vector<std::string> loads{};

    /**
     * A look: every `.thing` gets the control color, and a padding from the metrics
     */
    struct LookSheet : StyleSheet {
        void load(StyleManager *manager) override {
            loads.emplace_back("look");
            manager->style(".thing")
                   .backgroundColor(manager->theme().palette.control)
                   .color(manager->theme().palette.text)
                   .padding(manager->theme().metrics.spaceMd);
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

TEST_CASE("a style manager starts with One Dark", "[style][theme]") {
    StyleManager manager;
    REQUIRE(manager.theme().name == themes::oneDark.name);
    REQUIRE(manager.theme().palette.accent == themes::oneDark.palette.accent);
}

TEST_CASE("switching themes loads the stylesheets again, in order, with the new values", "[style][theme]") {
    loads.clear();
    auto div = std::make_shared<Div>();
    div->addClassName("thing");

    StyleManager manager;
    manager.loadStyleSheet<LookSheet>();
    manager.loadStyleSheet<AppSheet>();
    manager.setValid();

    Style dark = manager.computeStyle(div.get());
    REQUIRE(dark.backgroundColor() == themes::oneDark.palette.accent); // The app's rule wins
    REQUIRE(dark.color() == themes::oneDark.palette.text);

    manager.theme(themes::oneLight);
    REQUIRE(loads == std::vector<std::string>{"look", "app", "look", "app"});
    REQUIRE_FALSE(manager.valid());

    Style light = manager.computeStyle(div.get());
    REQUIRE(light.backgroundColor() == themes::oneLight.palette.accent); // Still the app's rule
    REQUIRE(light.color() == themes::oneLight.palette.text);             // The look's, with the new values
    REQUIRE(light.padding() == Length{themes::oneLight.metrics.spaceMd});
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
    constexpr Theme mixed{.name = "Mixed", .palette = themes::oneLight.palette, .metrics = {.radius = 2}};
    StyleManager manager;
    manager.theme(mixed);
    REQUIRE(manager.theme().palette.accent == themes::oneLight.palette.accent);
    REQUIRE(manager.theme().metrics.radius == 2.0f);
    REQUIRE(manager.theme().type.text == 12.0f);
}
