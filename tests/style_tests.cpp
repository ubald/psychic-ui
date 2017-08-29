#include "catch.hpp"
#include <psychicui/Style.hpp>

using namespace psychicui;

TEST_CASE( "Default style is always a new style", "[style]" ) {
    auto a = Style::getDefaultStyle();
    auto b = Style::getDefaultStyle();
    // Just to be sure we can compare shared_ptrs ;)
    REQUIRE( a == a );
    REQUIRE( a != b );
}

TEST_CASE( "Style values can be set and retrieved", "[style]" ) {
    auto style = Style::getDefaultStyle();

    SECTION("with color values") {
        REQUIRE(style->getValue(color) == 0xFFFFFFFF);
        style->setValue(color, 0xFFFF0000);
        REQUIRE(style->getValue(color) == 0xFFFF0000);
    }

    SECTION("with string values") {
        REQUIRE(style->getValue(fontFamily) == "");
        style->setValue(fontFamily, "Arial");
        REQUIRE(style->getValue(fontFamily) == "Arial");
    }

    SECTION("with bool values") {
        REQUIRE(!style->getValue(antiAlias));
        style->setValue(antiAlias, true);
        REQUIRE(style->getValue(antiAlias));
    }

    SECTION("with int values") {
        REQUIRE(style->getValue(textSize) == 12);
        style->setValue(textSize, 24);
        REQUIRE(style->getValue(textSize) == 24);
    }
}

TEST_CASE( "Styles can be overlayed", "[style]" ) {
    auto receiver = Style::getDefaultStyle();
    auto overlay = Style::getDefaultStyle();

    SECTION("with color values") {
        overlay->setValue(color, 0xFFFF0000);
        receiver->overlay(overlay);
        REQUIRE(overlay->getValue(color) == receiver->getValue(color));
    }

    SECTION("with string values") {
        overlay->setValue(fontFamily, "Arial");
        receiver->overlay(overlay);
        REQUIRE(overlay->getValue(fontFamily) == receiver->getValue(fontFamily));
    }

    SECTION("with bool values") {
        overlay->setValue(antiAlias, true);
        receiver->overlay(overlay);
        REQUIRE(overlay->getValue(antiAlias) == receiver->getValue(antiAlias));
    }

    SECTION("with int values") {
        overlay->setValue(textSize, 24);
        receiver->overlay(overlay);
        REQUIRE(overlay->getValue(textSize) == receiver->getValue(textSize));
    }
}