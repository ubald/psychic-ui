#include "catch2/catch.hpp"
#include <psychic-ui/style/StyleSelector.hpp>

using namespace psychic_ui;

TEST_CASE("rules from selectors", "[style]") {

    SECTION("invalids") {

        SECTION("empty rule") {
            auto rule = StyleSelector::fromSelector("");
            REQUIRE(rule == nullptr);
        }

        SECTION("space rule") {
            auto rule = StyleSelector::fromSelector(" ");
            REQUIRE(rule == nullptr);
        }

        SECTION("spaces rule") {
            auto rule = StyleSelector::fromSelector("   ");
            REQUIRE(rule == nullptr);
        }

        SECTION("spaces and dots rule") {
            auto rule = StyleSelector::fromSelector(" . . ");
            REQUIRE(rule == nullptr);
        }

        SECTION("spaces and hashtags rule") {
            auto rule = StyleSelector::fromSelector(" # # ");
            REQUIRE(rule == nullptr);
        }

        SECTION("spaces and colons rule") {
            auto rule = StyleSelector::fromSelector(" : : ");
            REQUIRE(rule == nullptr);
        }

        SECTION("spaces and all of the above rule") {
            auto rule = StyleSelector::fromSelector(" # . : ");
            REQUIRE(rule == nullptr);
        }

    }

    SECTION("tag rules") {

        SECTION("tag") {
            auto rule = StyleSelector::fromSelector("div");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo().empty());
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag:pseudo") {
            auto rule = StyleSelector::fromSelector("div:hover");
            auto a = std::unordered_set<Pseudo, std::hash<int>>{hover};
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo() == a);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag.class") {
            auto rule = StyleSelector::fromSelector("div.styled");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->pseudo().empty());
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag.class:pseudo") {
            auto rule = StyleSelector::fromSelector("div.styled:hover");
            auto a = std::unordered_set<Pseudo, std::hash<int>>{hover};
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->pseudo() == a);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag.class.class") {
            auto rule = StyleSelector::fromSelector("div.styled.more");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().size() == 2);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->classes()[1] == "more");
            REQUIRE(rule->pseudo().empty());
            REQUIRE(rule->next() == nullptr);
        }

        SECTION("tag.class.class:pseudo") {
            auto rule = StyleSelector::fromSelector("div.styled.more:hover");
            auto a = std::unordered_set<Pseudo, std::hash<int>>{hover};
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "div");
            REQUIRE(rule->classes().size() == 2);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->classes()[1] == "more");
            REQUIRE(rule->pseudo() == a);
            REQUIRE(rule->next() == nullptr);
        }

    }

    SECTION("class rules") {

        SECTION(".class") {
            auto rule = StyleSelector::fromSelector(".styled");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->pseudo().empty());
            REQUIRE(rule->next() == nullptr);
        }

        SECTION(".class:pseudo") {
            auto rule = StyleSelector::fromSelector(".styled:hover");
            auto a = std::unordered_set<Pseudo, std::hash<int>>{hover};
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->pseudo() == a);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION(".class.class") {
            auto rule = StyleSelector::fromSelector(".styled.more");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().size() == 2);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->classes()[1] == "more");
            REQUIRE(rule->pseudo().empty());
            REQUIRE(rule->next() == nullptr);
        }

        SECTION(".class.class:pseudo") {
            auto rule = StyleSelector::fromSelector(".styled.more:hover");
            auto a = std::unordered_set<Pseudo, std::hash<int>>{hover};
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().size() == 2);
            REQUIRE(rule->classes()[0] == "styled");
            REQUIRE(rule->classes()[1] == "more");
            REQUIRE(rule->pseudo() == a);
            REQUIRE(rule->next() == nullptr);
        }

    }

    SECTION("pseudo rules") {

        SECTION(":pseudo") {
            auto rule = StyleSelector::fromSelector(":hover");
            auto a = std::unordered_set<Pseudo, std::hash<int>>{hover};
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo() == a);
            REQUIRE(rule->next() == nullptr);
        }

        SECTION(":pseudo") {
            auto rule = StyleSelector::fromSelector(":hover:disabled");
            auto a = std::unordered_set<Pseudo, std::hash<int>>{hover, disabled};
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag().empty());
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo() == a);
            REQUIRE(rule->next() == nullptr);
        }

    }

    SECTION("nested rules") {

        SECTION("tag1 tag2") {
            auto rule = StyleSelector::fromSelector("tag1 tag2");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "tag2");
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo().empty());
            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->tag() == "tag1");
            REQUIRE(rule->next()->classes().empty());
            REQUIRE(rule->next()->pseudo().empty());
            REQUIRE(rule->next()->next() == nullptr);
        }

        SECTION("tag1.class1 tag2.class2") {
            auto rule = StyleSelector::fromSelector("tag1.class1 tag2.class2");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "tag2");
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "class2");
            REQUIRE(rule->pseudo().empty());
            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->tag() == "tag1");
            REQUIRE(rule->next()->classes().size() == 1);
            REQUIRE(rule->next()->classes()[0] == "class1");
            REQUIRE(rule->next()->pseudo().empty());
            REQUIRE(rule->next()->next() == nullptr);
        }

        SECTION("tag1.class1:pseudo1 tag2.class2:hover") {
            auto rule = StyleSelector::fromSelector("tag1.class1:pseudo1 tag2.class2:hover");
            auto a = std::unordered_set<Pseudo, std::hash<int>>{hover};
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "tag2");
            REQUIRE(rule->classes().size() == 1);
            REQUIRE(rule->classes()[0] == "class2");
            REQUIRE(rule->pseudo() == a);
            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->tag() == "tag1");
            REQUIRE(rule->next()->classes().size() == 1);
            REQUIRE(rule->next()->classes()[0] == "class1");
            REQUIRE(rule->next()->pseudo().empty());
            REQUIRE(rule->next()->next() == nullptr);
        }

        SECTION("just.for:fun .this.one .should not:explode") {
            auto rule = StyleSelector::fromSelector("just.for:fun .this.one .should not:explode");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "not");
            REQUIRE(rule->classes().empty());
            REQUIRE(rule->pseudo().empty());

            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->direct() == false);
            REQUIRE(rule->next()->tag().empty());
            REQUIRE(rule->next()->classes().size() == 1);
            REQUIRE(rule->next()->classes()[0] == "should");
            REQUIRE(rule->next()->pseudo().empty());

            REQUIRE(rule->next()->next() != nullptr);
            REQUIRE(rule->next()->next()->direct() == false);
            REQUIRE(rule->next()->next()->tag().empty());
            REQUIRE(rule->next()->next()->classes().size() == 2);
            REQUIRE(rule->next()->next()->classes()[0] == "this");
            REQUIRE(rule->next()->next()->classes()[1] == "one");
            REQUIRE(rule->next()->next()->pseudo().empty());

            REQUIRE(rule->next()->next()->next() != nullptr);
            REQUIRE(rule->next()->next()->next()->direct() == false);
            REQUIRE(rule->next()->next()->next()->tag() == "just");
            REQUIRE(rule->next()->next()->next()->classes().size() == 1);
            REQUIRE(rule->next()->next()->next()->classes()[0] == "for");
            REQUIRE(rule->next()->next()->next()->pseudo().empty());

            REQUIRE(rule->next()->next()->next()->next() == nullptr);
        }
    }

    SECTION("direct descendants") {

        SECTION("simple") {
            auto rule = StyleSelector::fromSelector("parent > child");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "child");

            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->direct() == true);
            REQUIRE(rule->next()->tag() == "parent");
        }

        SECTION("multiple") {
            auto rule = StyleSelector::fromSelector("parent > child > grandchild");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "grandchild");

            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->direct() == true);
            REQUIRE(rule->next()->tag() == "child");

            REQUIRE(rule->next()->next() != nullptr);
            REQUIRE(rule->next()->next()->direct() == true);
            REQUIRE(rule->next()->next()->tag() == "parent");
        }

        SECTION("complex") {
            auto rule = StyleSelector::fromSelector("parent > child car > wheel");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->direct() == false);
            REQUIRE(rule->tag() == "wheel");

            REQUIRE(rule->next() != nullptr);
            REQUIRE(rule->next()->direct() == true);
            REQUIRE(rule->next()->tag() == "car");

            REQUIRE(rule->next()->next() != nullptr);
            REQUIRE(rule->next()->next()->direct() == false);
            REQUIRE(rule->next()->next()->tag() == "child");

            REQUIRE(rule->next()->next()->next() != nullptr);
            REQUIRE(rule->next()->next()->next()->direct() == true);
            REQUIRE(rule->next()->next()->next()->tag() == "parent");
        }

    }

    SECTION("ids") {

        SECTION("simple") {
            auto rule = StyleSelector::fromSelector("#id");
            REQUIRE(rule != nullptr);
            REQUIRE(rule->id() == "id");
        }

    }

    SECTION("weights") {
        REQUIRE(StyleSelector::fromSelector("div")->weight() == 10);
        REQUIRE(StyleSelector::fromSelector("div#id")->weight() == 25);
        REQUIRE(StyleSelector::fromSelector("div.class")->weight() == 20);
        REQUIRE(StyleSelector::fromSelector("div#id.class")->weight() == 35);
        REQUIRE(StyleSelector::fromSelector("div.class:hover")->weight() == 21);
        REQUIRE(StyleSelector::fromSelector("div#id.class:hover")->weight() == 36);
        REQUIRE(StyleSelector::fromSelector(".class:hover")->weight() == 11);
        REQUIRE(StyleSelector::fromSelector("#id.class:hover")->weight() == 26);
        REQUIRE(StyleSelector::fromSelector(".class:fake")->weight() == 10);
        REQUIRE(StyleSelector::fromSelector("#id.class:fake")->weight() == 25);
        REQUIRE(StyleSelector::fromSelector("div span")->weight() == 20);
        REQUIRE(StyleSelector::fromSelector("div#id span#id")->weight() == 50);
        REQUIRE(StyleSelector::fromSelector("div span.class")->weight() == 30);
        REQUIRE(StyleSelector::fromSelector("div#id span.class")->weight() == 45);
        REQUIRE(StyleSelector::fromSelector("div span.class.second")->weight() == 40);
        REQUIRE(StyleSelector::fromSelector("div span.class.second:active")->weight() == 42);
        REQUIRE(StyleSelector::fromSelector("div.class span.class.second:hover")->weight() == 51);
        REQUIRE(StyleSelector::fromSelector("div.class:hover span.class.second:hover")->weight() == 52);
        REQUIRE(StyleSelector::fromSelector("div.class.second:hover span.class.second:hover")->weight() == 62);
    }
}
