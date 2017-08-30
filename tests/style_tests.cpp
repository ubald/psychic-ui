#include "catch.hpp"
#include <psychicui/style/Style.hpp>

using namespace psychicui;

TEST_CASE( "Style values can be set and retrieved", "[style]" ) {
    auto style = std::make_unique<Style>();

    SECTION("with color values") {
        REQUIRE(style->getValue(color) == 0xFF000000);
        style->setValue(color, 0xFFFF0000);
        REQUIRE(style->getValue(color) == 0xFFFF0000);
    }

    SECTION("with string values") {
        REQUIRE(style->getValue(fontFamily) == "");
        style->setValue(fontFamily, "Arial");
        REQUIRE(style->getValue(fontFamily) == "Arial");
    }

    SECTION("with float values") {
        REQUIRE(style->getValue(opacity) == 0.0f);
        style->setValue(opacity, 1.0f);
        REQUIRE(style->getValue(opacity) == 1.0f);
    }

    SECTION("with int values") {
        REQUIRE(style->getValue(fontSize) == 0);
        style->setValue(fontSize, 24);
        REQUIRE(style->getValue(fontSize) == 24);
    }

    SECTION("with bool values") {
        REQUIRE(!style->getValue(antiAlias));
        style->setValue(antiAlias, true);
        REQUIRE(style->getValue(antiAlias));
    }
}

TEST_CASE( "Styles can be overlayed", "[style]" ) {
    auto receiver = std::make_unique<Style>();
    auto overlay = std::make_unique<Style>();

    SECTION("with color values") {
        overlay->setValue(color, 0xFFFF0000);
        receiver->overlay(overlay.get());
        REQUIRE(overlay->getValue(color) == receiver->getValue(color));
    }

    SECTION("with string values") {
        overlay->setValue(fontFamily, "Arial");
        receiver->overlay(overlay.get());
        REQUIRE(overlay->getValue(fontFamily) == receiver->getValue(fontFamily));
    }

    SECTION("with float values") {
        overlay->setValue(opacity, 1.0f);
        receiver->overlay(overlay.get());
        REQUIRE(overlay->getValue(opacity) == receiver->getValue(opacity));
    }

    SECTION("with int values") {
        overlay->setValue(fontSize, 24);
        receiver->overlay(overlay.get());
        REQUIRE(overlay->getValue(fontSize) == receiver->getValue(fontSize));
    }

    SECTION("with bool values") {
        overlay->setValue(antiAlias, true);
        receiver->overlay(overlay.get());
        REQUIRE(overlay->getValue(antiAlias) == receiver->getValue(antiAlias));
    }

}

TEST_CASE( "Styles can have default values", "[style]" ) {
    auto receiver = std::make_unique<Style>();
    auto base = std::make_unique<Style>();

    SECTION("with color values") {
        base->setValue(color, 0xFFFF0000);
        receiver->defaults(base.get());
        REQUIRE(base->getValue(color) == receiver->getValue(color));
    }

    SECTION("with string values") {
        base->setValue(fontFamily, "Arial");
        receiver->defaults(base.get());
        REQUIRE(base->getValue(fontFamily) == receiver->getValue(fontFamily));
    }

    SECTION("with float values") {
        base->setValue(fontSize, 24);
        receiver->defaults(base.get());
        REQUIRE(base->getValue(fontSize) == receiver->getValue(fontSize));
    }

    SECTION("with int values") {
        base->setValue(opacity, 1.0f);
        receiver->defaults(base.get());
        REQUIRE(base->getValue(opacity) == receiver->getValue(opacity));
    }

    SECTION("with bool values") {
        base->setValue(antiAlias, true);
        receiver->defaults(base.get());
        REQUIRE(base->getValue(antiAlias) == receiver->getValue(antiAlias));
    }

    SECTION("not with already set color values") {
        receiver->setValue(color, 0xFF0000FF);
        base->setValue(color, 0xFFFF0000);
        receiver->defaults(base.get());
        REQUIRE(receiver->getValue(color) == 0xFF0000FF);
    }

    SECTION("not with already string values") {
        receiver->setValue(fontFamily, "Times");
        base->setValue(fontFamily, "Arial");
        receiver->defaults(base.get());
        REQUIRE(receiver->getValue(fontFamily) == "Times");
    }

    SECTION("not with already float values") {
        receiver->setValue(opacity, 0.5f);
        base->setValue(opacity, 1.0f);
        receiver->defaults(base.get());
        REQUIRE(receiver->getValue(opacity) == 0.5f);
    }

    SECTION("not with already int values") {
        receiver->setValue(fontSize, 12);
        base->setValue(fontSize, 24);
        receiver->defaults(base.get());
        REQUIRE(receiver->getValue(fontSize) == 12);
    }

    SECTION("not with already bool values") {
        receiver->setValue(antiAlias, false);
        base->setValue(antiAlias, true);
        receiver->defaults(base.get());
        REQUIRE(receiver->getValue(antiAlias) == false);
    }

}

TEST_CASE( "Styles can overlay inheritable values", "[style]" ) {
    auto receiver = std::make_unique<Style>();
    auto base = std::make_unique<Style>();

    SECTION("with color values") {
        base->setValue(color, 0xFFFF0000);
        base->setValue(backgroundColor, 0xFFFF0000);
        receiver->setValue(color, 0xFF0000FF);
        receiver->setValue(backgroundColor, 0xFF0000FF);
        receiver->overlayInheritable(base.get());
        REQUIRE(receiver->getValue(color) == 0xFFFF0000);
        REQUIRE(receiver->getValue(backgroundColor) == 0xFF0000FF);
    }

    SECTION("with string values") {
        base->setValue(fontFamily, "Arial");
        base->setValue(overflow, "hidden");
        receiver->setValue(fontFamily, "Times");
        receiver->setValue(overflow, "scroll");
        receiver->overlayInheritable(base.get());
        REQUIRE(receiver->getValue(fontFamily) == "Arial");
        REQUIRE(receiver->getValue(overflow) == "scroll");
    }

    SECTION("with float values") {
        base->setValue(opacity, 0.5f);
//        base->setValue(opacity, "hidden");
        receiver->setValue(opacity, 1.0f);
//        receiver->setValue(opacity, "scroll");
        receiver->overlayInheritable(base.get());
        REQUIRE(receiver->getValue(opacity) == 1.0f);
//        REQUIRE(receiver->getValue(opacity) == "scroll");
    }

    SECTION("with int values") {
        base->setValue(fontSize, 12);
        base->setValue(cornerRadius, 12);
        receiver->setValue(fontSize, 24);
        receiver->setValue(cornerRadius, 24);
        receiver->overlayInheritable(base.get());
        REQUIRE(receiver->getValue(fontSize) == 12);
        REQUIRE(receiver->getValue(cornerRadius) == 24);
    }

    SECTION("with bool values") {
        base->setValue(antiAlias, false);
//        base->setValue(antiAlias, false);
        receiver->setValue(antiAlias, true);
//        receiver->setValue(antiAlias, true);
        receiver->overlayInheritable(base.get());
        REQUIRE(receiver->getValue(antiAlias) == true);
//        REQUIRE(receiver->getValue(antiAlias) == false);
    }

}
