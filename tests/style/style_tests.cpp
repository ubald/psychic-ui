#include <cmath>
#include "catch2/catch.hpp"
#include <psychic-ui/style/Style.hpp>
#include <psychic-ui/Div.hpp>

using namespace psychic_ui;

TEST_CASE( "Style values can be set and retrieved", "[style]" ) {
    auto style = std::make_unique<Style>();

    SECTION("with color values") {
        REQUIRE(style->get(color) == 0xFF000000);
        style->set(color, 0xFFFF0000);
        REQUIRE(style->get(color) == 0xFFFF0000);
    }

    SECTION("with string values") {
        REQUIRE(style->get(fontFamily) == "");
        style->set(fontFamily, "Arial");
        REQUIRE(style->get(fontFamily) == "Arial");
    }

    SECTION("with float values") {
        REQUIRE(std::isnan(style->get(opacity)));
        style->set(opacity, 1.0f);
        REQUIRE(style->get(opacity) == 1.0f);
    }

//    SECTION("with int values") {
//        REQUIRE(style->get(fontSize) == 0);
//        style->set(fontSize, 24);
//        REQUIRE(style->get(fontSize) == 24);
//    }

    SECTION("with bool values") {
        REQUIRE(!style->get(antiAlias));
        style->set(antiAlias, true);
        REQUIRE(style->get(antiAlias));
    }
}

TEST_CASE( "Styles can be overlayed", "[style]" ) {
    auto receiver = std::make_unique<Style>();
    auto overlay = std::make_unique<Style>();

    SECTION("with color values") {
        overlay->set(color, 0xFFFF0000);
        receiver->overlay(overlay.get());
        REQUIRE(overlay->get(color) == receiver->get(color));
    }

    SECTION("with string values") {
        overlay->set(fontFamily, "Arial");
        receiver->overlay(overlay.get());
        REQUIRE(overlay->get(fontFamily) == receiver->get(fontFamily));
    }

    SECTION("with float values") {
        overlay->set(opacity, 1.0f);
        receiver->overlay(overlay.get());
        REQUIRE(overlay->get(opacity) == receiver->get(opacity));
    }

//    SECTION("with int values") {
//        overlay->set(fontSize, 24);
//        receiver->overlay(overlay.get());
//        REQUIRE(overlay->get(fontSize) == receiver->get(fontSize));
//    }

    SECTION("with bool values") {
        overlay->set(antiAlias, true);
        receiver->overlay(overlay.get());
        REQUIRE(overlay->get(antiAlias) == receiver->get(antiAlias));
    }

}

TEST_CASE( "Styles can have default values", "[style]" ) {
    auto receiver = std::make_unique<Style>();
    auto base = std::make_unique<Style>();

    SECTION("with color values") {
        base->set(color, 0xFFFF0000);
        receiver->defaults(base.get());
        REQUIRE(base->get(color) == receiver->get(color));
    }

    SECTION("with string values") {
        base->set(fontFamily, "Arial");
        receiver->defaults(base.get());
        REQUIRE(base->get(fontFamily) == receiver->get(fontFamily));
    }

    SECTION("with float values") {
        base->set(opacity, 0.5f);
        receiver->defaults(base.get());
        REQUIRE(base->get(opacity) == receiver->get(opacity));
    }

//    SECTION("with int values") {
//        base->set(opacity, 1.0f);
//        receiver->defaults(base.get());
//        REQUIRE(base->get(opacity) == receiver->get(opacity));
//    }

    SECTION("with bool values") {
        base->set(antiAlias, true);
        receiver->defaults(base.get());
        REQUIRE(base->get(antiAlias) == receiver->get(antiAlias));
    }

    SECTION("not with already set color values") {
        receiver->set(color, 0xFF0000FF);
        base->set(color, 0xFFFF0000);
        receiver->defaults(base.get());
        REQUIRE(receiver->get(color) == 0xFF0000FF);
    }

    SECTION("not with already string values") {
        receiver->set(fontFamily, "Times");
        base->set(fontFamily, "Arial");
        receiver->defaults(base.get());
        REQUIRE(receiver->get(fontFamily) == "Times");
    }

    SECTION("not with already float values") {
        receiver->set(opacity, 0.5f);
        base->set(opacity, 1.0f);
        receiver->defaults(base.get());
        REQUIRE(receiver->get(opacity) == 0.5f);
    }

    SECTION("not with already int values") {
        receiver->set(fontSize, 12);
        base->set(fontSize, 24);
        receiver->defaults(base.get());
        REQUIRE(receiver->get(fontSize) == 12);
    }

    SECTION("not with already bool values") {
        receiver->set(antiAlias, false);
        base->set(antiAlias, true);
        receiver->defaults(base.get());
        REQUIRE(receiver->get(antiAlias) == false);
    }

}

TEST_CASE( "Styles can overlay inheritable values", "[style]" ) {
    auto receiver = std::make_unique<Style>();
    auto inheritable = std::make_unique<Style>();
    auto div = std::make_shared<Div>();

    SECTION("with color values") {
        inheritable->set(color, 0xFFFF0000);
        inheritable->set(backgroundColor, 0xFFFF0000);
        receiver->set(color, 0xFF0000FF);
        receiver->set(backgroundColor, 0xFF0000FF);
        receiver->overlayInheritable(inheritable.get(), div.get());
        REQUIRE(receiver->get(color) == 0xFFFF0000);
        REQUIRE(receiver->get(backgroundColor) == 0xFF0000FF);
    }

    SECTION("with string values") {
        inheritable->set(fontFamily, "Arial");
        inheritable->set(overflow, "hidden");
        receiver->set(fontFamily, "Times");
        receiver->set(overflow, "scroll");
        receiver->overlayInheritable(inheritable.get(), div.get());
        REQUIRE(receiver->get(fontFamily) == "Arial");
        REQUIRE(receiver->get(overflow) == "scroll");
    }

    SECTION("with float values") {
        inheritable->set(opacity, 0.5f);
        receiver->set(opacity, 1.0f);
        receiver->overlayInheritable(inheritable.get(), div.get());
        REQUIRE(receiver->get(opacity) == 1.0f);
    }

//    SECTION("with int values") {
//        inheritable->set(fontSize, 12);
//        inheritable->set(cornerRadius, 12);
//        receiver->set(fontSize, 24);
//        receiver->set(cornerRadius, 24);
//        receiver->overlayInheritable(inheritable.get());
//        REQUIRE(receiver->get(fontSize) == 12);
//        REQUIRE(receiver->get(cornerRadius) == 24);
//    }

    SECTION("with bool values") {
        inheritable->set(antiAlias, false);
        receiver->set(antiAlias, true);
        receiver->overlayInheritable(inheritable.get(), div.get());
        REQUIRE(receiver->get(antiAlias) == true);
    }

}
