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
            styleManager.style(".a")->set(padding, 1.0f);
            styleManager.style(".b")->set(padding, 2.0f);
            for (int i = 0; i < extra; ++i) {
                styleManager.style(".unrelated" + std::to_string(i))->set(padding, 9.0f);
            }
            REQUIRE(styleManager.computeStyle(div.get())->get(padding) == 2.0f);
        }

        SECTION(".a declared last") {
            StyleManager styleManager;
            styleManager.style(".b")->set(padding, 2.0f);
            styleManager.style(".a")->set(padding, 1.0f);
            for (int i = 0; i < extra; ++i) {
                styleManager.style(".unrelated" + std::to_string(i))->set(padding, 9.0f);
            }
            REQUIRE(styleManager.computeStyle(div.get())->get(padding) == 1.0f);
        }

        SECTION("unrelated rules declared in between") {
            StyleManager styleManager;
            styleManager.style(".a")->set(padding, 1.0f);
            for (int i = 0; i < extra; ++i) {
                styleManager.style(".unrelated" + std::to_string(i))->set(padding, 9.0f);
            }
            styleManager.style(".b")->set(padding, 2.0f);
            REQUIRE(styleManager.computeStyle(div.get())->get(padding) == 2.0f);
        }
    }
}

TEST_CASE("a heavier rule wins whatever the declaration order", "[style][cascade]") {
    auto div = std::make_shared<Div>();
    div->setClassNames({"a", "b"});

    StyleManager styleManager;
    styleManager.style(".a.b")->set(padding, 3.0f);
    styleManager.style(".a")->set(padding, 1.0f);
    styleManager.style(".b")->set(padding, 2.0f);
    REQUIRE(styleManager.computeStyle(div.get())->get(padding) == 3.0f);
}

TEST_CASE("a rule declared again keeps its place", "[style][cascade]") {
    auto div = std::make_shared<Div>();
    div->setClassNames({"a", "b"});

    StyleManager styleManager;
    styleManager.style(".a")->set(padding, 1.0f);
    styleManager.style(".b")->set(padding, 2.0f);
    styleManager.style(".a")->set(padding, 4.0f);
    REQUIRE(styleManager.computeStyle(div.get())->get(padding) == 2.0f);
}

TEST_CASE("resetting the style manager", "[style][cascade]") {
    auto div = std::make_shared<Div>();
    div->setClassNames({"a", "b"});

    StyleManager styleManager;
    Style        *a = styleManager.style(".a");
    a->set(padding, 1.0f);
    styleManager.style(".b")->set(padding, 2.0f);
    styleManager.reset();

    SECTION("drops every rule") {
        REQUIRE_FALSE(styleManager.computeStyle(div.get())->has(padding));
    }

    SECTION("keeps the styles handed out valid, and out of the cascade until declared again") {
        a->set(padding, 5.0f);
        REQUIRE_FALSE(styleManager.computeStyle(div.get())->has(padding));
        REQUIRE(styleManager.style(".a") == a);
        REQUIRE_FALSE(a->has(padding));
    }

    SECTION("orders the rules declared again by their new declaration") {
        styleManager.style(".b")->set(padding, 2.0f);
        styleManager.style(".a")->set(padding, 1.0f);
        REQUIRE(styleManager.computeStyle(div.get())->get(padding) == 1.0f);
    }
}
