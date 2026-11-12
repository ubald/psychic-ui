#include <memory>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Style.hpp>
#include <psychic-ui/Div.hpp>

using namespace psychic_ui;

TEST_CASE("on equal specificity the rule declared last wins, whatever else is declared", "[style][cascade]") {
    auto div = std::make_shared<Div>();
    div->setClassNames({"a", "b"});

    for (int extra: {0, 1, 5, 20, 100}) {
        CAPTURE(extra);

        SECTION(".b declared last") {
            StyleManager styleManager;
            styleManager.style(".a").padding(1.0f);
            styleManager.style(".b").padding(2.0f);
            for (int i = 0; i < extra; ++i) {
                styleManager.style(".unrelated" + std::to_string(i)).padding(9.0f);
            }
            REQUIRE(styleManager.computeStyle(div.get()).padding() == 2.0f);
        }

        SECTION(".a declared last") {
            StyleManager styleManager;
            styleManager.style(".b").padding(2.0f);
            styleManager.style(".a").padding(1.0f);
            for (int i = 0; i < extra; ++i) {
                styleManager.style(".unrelated" + std::to_string(i)).padding(9.0f);
            }
            REQUIRE(styleManager.computeStyle(div.get()).padding() == 1.0f);
        }

        SECTION("unrelated rules declared in between") {
            StyleManager styleManager;
            styleManager.style(".a").padding(1.0f);
            for (int i = 0; i < extra; ++i) {
                styleManager.style(".unrelated" + std::to_string(i)).padding(9.0f);
            }
            styleManager.style(".b").padding(2.0f);
            REQUIRE(styleManager.computeStyle(div.get()).padding() == 2.0f);
        }
    }
}

TEST_CASE("a heavier rule wins whatever the declaration order", "[style][cascade]") {
    auto div = std::make_shared<Div>();
    div->setClassNames({"a", "b"});

    StyleManager styleManager;
    styleManager.style(".a.b").padding(3.0f);
    styleManager.style(".a").padding(1.0f);
    styleManager.style(".b").padding(2.0f);
    REQUIRE(styleManager.computeStyle(div.get()).padding() == 3.0f);
}

TEST_CASE("a rule declared again keeps its place", "[style][cascade]") {
    auto div = std::make_shared<Div>();
    div->setClassNames({"a", "b"});

    StyleManager styleManager;
    styleManager.style(".a").padding(1.0f);
    styleManager.style(".b").padding(2.0f);
    styleManager.style(".a").padding(4.0f);
    REQUIRE(styleManager.computeStyle(div.get()).padding() == 2.0f);
}

TEST_CASE("resetting the style manager", "[style][cascade]") {
    auto div = std::make_shared<Div>();
    div->setClassNames({"a", "b"});

    StyleManager styleManager;
    Style        &a = styleManager.style(".a");
    a.padding(1.0f);
    styleManager.style(".b").padding(2.0f);
    styleManager.reset();

    SECTION("drops every rule") {
        REQUIRE_FALSE(styleManager.computeStyle(div.get()).has(Property::padding));
    }

    SECTION("keeps the styles handed out valid, and out of the cascade until declared again") {
        a.padding(5.0f);
        REQUIRE_FALSE(styleManager.computeStyle(div.get()).has(Property::padding));
        REQUIRE(&styleManager.style(".a") == &a);
        REQUIRE_FALSE(a.has(Property::padding));
    }

    SECTION("orders the rules declared again by their new declaration") {
        styleManager.style(".b").padding(2.0f);
        styleManager.style(".a").padding(1.0f);
        REQUIRE(styleManager.computeStyle(div.get()).padding() == 1.0f);
    }
}

TEST_CASE("rules are found whatever their rightmost part requires", "[style][cascade]") {
    StyleManager styleManager;
    styleManager.style(":hover").fontSize(1);
    styleManager.style("div").fontSize(2);
    styleManager.style(".a").fontSize(3);
    styleManager.style("div.a.b").fontSize(4);
    styleManager.style("#named").fontSize(5);
    styleManager.style(".a:hover").fontSize(6);

    auto div = std::make_shared<Div>();
    REQUIRE(styleManager.computeStyle(div.get()).fontSize() == 2);

    div->setMouseOver(true);
    REQUIRE(styleManager.computeStyle(div.get()).fontSize() == 2); // div (10) beats :hover (1)

    div->setClassNames({"A"}); // Class names are case insensitive, like selectors
    REQUIRE(styleManager.computeStyle(div.get()).fontSize() == 6);

    div->setMouseOver(false);
    div->addClassName("b");
    REQUIRE(styleManager.computeStyle(div.get()).fontSize() == 4);

    div->setId("Named");
    REQUIRE(styleManager.computeStyle(div.get()).fontSize() == 4); // div.a.b (30) beats #named (15)

    auto other = std::make_shared<Div>();
    other->setId("named");
    REQUIRE(styleManager.computeStyle(other.get()).fontSize() == 5);

    styleManager.style("#" + other->internalId()).fontSize(7);
    REQUIRE(styleManager.computeStyle(other.get()).fontSize() == 7); // Declared last
}
