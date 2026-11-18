#include <string>
#include <catch2/catch_test_macros.hpp>
#include <psychic-color/spaces/HSB.hpp>
#include <psychic-ui/style/Style.hpp>

using namespace psychic_ui;

// region Misuse doesn't compile

namespace {
    // Each concept is true when the call compiles. Templates, so that a call that doesn't compile is `false`
    // instead of an error.
    template<class S> concept VisibleFromString = requires(S s) { s.visible("false"); };
    template<class S> concept VisibleFromInt = requires(S s) { s.visible(1); };
    template<class S> concept VisibleFromBool = requires(S s) { s.visible(false); };
    template<class S> concept ColorFromInt = requires(S s) { s.color(0xFF0000); };
    template<class S> concept ColorFromRGB = requires(S s) { s.color(rgb(0xFF0000)); };
    template<class S> concept ColorFromHSB = requires(S s, psychic_color::HSB hsb) { s.color(hsb); };
    template<class S> concept FontFromZero = requires(S s) { s.fontFamily(0); };
    template<class S> concept FontFromNull = requires(S s) { s.fontFamily(nullptr); };
    template<class S> concept FontFromLiteral = requires(S s) { s.fontFamily("Arial"); };
    template<class S> concept FontFromString = requires(S s, std::string f) { s.fontFamily(f); };
    template<class S> concept WidthFromString = requires(S s) { s.width("100"); };
    template<class S> concept WidthFromBool = requires(S s) { s.width(true); };
    template<class S> concept WidthFromInt = requires(S s) { s.width(100); };
    template<class S> concept WidthFromFloat = requires(S s) { s.width(1.5f); };
    template<class S> concept OpacityFromString = requires(S s) { s.opacity("0.5"); };
    template<class S> concept OpacityFromBool = requires(S s) { s.opacity(true); };
    template<class S> concept OverflowFromString = requires(S s) { s.overflow("scroll"); };
    template<class S> concept OverflowFromInt = requires(S s) { s.overflow(2); };
    template<class S> concept CursorFromFloat = requires(S s) { s.cursor(2.9f); };
    template<class S> concept DirectionFromWrongEnum = requires(S s) { s.direction(TextDirection::Rtl); };
    template<class E> concept HasScrol = requires { E::Scrol; };
    template<class E> concept HasScroll = requires { E::Scroll; };
    template<class S> concept HasPaddingPercent = requires(S s) { s.paddingPercent(0.5f); };
    template<class S> concept HasFlexDirection = requires(S s) { s.flexDirection(Direction::Row); };
}

static_assert(!VisibleFromString<Style>, "a const char * doesn't convert to bool");
static_assert(!VisibleFromInt<Style>);
static_assert(VisibleFromBool<Style>);
static_assert(!ColorFromInt<Style>, "a bare integer doesn't say whether it is ARGB or RGB");
static_assert(ColorFromRGB<Style>);
static_assert(ColorFromHSB<Style>, "psychic-color colors convert");
static_assert(!FontFromZero<Style>, "0 would be a null const char *");
static_assert(!FontFromNull<Style>);
static_assert(FontFromLiteral<Style>);
static_assert(FontFromString<Style>);
static_assert(!WidthFromString<Style>);
static_assert(!WidthFromBool<Style>);
static_assert(WidthFromInt<Style>);
static_assert(WidthFromFloat<Style>);
static_assert(!OpacityFromString<Style>);
static_assert(!OpacityFromBool<Style>);
static_assert(!OverflowFromString<Style>, "enumerated values are scoped enums, not strings");
static_assert(!OverflowFromInt<Style>);
static_assert(!CursorFromFloat<Style>, "no float to int truncation");
static_assert(!DirectionFromWrongEnum<Style>);
static_assert(!HasScrol<Overflow>, "a misspelled value is a compile error");
static_assert(HasScroll<Overflow>);
static_assert(!HasPaddingPercent<Style>, "a length property exists once, in any unit");
static_assert(!HasFlexDirection<Style>, "the flex axis is `direction`");

// endregion

TEST_CASE("lengths", "[style][values]") {
    SECTION("a number is pixels") {
        constexpr Length l = 24;
        STATIC_REQUIRE(l.isPx());
        STATIC_REQUIRE(l.value == 24.0f);
        STATIC_REQUIRE(Length{1.5} == Length::px(1.5f));
        STATIC_REQUIRE(24_px == Length{24});
        STATIC_REQUIRE(2.5_px == Length::px(2.5f));
    }

    SECTION("percents are 0-100, like CSS") {
        constexpr Length l = 50_pct;
        STATIC_REQUIRE(l.isPercent());
        STATIC_REQUIRE(l.value == 50.0f);
        STATIC_REQUIRE(12.5_pct == Length::percent(12.5f));
        STATIC_REQUIRE(l.resolve(200.0f) == 100.0f);
    }

    SECTION("auto is a unit, not a NaN") {
        STATIC_REQUIRE(Auto.isAuto());
        STATIC_REQUIRE(Auto == Auto);
        STATIC_REQUIRE(Auto != Length{0});
        STATIC_REQUIRE(Auto.resolve(200.0f, 7.0f) == 7.0f);
    }

    SECTION("the same value in another unit differs") {
        STATIC_REQUIRE(Length{50} != 50_pct);
    }
}

TEST_CASE("colors", "[style][values]") {
    SECTION("rgb is opaque, argb says its alpha") {
        STATIC_REQUIRE(rgb(0xFF0000).argb() == 0xFFFF0000);
        STATIC_REQUIRE(rgb(0x12, 0x34, 0x56).argb() == 0xFF123456);
        STATIC_REQUIRE(argb(0x80FF0000).alpha() == 0x80);
        STATIC_REQUIRE(rgb(0xFF0000).withAlpha(0x40) == argb(0x40FF0000));
    }

    SECTION("psychic-color colors convert, opaque") {
        const psychic_color::HSB hsb{0x0C83EB};
        REQUIRE(Color{hsb}.argb() == (0xFF000000u | hsb.getColor()));
        REQUIRE(Color{hsb}.withAlpha(0x40).argb() == hsb.getColorAlpha(0x40));
    }
}

TEST_CASE("atoms", "[style][values]") {
    Atom a{"Ubuntu Regular"};
    Atom b{std::string("Ubuntu ") + "Regular"};
    REQUIRE(a == b);
    REQUIRE(&a.str() == &b.str());
    REQUIRE(a == "Ubuntu Regular");
    REQUIRE(Atom{} == "");
    REQUIRE(Atom{}.empty());
    REQUIRE_FALSE(a == Atom{"Ubuntu Light"});
}
