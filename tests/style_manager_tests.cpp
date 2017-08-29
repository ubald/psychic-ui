#include "catch.hpp"
#include <psychicui/StyleManager.hpp>

using namespace psychicui;

TEST_CASE( "basic style getter", "[style]" ) {
    StyleManager styleManager;

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

    GIVEN( "a type only style" ) {
        auto style = styleManager.getStyle("component");
        style->setValue(color, 0xFFFF0000);
        style->setValue(fontFamily, "Arial");
        style->setValue(antiAlias, true);
        style->setValue(textSize, 24);

        WHEN( "retrieving with a type" ) {
            REQUIRE(*style == *styleManager.computeStyle("component"));
        }

        WHEN( "retrieving with a type and class" ) {
            REQUIRE(*style == *styleManager.computeStyle("component.style"));
        }
    }
}