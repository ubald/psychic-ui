#include "catch.hpp"
#include <psychicui/style/StyleManager.hpp>
#include <psychicui/style/Style.hpp>

using namespace psychicui;

TEST_CASE( "basic style getter", "[style]" ) {
    StyleManager styleManager;

    SECTION("returns dummy style if selector is garbage") {
        auto style = styleManager.getStyle("");
        REQUIRE(style == Style::dummyStyle.get());
    }

    SECTION("returns the same style with same name") {
        auto style = styleManager.getStyle("component");
        REQUIRE(style == styleManager.getStyle("component"));
    }

    SECTION("in a case insensitive manner") {
        auto style = styleManager.getStyle("component");
        REQUIRE(style == styleManager.getStyle("Component"));
        REQUIRE(style == styleManager.getStyle("cOmPoNeNt"));
    }
}

SCENARIO( "styles can be declared and computed" ) {
    StyleManager styleManager;

    GIVEN( "a single component" ) {
        auto component = std::make_shared<Component>();

        WHEN( "component has no class names" ) {

            THEN("it should retrieve a \"component\" selector") {
                auto style = styleManager.getStyle("component");
                style->setValue(fontFamily, "component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should not retrieve a \"component.class\" selector") {
                auto style = styleManager.getStyle("component.class");
                style->setValue(fontFamily, "component.class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(component.get()));
            }

            THEN("it should not retrieve a \"component .class\" selector") {
                auto style = styleManager.getStyle("component .class");
                style->setValue(fontFamily, "component .class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(component.get()));
            }

            THEN("it should not retrieve a \".class\" selector") {
                auto style = styleManager.getStyle(".class");
                style->setValue(fontFamily, ".class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(component.get()));
            }
        }

        WHEN( "component has class name" ) {
            component->setClassNames({"class"});

            THEN("it should retrieve a \"component\" selector") {
                auto style = styleManager.getStyle("component");
                style->setValue(fontFamily, "component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \"component.class\" selector") {
                auto style = styleManager.getStyle("component.class");
                style->setValue(fontFamily, "component.class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should not retrieve a \"component .class\" selector") {
                auto style = styleManager.getStyle("component .class");
                style->setValue(fontFamily, "component .class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager.getStyle(".class");
                style->setValue(fontFamily, ".class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should not retrieve a \".other\" selector") {
                auto style = styleManager.getStyle(".other");
                style->setValue(fontFamily, ".other"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(component.get()));
            }

            THEN("it should not retrieve a \".class.other\" selector") {
                auto style = styleManager.getStyle(".class.other");
                style->setValue(fontFamily, ".class.other"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(component.get()));
            }
        }

        WHEN( "component has multiple class names" ) {
            component->setClassNames({"class", "other"});

            THEN("it should retrieve a \"component\" selector") {
                auto style = styleManager.getStyle("component");
                style->setValue(fontFamily, "component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \"component.class\" selector") {
                auto style = styleManager.getStyle("component.class");
                style->setValue(fontFamily, "component.class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \"component.class.other\" selector") {
                auto style = styleManager.getStyle("component.class.other");
                style->setValue(fontFamily, "component.class.other"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \"component.other.class\" selector") {
                auto style = styleManager.getStyle("component.other.class");
                style->setValue(fontFamily, "component.other.class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should not retrieve a \"component .class\" selector") {
                auto style = styleManager.getStyle("component .class");
                style->setValue(fontFamily, "component .class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager.getStyle(".class");
                style->setValue(fontFamily, ".class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \".other\" selector") {
                auto style = styleManager.getStyle(".other");
                style->setValue(fontFamily, ".other"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \".class.other\" selector") {
                auto style = styleManager.getStyle(".class.other");
                style->setValue(fontFamily, ".class.other"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }

            THEN("it should retrieve a \".other.class\" selector") {
                auto style = styleManager.getStyle(".other.class");
                style->setValue(fontFamily, ".other.class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(component.get()));
            }
        }

    }
    
    GIVEN( "a parent/child component relationship" ) {
        auto parent = std::make_shared<Component>();
        auto child = std::make_shared<Component>();
        parent->addChild(child);
        
        WHEN( "neither have class names" ) {

            THEN("it should retrieve a \"component\" selector") {
                auto style = styleManager.getStyle("component");
                style->setValue(fontFamily, "component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component component\" selector") {
                auto style = styleManager.getStyle("component component");
                style->setValue(fontFamily, "component component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component.class\" selector") {
                auto style = styleManager.getStyle("component.class");
                style->setValue(fontFamily, "component.class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component.class component\" selector") {
                auto style = styleManager.getStyle("component.class component");
                style->setValue(fontFamily, "component.class component"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component .class\" selector") {
                auto style = styleManager.getStyle("component .class");
                style->setValue(fontFamily, "component .class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component component.class\" selector") {
                auto style = styleManager.getStyle("component component.class");
                style->setValue(fontFamily, "component component.class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }
        }

        WHEN( "parent has a class names" ) {
            parent->setClassNames({"class"});

            THEN("it should retrieve a \"component\" selector") {
                auto style = styleManager.getStyle("component");
                style->setValue(fontFamily, "component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component component\" selector") {
                auto style = styleManager.getStyle("component component");
                style->setValue(fontFamily, "component component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component.class\" selector") {
                auto style = styleManager.getStyle("component.class");
                style->setValue(fontFamily, "component.class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component.class component\" selector") {
                auto style = styleManager.getStyle("component.class component");
                style->setValue(fontFamily, "component.class component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component .class\" selector") {
                auto style = styleManager.getStyle("component .class");
                style->setValue(fontFamily, "component .class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component component.class\" selector") {
                auto style = styleManager.getStyle("component component.class");
                style->setValue(fontFamily, "component component.class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class component\" selector") {
                auto style = styleManager.getStyle(".class component");
                style->setValue(fontFamily, ".class component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager.getStyle(".class");
                style->setValue(fontFamily, ".class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class .other\" selector") {
                auto style = styleManager.getStyle(".class .other");
                style->setValue(fontFamily, ".class .other"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }
        }

        WHEN( "child has a class names" ) {
            child->setClassNames({"class"});

            THEN("it should retrieve a \"component\" selector") {
                auto style = styleManager.getStyle("component");
                style->setValue(fontFamily, "component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component component\" selector") {
                auto style = styleManager.getStyle("component component");
                style->setValue(fontFamily, "component component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component.class\" selector") {
                auto style = styleManager.getStyle("component.class");
                style->setValue(fontFamily, "component.class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component.class component\" selector") {
                auto style = styleManager.getStyle("component.class component");
                style->setValue(fontFamily, "component.class component"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component .class\" selector") {
                auto style = styleManager.getStyle("component .class");
                style->setValue(fontFamily, "component .class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component component.class\" selector") {
                auto style = styleManager.getStyle("component component.class");
                style->setValue(fontFamily, "component component.class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class component\" selector") {
                auto style = styleManager.getStyle(".class component");
                style->setValue(fontFamily, ".class component"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager.getStyle(".class");
                style->setValue(fontFamily, ".class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \".class .other\" selector") {
                auto style = styleManager.getStyle(".class .other");
                style->setValue(fontFamily, ".class .other"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }
        }

        WHEN( "both have a class names" ) {
            parent->setClassNames({"class"});
            child->setClassNames({"other"});

            THEN("it should retrieve a \"component\" selector") {
                auto style = styleManager.getStyle("component");
                style->setValue(fontFamily, "component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component component\" selector") {
                auto style = styleManager.getStyle("component component");
                style->setValue(fontFamily, "component component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component.class\" selector") {
                auto style = styleManager.getStyle("component.class");
                style->setValue(fontFamily, "component.class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \"component.class component\" selector") {
                auto style = styleManager.getStyle("component.class component");
                style->setValue(fontFamily, "component.class component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component .class\" selector") {
                auto style = styleManager.getStyle("component .class");
                style->setValue(fontFamily, "component .class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should not retrieve a \"component component.class\" selector") {
                auto style = styleManager.getStyle("component component.class");
                style->setValue(fontFamily, "component component.class"); // Just easier to check like this
                REQUIRE(*style != *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class component\" selector") {
                auto style = styleManager.getStyle(".class component");
                style->setValue(fontFamily, ".class component"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class\" selector") {
                auto style = styleManager.getStyle(".class");
                style->setValue(fontFamily, ".class"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class .other\" selector") {
                auto style = styleManager.getStyle(".class .other");
                style->setValue(fontFamily, ".class .other"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }

            THEN("it should retrieve a \".class component.other\" selector") {
                auto style = styleManager.getStyle(".class component.other");
                style->setValue(fontFamily, ".class component.other"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(child.get()));
            }
        }

    }

    GIVEN( "three component" ) {
        auto a = std::make_shared<Component>();
        auto b = std::make_shared<Component>();
        a->addChild(b);
        auto c = std::make_shared<Component>();
        b->addChild(c);

        WHEN("skipping a node") {
            a->setClassNames({"first"});
            c->setClassNames({"last"});

            THEN("it should retrieve a \".first\" selector") {
                auto style = styleManager.getStyle(".first");
                style->setValue(fontFamily, ".first"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(c.get()));
            }

            THEN("it should retrieve a \".first .last\" selector") {
                auto style = styleManager.getStyle(".first .last");
                style->setValue(fontFamily, ".first .last"); // Just easier to check like this
                REQUIRE(*style == *styleManager.computeStyle(c.get()));
            }

        }

    }

}