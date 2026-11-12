#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/style/Style.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Skin.hpp>

using namespace psychic_ui;

TEST_CASE( "Style values can be set and retrieved", "[style]" ) {
    Style style;

    SECTION("with color values") {
        REQUIRE_FALSE(style.has(Property::color));
        REQUIRE(style.color() == argb(0xFF000000));
        style.color(argb(0xFFFF0000));
        REQUIRE(style.has(Property::color));
        REQUIRE(style.color() == argb(0xFFFF0000));
    }

    SECTION("with string values") {
        REQUIRE(style.fontFamily() == "");
        style.fontFamily("Arial");
        REQUIRE(style.fontFamily() == "Arial");
        REQUIRE(style.fontFamily() == Atom{"Arial"});
    }

    SECTION("with float values") {
        REQUIRE_FALSE(style.has(Property::opacity));
        REQUIRE(style.opacity() == 1.0f);
        style.opacity(0.5f);
        REQUIRE(style.opacity() == 0.5f);
    }

    SECTION("with length values") {
        REQUIRE(style.width() == Auto);
        style.width(24);
        REQUIRE(style.width() == Length::px(24));
        style.width(50_pct);
        REQUIRE(style.width() == Length::percent(50));
    }

    SECTION("with enumerated values") {
        REQUIRE(style.overflow() == Overflow::Visible);
        style.overflow(Overflow::Scroll);
        REQUIRE(style.overflow() == Overflow::Scroll);
    }

    SECTION("with bool values") {
        REQUIRE(!style.antiAlias());
        style.antiAlias(true);
        REQUIRE(style.antiAlias());
    }

    SECTION("and unset") {
        style.padding(12).color(argb(0xFFFF0000));
        style.unset(Property::padding);
        REQUIRE_FALSE(style.has(Property::padding));
        REQUIRE(style.padding() == Length{});
        REQUIRE(style.has(Property::color));
        style.clear();
        REQUIRE(style.properties().none());
        REQUIRE(style == Style{});
    }
}

TEST_CASE( "Styles can be overlayed", "[style]" ) {
    Style receiver;
    Style overlay;

    SECTION("with color values") {
        overlay.color(argb(0xFFFF0000));
        receiver.overlay(overlay);
        REQUIRE(overlay.color() == receiver.color());
    }

    SECTION("with string values") {
        overlay.fontFamily("Arial");
        receiver.overlay(overlay);
        REQUIRE(overlay.fontFamily() == receiver.fontFamily());
    }

    SECTION("with float values") {
        overlay.opacity(1.0f);
        receiver.overlay(overlay);
        REQUIRE(overlay.opacity() == receiver.opacity());
    }

    SECTION("with bool values") {
        overlay.antiAlias(true);
        receiver.overlay(overlay);
        REQUIRE(overlay.antiAlias() == receiver.antiAlias());
    }

    SECTION("only with what the overlay sets") {
        receiver.padding(4).margin(8);
        overlay.padding(12);
        receiver.overlay(overlay);
        REQUIRE(receiver.padding() == 12);
        REQUIRE(receiver.margin() == 8);
    }

    SECTION("only with the properties asked for") {
        overlay.padding(12).margin(8);
        receiver.overlay(overlay, mask({Property::margin}));
        REQUIRE_FALSE(receiver.has(Property::padding));
        REQUIRE(receiver.margin() == 8);
    }
}

TEST_CASE( "Styles can have default values", "[style]" ) {
    Style receiver;
    Style base;

    SECTION("with color values") {
        base.color(argb(0xFFFF0000));
        receiver.defaults(base);
        REQUIRE(base.color() == receiver.color());
    }

    SECTION("with string values") {
        base.fontFamily("Arial");
        receiver.defaults(base);
        REQUIRE(base.fontFamily() == receiver.fontFamily());
    }

    SECTION("with float values") {
        base.opacity(0.5f);
        receiver.defaults(base);
        REQUIRE(base.opacity() == receiver.opacity());
    }

    SECTION("with bool values") {
        base.antiAlias(true);
        receiver.defaults(base);
        REQUIRE(base.antiAlias() == receiver.antiAlias());
    }

    SECTION("not with already set color values") {
        receiver.color(argb(0xFF0000FF));
        base.color(argb(0xFFFF0000));
        receiver.defaults(base);
        REQUIRE(receiver.color() == argb(0xFF0000FF));
    }

    SECTION("not with already string values") {
        receiver.fontFamily("Times");
        base.fontFamily("Arial");
        receiver.defaults(base);
        REQUIRE(receiver.fontFamily() == "Times");
    }

    SECTION("not with already float values") {
        receiver.opacity(0.5f);
        base.opacity(1.0f);
        receiver.defaults(base);
        REQUIRE(receiver.opacity() == 0.5f);
    }

    SECTION("not with already set length values") {
        receiver.padding(12);
        base.padding(24);
        receiver.defaults(base);
        REQUIRE(receiver.padding() == 12);
    }

    SECTION("not with already bool values") {
        receiver.antiAlias(false);
        base.antiAlias(true);
        receiver.defaults(base);
        REQUIRE(receiver.antiAlias() == false);
    }
}

