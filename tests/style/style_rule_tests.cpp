#include "catch.hpp"
#include <psychicui/style/StyleRule.hpp>

using namespace psychicui;

TEST_CASE("rules from selectors", "[style]") {

    SECTION("invalids") {

        SECTION("empty rule") {
            auto rule = Rule::fromSelector("");
            REQUIRE(rule == nullptr);
        }

        SECTION("space rule") {
            auto rule = Rule::fromSelector(" ");
            REQUIRE(rule == nullptr);
        }

        SECTION("spaces rule") {
            auto rule = Rule::fromSelector("   ");
            REQUIRE(rule == nullptr);
        }

        SECTION("spaces and dots rule") {
            auto rule = Rule::fromSelector(" . . ");
            REQUIRE(rule == nullptr);
        }

    }

    SECTION("tag rules") {

        SECTION("tag") {
            auto rule = Rule::fromSelector("div");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo() == none);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag:pseudo") {
            auto rule = Rule::fromSelector("div:hover");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo() == hover);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag.class") {
            auto rule = Rule::fromSelector("div.styled");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->pseudo() == none);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag.class:pseudo") {
            auto rule = Rule::fromSelector("div.styled:hover");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->pseudo() == hover);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag.class.class") {
            auto rule = Rule::fromSelector("div.styled.more");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().size() == 2);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->classes()[1] == "more");
            REQUIRE(rule->pseudo() == none);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag.class.class:pseudo") {
            auto rule = Rule::fromSelector("div.styled.more:hover");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().size() == 2);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->classes()[1] == "more");
            REQUIRE(rule->pseudo() == hover);
            REQUIRE(rule->next() == nullptr);
        }

    }

    SECTION("class rules") {

        SECTION(".class") {
            auto rule = Rule::fromSelector(".styled");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->pseudo() == none);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION(".class:pseudo") {
            auto rule = Rule::fromSelector(".styled:hover");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->pseudo() == hover);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION(".class.class") {
            auto rule = Rule::fromSelector(".styled.more");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().size() == 2);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->classes()[1] == "more");
            REQUIRE(rule->pseudo() == none);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION(".class.class:pseudo") {
            auto rule = Rule::fromSelector(".styled.more:hover");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().size() == 2);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->classes()[1] == "more");
            REQUIRE(rule->pseudo() == hover);
            REQUIRE(rule->next() == nullptr);
        }

    }

    SECTION("pseudo rules") {

        SECTION(":pseudo") {
            auto rule = Rule::fromSelector(":hover");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo() == hover);
            REQUIRE(rule->next() == nullptr);
        }

    }

    SECTION("nested rules") {

        SECTION("tag1 tag2") {
            auto rule = Rule::fromSelector("tag1 tag2");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "tag2");
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo() == none);
            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->tag() == "tag1");
            REQUIRE(rule->next()->classes().empty());
            REQUIRE(rule->next()->pseudo() == none);
            REQUIRE(rule->next()->next() == nullptr);
        }

        SECTION("tag1.class1 tag2.class2") {
            auto rule = Rule::fromSelector("tag1.class1 tag2.class2");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "tag2");
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "class2");
            REQUIRE(rule->pseudo() == none);
            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->tag() == "tag1");
            REQUIRE(rule->next()->classes().size() == 1);
            REQUIRE(rule->next()->classes()[0] == "class1");
            REQUIRE(rule->next()->pseudo() == none);
            REQUIRE(rule->next()->next() == nullptr);
        }

        SECTION("tag1.class1:pseudo1 tag2.class2:hover") {
            auto rule = Rule::fromSelector("tag1.class1:pseudo1 tag2.class2:hover");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "tag2");
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "class2");
            REQUIRE(rule->pseudo() == hover);
            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->tag() == "tag1");
            REQUIRE(rule->next()->classes().size() == 1);
            REQUIRE(rule->next()->classes()[0] == "class1");
            REQUIRE(rule->next()->pseudo() == none);
            REQUIRE(rule->next()->next() == nullptr);
        }

        SECTION("just.for:fun .this.one .should not:explode") {
            auto rule = Rule::fromSelector("just.for:fun .this.one .should not:explode");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->tag() == "not");
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo() == none);

            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->tag().empty());
            REQUIRE(rule->next()->classes().size() == 1);
            REQUIRE(rule->next()->classes()[0] == "should");
            REQUIRE(rule->next()->pseudo() == none);

            REQUIRE(rule->next()->next() != nullptr);
            REQUIRE(rule->next()->next()->tag().empty());
            REQUIRE(rule->next()->next()->classes().size() == 2);
            REQUIRE(rule->next()->next()->classes()[0] == "this");
            REQUIRE(rule->next()->next()->classes()[1] == "one");
            REQUIRE(rule->next()->next()->pseudo() == none);

            REQUIRE(rule->next()->next()->next() != nullptr);
            REQUIRE(rule->next()->next()->next()->tag() == "just");
            REQUIRE(rule->next()->next()->next()->classes().size() == 1);
            REQUIRE(rule->next()->next()->next()->classes()[0] == "for");
            REQUIRE(rule->next()->next()->next()->pseudo() == none);

            REQUIRE(rule->next()->next()->next()->next() == nullptr);
        }
    }

    SECTION("weights") {

        REQUIRE(Rule::fromSelector("div")->weight() == 1);
        REQUIRE(Rule::fromSelector("div.class")->weight() == 2);
        REQUIRE(Rule::fromSelector("div.class:hover")->weight() == 3);
        REQUIRE(Rule::fromSelector(".class:hover")->weight() == 2);
        REQUIRE(Rule::fromSelector(".class:fake")->weight() == 1);
        REQUIRE(Rule::fromSelector("div span")->weight() == 2);
        REQUIRE(Rule::fromSelector("div span.class")->weight() == 3);
        REQUIRE(Rule::fromSelector("div span.class.second")->weight() == 4);
        REQUIRE(Rule::fromSelector("div span.class.second:hover")->weight() == 5);
        REQUIRE(Rule::fromSelector("div.class span.class.second:hover")->weight() == 6);
        REQUIRE(Rule::fromSelector("div.class:hover span.class.second:hover")->weight() == 7);
        REQUIRE(Rule::fromSelector("div.class.second:hover span.class.second:hover")->weight() == 8);


    }
}