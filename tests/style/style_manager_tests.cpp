#include <memory>
#include "catch.hpp"
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Style.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Button.hpp>

using namespace psychic_ui;

TEST_CASE("basic style getter", "[style]") {
    auto styleManager = std::make_shared<StyleManager>();

    SECTION("returns dummy style if selector is garbage") {
        auto style = styleManager->style("");
        REQUIRE(style == Style::dummyStyle.get());
    }

    SECTION("returns the same style with same name") {
        auto style = styleManager->style("div");
        REQUIRE(style == styleManager->style("div"));
    }

    SECTION("in a case insensitive manner") {
        auto style = styleManager->style("div");
        REQUIRE(style == styleManager->style("Div"));
        REQUIRE(style == styleManager->style("div"));
    }
}

SCENARIO("styles can be declared and computed") {
    auto styleManager = std::make_shared<StyleManager>();

    GIVEN("a single div") {
        auto div = std::make_shared<Div>();
        div->setStyleManager(styleManager);

        WHEN("div has no class names") {

            THEN("it should retrieve a \"div\" selector") {
                auto style = styleManager->style("div");
                style->set(fontFamily, "div"); // Just easier to check like this
                div->updateStyle();
                auto s = styleManager->computeStyle(div.get());
                REQUIRE(*style == *s);
            }

            THEN("it should not retrieve a \"div.class\" selector") {
                auto style = styleManager->style("div.class");
                style->set(fontFamily, "div.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style != *styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto style = styleManager->style("div .class");
                style->set(fontFamily, "div .class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style != *styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \".class\" selector") {
                auto style = styleManager->style(".class");
                style->set(fontFamily, ".class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style != *styleManager->computeStyle(div.get()));
            }
        }

        WHEN("div has id") {
            div->setId("myDiv");

            THEN("it should retrieve a \"#myDiv\" selector") {
                auto style = styleManager->style("#myDiv");
                style->set(fontFamily, "div"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"#__internal_id__\" selector") {
                auto style = styleManager->style("#" + div->internalId());
                style->set(fontFamily, "div by internal id"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }
        }

        WHEN("div has class name") {
            div->setClassNames({"class"});

            THEN("it should retrieve a \"div\" selector") {
                auto style = styleManager->style("div");
                style->set(fontFamily, "div"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto style = styleManager->style("div.class");
                style->set(fontFamily, "div.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto style = styleManager->style("div .class");
                style->set(fontFamily, "div .class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style != *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager->style(".class");
                style->set(fontFamily, ".class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \".other\" selector") {
                auto style = styleManager->style(".other");
                style->set(fontFamily, ".other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style != *styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \".class.other\" selector") {
                auto style = styleManager->style(".class.other");
                style->set(fontFamily, ".class.other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style != *styleManager->computeStyle(div.get()));
            }
        }

        WHEN("div has multiple class names") {
            div->setClassNames({"class", "other"});

            THEN("it should retrieve a \"div\" selector") {
                auto style = styleManager->style("div");
                style->set(fontFamily, "div"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto style = styleManager->style("div.class");
                style->set(fontFamily, "div.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"div.class.other\" selector") {
                auto style = styleManager->style("div.class.other");
                style->set(fontFamily, "div.class.other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"div.other.class\" selector") {
                auto style = styleManager->style("div.other.class");
                style->set(fontFamily, "div.other.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto style = styleManager->style("div .class");
                style->set(fontFamily, "div .class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style != *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager->style(".class");
                style->set(fontFamily, ".class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".other\" selector") {
                auto style = styleManager->style(".other");
                style->set(fontFamily, ".other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".class.other\" selector") {
                auto style = styleManager->style(".class.other");
                style->set(fontFamily, ".class.other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".other.class\" selector") {
                auto style = styleManager->style(".other.class");
                style->set(fontFamily, ".other.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(*style == *styleManager->computeStyle(div.get()));
            }
        }

    }

    GIVEN("a parent/child div relationship") {
        auto parent = std::make_shared<Div>();
        parent->setStyleManager(styleManager);
        auto child = std::make_shared<Div>();
        parent->add(child);

        WHEN("neither have class names") {

            THEN("it should retrieve a \"div\" selector") {
                auto style = styleManager->style("div");
                style->set(fontFamily, "div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div\" selector") {
                auto style = styleManager->style("div div");
                style->set(fontFamily, "div div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div.class\" selector") {
                auto style = styleManager->style("div.class");
                style->set(fontFamily, "div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div.class div\" selector") {
                auto style = styleManager->style("div.class div");
                style->set(fontFamily, "div.class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto style = styleManager->style("div .class");
                style->set(fontFamily, "div .class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div div.class\" selector") {
                auto style = styleManager->style("div div.class");
                style->set(fontFamily, "div div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }
        }

        WHEN("parent has a class names") {
            parent->setClassNames({"class"});

            THEN("it should retrieve a \"div\" selector") {
                auto style = styleManager->style("div");
                style->set(fontFamily, "div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div\" selector") {
                auto style = styleManager->style("div div");
                style->set(fontFamily, "div div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto style = styleManager->style("div.class");
                style->set(fontFamily, "div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class div\" selector") {
                auto style = styleManager->style("div.class div");
                style->set(fontFamily, "div.class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto style = styleManager->style("div .class");
                style->set(fontFamily, "div .class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div div.class\" selector") {
                auto style = styleManager->style("div div.class");
                style->set(fontFamily, "div div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class div\" selector") {
                auto style = styleManager->style(".class div");
                style->set(fontFamily, ".class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager->style(".class");
                style->set(fontFamily, ".class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class .other\" selector") {
                auto style = styleManager->style(".class .other");
                style->set(fontFamily, ".class .other"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }
        }

        WHEN("child has a class names") {
            child->setClassNames({"class"});

            THEN("it should retrieve a \"div\" selector") {
                auto style = styleManager->style("div");
                style->set(fontFamily, "div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div\" selector") {
                auto style = styleManager->style("div div");
                style->set(fontFamily, "div div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto style = styleManager->style("div.class");
                style->set(fontFamily, "div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div.class div\" selector") {
                auto style = styleManager->style("div.class div");
                style->set(fontFamily, "div.class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div .class\" selector") {
                auto style = styleManager->style("div .class");
                style->set(fontFamily, "div .class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div.class\" selector") {
                auto style = styleManager->style("div div.class");
                style->set(fontFamily, "div div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class div\" selector") {
                auto style = styleManager->style(".class div");
                style->set(fontFamily, ".class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager->style(".class");
                style->set(fontFamily, ".class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class .other\" selector") {
                auto style = styleManager->style(".class .other");
                style->set(fontFamily, ".class .other"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }
        }

        WHEN("both have a class names") {
            parent->setClassNames({"class"});
            child->setClassNames({"other"});

            THEN("it should retrieve a \"div\" selector") {
                auto style = styleManager->style("div");
                style->set(fontFamily, "div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div\" selector") {
                auto style = styleManager->style("div div");
                style->set(fontFamily, "div div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto style = styleManager->style("div.class");
                style->set(fontFamily, "div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class div\" selector") {
                auto style = styleManager->style("div.class div");
                style->set(fontFamily, "div.class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto style = styleManager->style("div .class");
                style->set(fontFamily, "div .class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div div.class\" selector") {
                auto style = styleManager->style("div div.class");
                style->set(fontFamily, "div div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style != *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class div\" selector") {
                auto style = styleManager->style(".class div");
                style->set(fontFamily, ".class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager->style(".class");
                style->set(fontFamily, ".class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class .other\" selector") {
                auto style = styleManager->style(".class .other");
                style->set(fontFamily, ".class .other"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class div.other\" selector") {
                auto style = styleManager->style(".class div.other");
                style->set(fontFamily, ".class div.other"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(child.get()));
            }
        }

    }

    GIVEN("three div") {
        auto a = std::make_shared<Div>();
        a->setStyleManager(styleManager);
        auto b = std::make_shared<Div>();
        a->add(b);
        auto c = std::make_shared<Div>();
        b->add(c);

        WHEN("skipping a node") {
            a->setClassNames({"first"});
            c->setClassNames({"last"});

            THEN("it should retrieve a \".first\" selector") {
                auto style = styleManager->style(".first");
                style->set(fontFamily, ".first"); // Just easier to check like this
                a->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(c.get()));
            }

            THEN("it should retrieve a \".first .last\" selector") {
                auto style = styleManager->style(".first .last");
                style->set(fontFamily, ".first .last"); // Just easier to check like this
                a->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(c.get()));
            }

            THEN("it should retrieve a \".first div .last\" selector") {
                auto style = styleManager->style(".first div .last");
                style->set(fontFamily, ".first div .last"); // Just easier to check like this
                a->updateStyleRecursive();
                REQUIRE(*style == *styleManager->computeStyle(c.get()));
            }

        }

        WHEN("targeting a direct descendant") {
            a->setClassNames({"a"});
            b->setClassNames({"b"});
            c->setClassNames({"c"});
            auto style = styleManager->style(".a > div");
                style->set(width, 123);
            a->updateStyleRecursive();
            REQUIRE(std::isnan(a->computedStyle()->get(width)));
            REQUIRE(b->computedStyle()->get(width) == 123);
            REQUIRE(std::isnan(c->computedStyle()->get(width)));
        }

    }

}

TEST_CASE("Regressions") {
    auto styleManager = std::make_shared<StyleManager>();

    SECTION("Hovered button should transfer color to label") {
        // We're not testing buttons here, so we're emulating with only divs

        auto windowStyle = styleManager->style(".window");
        windowStyle->set(color, 0xFF00FF00);

        auto style = styleManager->style(".button");
        style->set(color, 0xFFFF0000);

        auto styleHover = styleManager->style(".button:hover");
        styleHover->set(color, 0xFF0000FF);

        auto window = std::make_shared<Div>();
        window->setStyleManager(styleManager);
        window->setClassNames({"window"});

        auto button = std::make_shared<Div>();
        button->setClassNames({"button"});
        window->add(button);

        auto label = std::make_shared<Div>();
        button->add(label);

        window->updateStyleRecursive();
        std::unique_ptr<Style> buttonComputed = std::move(styleManager->computeStyle(button.get()));
        std::unique_ptr<Style> labelComputed  = std::move(styleManager->computeStyle(label.get()));
        REQUIRE(buttonComputed->get(color) == 0xFFFF0000);
        REQUIRE(labelComputed->get(color) == 0xFFFF0000);

        button->setMouseOver(true);
        window->updateStyleRecursive();
        buttonComputed = std::move(styleManager->computeStyle(button.get()));
        labelComputed  = std::move(styleManager->computeStyle(label.get()));
        REQUIRE(buttonComputed->get(color) == 0xFF0000FF);
        REQUIRE(labelComputed->get(color) == 0xFF0000FF);

    }

    SECTION("Pseudos should be prioritized") {
        // We're not testing buttons here, so we're emulating with only divs

        styleManager->style(".test")
                    ->set(color, 0xFFFF0000);

        styleManager->style(".test:hover")
                    ->set(color, 0xFF0000FF);

        styleManager->style(".test:active")
                    ->set(color, 0xFF00FF00);

        auto div = std::make_shared<Div>();
        div->setStyleManager(styleManager);
        div->setClassNames({"test"});

        REQUIRE(styleManager->computeStyle(div.get())->get(color) == 0xFFFF0000);

        div->setMouseOver(true);
        REQUIRE(styleManager->computeStyle(div.get())->get(color) == 0xFF0000FF);

        div->setMouseDown(true);
        REQUIRE(styleManager->computeStyle(div.get())->get(color) == 0xFF00FF00);

    }

    SECTION("Should match an added style class") {
        styleManager->style("div")
                    ->set(color, 0xFFFF0000);

        styleManager->style("div.class")
                    ->set(color, 0xFF0000FF);

        auto div = std::make_shared<Div>();
        div->setStyleManager(styleManager);

        REQUIRE(styleManager->computeStyle(div.get())->get(color) == 0xFFFF0000);

        div->addClassName("class");
        REQUIRE(styleManager->computeStyle(div.get())->get(color) == 0xFF0000FF);
    }

    SECTION("Should match a parent tag") {
        styleManager->style("Component")
                    ->set(color, 0xFFFF0000);

        auto btn = std::make_shared<Button>();
        btn->setStyleManager(styleManager);

        REQUIRE(styleManager->computeStyle(btn.get())->get(color) == 0xFFFF0000);
    }

}
