#include <memory>
#include <stdexcept>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Style.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/style/Color.hpp>
#include <psychic-ui/style/Properties.hpp>

using namespace psychic_ui;

TEST_CASE("basic style getter", "[style]") {
    auto styleManager = std::make_shared<StyleManager>();

    SECTION("throws if the selector is garbage") {
        REQUIRE_THROWS_AS(styleManager->style(""), std::invalid_argument);
        REQUIRE_THROWS_AS(styleManager->style(" . . "), std::invalid_argument);
    }

    SECTION("returns the same style with same name") {
        auto &style = styleManager->style("div");
        REQUIRE(&style == &styleManager->style("div"));
    }

    SECTION("in a case insensitive manner") {
        auto &style = styleManager->style("div");
        REQUIRE(&style == &styleManager->style("Div"));
        REQUIRE(&style == &styleManager->style("div"));
    }
}

SCENARIO("styles can be declared and computed") {
    auto styleManager = std::make_shared<StyleManager>();

    GIVEN("a single div") {
        auto div = std::make_shared<Div>();
        div->setStyleManager(styleManager);

        WHEN("div has no class names") {

            THEN("it should retrieve a \"div\" selector") {
                auto &style = styleManager->style("div");
                style.fontFamily("div"); // Just easier to check like this
                div->updateStyle();
                auto s = styleManager->computeStyle(div.get());
                REQUIRE(style == s);
            }

            THEN("it should not retrieve a \"div.class\" selector") {
                auto &style = styleManager->style("div.class");
                style.fontFamily("div.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style != styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto &style = styleManager->style("div .class");
                style.fontFamily("div .class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style != styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \".class\" selector") {
                auto &style = styleManager->style(".class");
                style.fontFamily(".class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style != styleManager->computeStyle(div.get()));
            }
        }

        WHEN("div has id") {
            div->id("myDiv");

            THEN("it should retrieve a \"#myDiv\" selector") {
                auto &style = styleManager->style("#myDiv");
                style.fontFamily("div"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"#__internal_id__\" selector") {
                auto &style = styleManager->style("#" + div->internalId());
                style.fontFamily("div by internal id"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }
        }

        WHEN("div has class name") {
            div->classNames({"class"});

            THEN("it should retrieve a \"div\" selector") {
                auto &style = styleManager->style("div");
                style.fontFamily("div"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto &style = styleManager->style("div.class");
                style.fontFamily("div.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto &style = styleManager->style("div .class");
                style.fontFamily("div .class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style != styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto &style = styleManager->style(".class");
                style.fontFamily(".class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \".other\" selector") {
                auto &style = styleManager->style(".other");
                style.fontFamily(".other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style != styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \".class.other\" selector") {
                auto &style = styleManager->style(".class.other");
                style.fontFamily(".class.other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style != styleManager->computeStyle(div.get()));
            }
        }

        WHEN("div has multiple class names") {
            div->classNames({"class", "other"});

            THEN("it should retrieve a \"div\" selector") {
                auto &style = styleManager->style("div");
                style.fontFamily("div"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto &style = styleManager->style("div.class");
                style.fontFamily("div.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"div.class.other\" selector") {
                auto &style = styleManager->style("div.class.other");
                style.fontFamily("div.class.other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \"div.other.class\" selector") {
                auto &style = styleManager->style("div.other.class");
                style.fontFamily("div.other.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto &style = styleManager->style("div .class");
                style.fontFamily("div .class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style != styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto &style = styleManager->style(".class");
                style.fontFamily(".class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".other\" selector") {
                auto &style = styleManager->style(".other");
                style.fontFamily(".other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".class.other\" selector") {
                auto &style = styleManager->style(".class.other");
                style.fontFamily(".class.other"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
            }

            THEN("it should retrieve a \".other.class\" selector") {
                auto &style = styleManager->style(".other.class");
                style.fontFamily(".other.class"); // Just easier to check like this
                div->updateStyle();
                REQUIRE(style == styleManager->computeStyle(div.get()));
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
                auto &style = styleManager->style("div");
                style.fontFamily("div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div\" selector") {
                auto &style = styleManager->style("div div");
                style.fontFamily("div div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div.class\" selector") {
                auto &style = styleManager->style("div.class");
                style.fontFamily("div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div.class div\" selector") {
                auto &style = styleManager->style("div.class div");
                style.fontFamily("div.class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto &style = styleManager->style("div .class");
                style.fontFamily("div .class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div div.class\" selector") {
                auto &style = styleManager->style("div div.class");
                style.fontFamily("div div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }
        }

        WHEN("parent has a class names") {
            parent->classNames({"class"});

            THEN("it should retrieve a \"div\" selector") {
                auto &style = styleManager->style("div");
                style.fontFamily("div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div\" selector") {
                auto &style = styleManager->style("div div");
                style.fontFamily("div div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto &style = styleManager->style("div.class");
                style.fontFamily("div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class div\" selector") {
                auto &style = styleManager->style("div.class div");
                style.fontFamily("div.class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto &style = styleManager->style("div .class");
                style.fontFamily("div .class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div div.class\" selector") {
                auto &style = styleManager->style("div div.class");
                style.fontFamily("div div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class div\" selector") {
                auto &style = styleManager->style(".class div");
                style.fontFamily(".class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto &style = styleManager->style(".class");
                style.fontFamily(".class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class .other\" selector") {
                auto &style = styleManager->style(".class .other");
                style.fontFamily(".class .other"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }
        }

        WHEN("child has a class names") {
            child->classNames({"class"});

            THEN("it should retrieve a \"div\" selector") {
                auto &style = styleManager->style("div");
                style.fontFamily("div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div\" selector") {
                auto &style = styleManager->style("div div");
                style.fontFamily("div div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto &style = styleManager->style("div.class");
                style.fontFamily("div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div.class div\" selector") {
                auto &style = styleManager->style("div.class div");
                style.fontFamily("div.class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div .class\" selector") {
                auto &style = styleManager->style("div .class");
                style.fontFamily("div .class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div.class\" selector") {
                auto &style = styleManager->style("div div.class");
                style.fontFamily("div div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class div\" selector") {
                auto &style = styleManager->style(".class div");
                style.fontFamily(".class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto &style = styleManager->style(".class");
                style.fontFamily(".class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class .other\" selector") {
                auto &style = styleManager->style(".class .other");
                style.fontFamily(".class .other"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }
        }

        WHEN("both have a class names") {
            parent->classNames({"class"});
            child->classNames({"other"});

            THEN("it should retrieve a \"div\" selector") {
                auto &style = styleManager->style("div");
                style.fontFamily("div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div div\" selector") {
                auto &style = styleManager->style("div div");
                style.fontFamily("div div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class\" selector") {
                auto &style = styleManager->style("div.class");
                style.fontFamily("div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \"div.class div\" selector") {
                auto &style = styleManager->style("div.class div");
                style.fontFamily("div.class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div .class\" selector") {
                auto &style = styleManager->style("div .class");
                style.fontFamily("div .class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"div div.class\" selector") {
                auto &style = styleManager->style("div div.class");
                style.fontFamily("div div.class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style != styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class div\" selector") {
                auto &style = styleManager->style(".class div");
                style.fontFamily(".class div"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto &style = styleManager->style(".class");
                style.fontFamily(".class"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class .other\" selector") {
                auto &style = styleManager->style(".class .other");
                style.fontFamily(".class .other"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class div.other\" selector") {
                auto &style = styleManager->style(".class div.other");
                style.fontFamily(".class div.other"); // Just easier to check like this
                parent->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(child.get()));
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
            a->classNames({"first"});
            c->classNames({"last"});

            THEN("it should retrieve a \".first\" selector") {
                auto &style = styleManager->style(".first");
                style.fontFamily(".first"); // Just easier to check like this
                a->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(c.get()));
            }

            THEN("it should retrieve a \".first .last\" selector") {
                auto &style = styleManager->style(".first .last");
                style.fontFamily(".first .last"); // Just easier to check like this
                a->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(c.get()));
            }

            THEN("it should retrieve a \".first div .last\" selector") {
                auto &style = styleManager->style(".first div .last");
                style.fontFamily(".first div .last"); // Just easier to check like this
                a->updateStyleRecursive();
                REQUIRE(style == styleManager->computeStyle(c.get()));
            }

        }

        WHEN("targeting a direct descendant") {
            a->classNames({"a"});
            b->classNames({"b"});
            c->classNames({"c"});
            auto &style = styleManager->style(".a > div");
                style.width(123);
            a->updateStyleRecursive();
            REQUIRE_FALSE(a->computedStyle().has(Property::width));
            REQUIRE(b->computedStyle().width() == 123);
            REQUIRE_FALSE(c->computedStyle().has(Property::width));
        }

    }

}

TEST_CASE("Regressions") {
    auto styleManager = std::make_shared<StyleManager>();

    SECTION("Hovered button should transfer color to label") {
        // We're not testing buttons here, so we're emulating with only divs

        auto &windowStyle = styleManager->style(".window");
        windowStyle.color(argb(0xFF00FF00));

        auto &style = styleManager->style(".button");
        style.color(argb(0xFFFF0000));

        auto &styleHover = styleManager->style(".button:hover");
        styleHover.color(argb(0xFF0000FF));

        auto window = std::make_shared<Div>();
        window->setStyleManager(styleManager);
        window->classNames({"window"});

        auto button = std::make_shared<Div>();
        button->classNames({"button"});
        window->add(button);

        auto label = std::make_shared<Div>();
        button->add(label);

        window->updateStyleRecursive();
        Style buttonComputed = styleManager->computeStyle(button.get());
        Style labelComputed = styleManager->computeStyle(label.get());
        REQUIRE(buttonComputed.color() == argb(0xFFFF0000));
        REQUIRE(labelComputed.color() == argb(0xFFFF0000));

        button->setMouseOver(true);
        window->updateStyleRecursive();
        buttonComputed = styleManager->computeStyle(button.get());
        labelComputed = styleManager->computeStyle(label.get());
        REQUIRE(buttonComputed.color() == argb(0xFF0000FF));
        REQUIRE(labelComputed.color() == argb(0xFF0000FF));

    }

    SECTION("Pseudos should be prioritized") {
        // We're not testing buttons here, so we're emulating with only divs

        styleManager->style(".test")
                    .color(argb(0xFFFF0000));

        styleManager->style(".test:hover")
                    .color(argb(0xFF0000FF));

        styleManager->style(".test:active")
                    .color(argb(0xFF00FF00));

        auto div = std::make_shared<Div>();
        div->setStyleManager(styleManager);
        div->classNames({"test"});

        REQUIRE(styleManager->computeStyle(div.get()).color() == argb(0xFFFF0000));

        div->setMouseOver(true);
        REQUIRE(styleManager->computeStyle(div.get()).color() == argb(0xFF0000FF));

        div->setMouseDown(true);
        REQUIRE(styleManager->computeStyle(div.get()).color() == argb(0xFF00FF00));

    }

    SECTION("Should match an added style class") {
        styleManager->style("div")
                    .color(argb(0xFFFF0000));

        styleManager->style("div.class")
                    .color(argb(0xFF0000FF));

        auto div = std::make_shared<Div>();
        div->setStyleManager(styleManager);

        REQUIRE(styleManager->computeStyle(div.get()).color() == argb(0xFFFF0000));

        div->addClassName("class");
        REQUIRE(styleManager->computeStyle(div.get()).color() == argb(0xFF0000FF));
    }

    SECTION("Should match a parent tag") {
        styleManager->style("Component")
                    .color(argb(0xFFFF0000));

        auto btn = std::make_shared<Button>();
        btn->setStyleManager(styleManager);

        REQUIRE(styleManager->computeStyle(btn.get()).color() == argb(0xFFFF0000));
    }

}