TEST_CASE( "Styles can overlay inheritable values", "[style]" ) {
    Style receiver;
    Style inheritable;
    auto  div = std::make_shared<Div>();

    SECTION("with color values") {
        inheritable.color(argb(0xFFFF0000));
        inheritable.backgroundColor(argb(0xFFFF0000));
        receiver.color(argb(0xFF0000FF));
        receiver.backgroundColor(argb(0xFF0000FF));
        receiver.overlay(inheritable, div->inheritedProperties());
        REQUIRE(receiver.color() == argb(0xFFFF0000));
        REQUIRE(receiver.backgroundColor() == argb(0xFF0000FF));
    }

    SECTION("with string values") {
        inheritable.fontFamily("Arial");
        inheritable.overflow(Overflow::Hidden);
        receiver.fontFamily("Times");
        receiver.overflow(Overflow::Scroll);
        receiver.overlay(inheritable, div->inheritedProperties());
        REQUIRE(receiver.fontFamily() == "Arial");
        REQUIRE(receiver.overflow() == Overflow::Scroll);
    }

    SECTION("with float values") {
        inheritable.opacity(0.5f);
        receiver.opacity(1.0f);
        receiver.overlay(inheritable, div->inheritedProperties());
        REQUIRE(receiver.opacity() == 1.0f);
    }

    SECTION("with bool values") {
        inheritable.antiAlias(false);
        receiver.antiAlias(true);
        receiver.overlay(inheritable, div->inheritedProperties());
        REQUIRE(receiver.antiAlias() == true);
    }
}

TEST_CASE( "Styles compare by value, auto included", "[style]" ) {
    // With NaN as the auto sentinel, a style holding auto wasn't even equal to itself
    Style a;
    a.width(Auto).lineHeight(Auto);
    REQUIRE(a == a);

    Style b;
    b.width(Auto).lineHeight(Auto);
    REQUIRE(a == b);

    SECTION("an unset property differs from one set to its default") {
        Style c;
        REQUIRE(c != a);
        REQUIRE(c.diff(a) == mask({Property::width, Property::lineHeight}));
    }

    SECTION("diff lists the changed properties") {
        b.width(24).color(argb(0xFFFF0000));
        REQUIRE(a.diff(b) == mask({Property::width, Property::color}));
        REQUIRE(b.diff(a) == a.diff(b));
    }
}

TEST_CASE( "A style notifies its owner only when a value changes", "[style]" ) {
    int   changes = 0;
    Style style{[](void *count) { ++*static_cast<int *>(count); }, &changes};

    style.width(Auto);
    style.width(Auto);
    style.width(Auto);
    REQUIRE(changes == 1);

    style.width(24);
    REQUIRE(changes == 2);

    SECTION("copying a style copies its values, not its owner") {
        Style copy{style};
        copy.width(48);
        REQUIRE(changes == 2);

        style = copy;
        REQUIRE(changes == 3);
        REQUIRE(style.width() == 48);
    }
}

TEST_CASE( "Every property has a name", "[style]" ) {
    REQUIRE(propertyName(Property::color) == "color");
    REQUIRE(propertyName(Property::borderBottom) == "borderBottom");
    REQUIRE(propertyName(Property::direction) == "direction");
    REQUIRE(propertyName(Property::textDirection) == "textDirection");
}

TEST_CASE( "Only layout properties are pushed to Yoga", "[style]" ) {
    REQUIRE(layoutProperties.test(index(Property::width)));
    REQUIRE(layoutProperties.test(index(Property::direction)));
    REQUIRE(layoutProperties.test(index(Property::padding)));
    REQUIRE_FALSE(layoutProperties.test(index(Property::color)));
    REQUIRE_FALSE(layoutProperties.test(index(Property::borderRadius)));
    REQUIRE_FALSE(layoutProperties.test(index(Property::visible)));
    REQUIRE((layoutProperties & paintProperties).none());
    REQUIRE((layoutProperties | paintProperties).all());
}

TEST_CASE( "Inherited properties follow CSS", "[style]" ) {
    for (auto property: {Property::color, Property::fontFamily, Property::fontSize, Property::lineHeight, Property::cursor}) {
        CAPTURE(propertyName(property));
        REQUIRE(inheritedProperties.test(index(property)));
    }
    for (auto property: {Property::backgroundColor, Property::padding, Property::width, Property::visible}) {
        CAPTURE(propertyName(property));
        REQUIRE_FALSE(inheritedProperties.test(index(property)));
    }
}
