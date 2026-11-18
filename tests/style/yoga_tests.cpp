#include <array>
#include <cstddef>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <yoga/Yoga.h>

TEST_CASE( "Making sure yoga does what we want" ) {
    auto *config = YGConfigNew();
    YGConfigSetUseWebDefaults(config, true);
    auto *container = YGNodeNewWithConfig(config);
    auto *child = YGNodeNewWithConfig(config);
    YGNodeInsertChild(container, child, YGNodeGetChildCount(container));


    SECTION("specific margin overrides less specific") {
        YGNodeStyleSetWidth(child, 100);
        YGNodeStyleSetHeight(child, 100);

        YGNodeStyleSetMargin(child, YGEdgeAll, 48);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(child) == 48);
        REQUIRE(YGNodeLayoutGetMargin(child, YGEdgeLeft) == 48);

        YGNodeStyleSetMargin(child, YGEdgeHorizontal, 24);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(child) == 24);
        REQUIRE(YGNodeLayoutGetMargin(child, YGEdgeLeft) == 24);

        YGNodeStyleSetMargin(child, YGEdgeLeft, 12);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(child) == 12);
        REQUIRE(YGNodeLayoutGetMargin(child, YGEdgeLeft) == 12);
    }

    SECTION("specific padding overrides less specific") {
        YGNodeStyleSetWidth(child, 100);
        YGNodeStyleSetHeight(child, 100);

        YGNodeStyleSetPadding(container, YGEdgeAll, 48);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(child) == 48);
        REQUIRE(YGNodeLayoutGetPadding(container, YGEdgeLeft) == 48);

        YGNodeStyleSetPadding(container, YGEdgeHorizontal, 24);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(child) == 24);
        REQUIRE(YGNodeLayoutGetPadding(container, YGEdgeLeft) == 24);

        YGNodeStyleSetPadding(container, YGEdgeLeft, 12);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(child) == 12);
        REQUIRE(YGNodeLayoutGetPadding(container, YGEdgeLeft) == 12);
    }

    YGNodeFreeRecursive(container);
    YGConfigFree(config);
}

TEST_CASE( "Wrapping container keeps the cross axis margin of flex-start items" ) {
    // Same settings as the config every Div uses (Div.cpp)
    auto *config = YGConfigNew();
    YGConfigSetUseWebDefaults(config, true);
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
    YGConfigSetPointScaleFactor(config, 0.0f);

    auto *container = YGNodeNewWithConfig(config);
    YGNodeStyleSetFlexDirection(container, YGFlexDirectionRow);
    YGNodeStyleSetFlexWrap(container, YGWrapWrap);
    YGNodeStyleSetAlignItems(container, YGAlignFlexStart);
    YGNodeStyleSetWidth(container, 300);

    // Two items with a margin on the first line, a third without one wraps onto the second
    std::array<YGNodeRef, 3> items{};
    for (std::size_t i = 0; i < items.size(); ++i) {
        YGNodeRef item = YGNodeNewWithConfig(config);
        YGNodeStyleSetWidth(item, 100);
        YGNodeStyleSetHeight(item, 40);
        if (i < 2) {
            YGNodeStyleSetMargin(item, YGEdgeAll, 12);
        }
        YGNodeInsertChild(container, item, i);
        items.at(i) = item;
    }

    YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionLTR);

    REQUIRE(YGNodeLayoutGetLeft(items[0]) == 12);
    REQUIRE(YGNodeLayoutGetTop(items[0]) == 12);
    REQUIRE(YGNodeLayoutGetLeft(items[1]) == 136);
    REQUIRE(YGNodeLayoutGetTop(items[1]) == 12);
    REQUIRE(YGNodeLayoutGetLeft(items[2]) == 0);
    REQUIRE(YGNodeLayoutGetTop(items[2]) == 64);
    REQUIRE(YGNodeLayoutGetHeight(container) == 104);

    YGNodeFreeRecursive(container);
    YGConfigFree(config);
}
