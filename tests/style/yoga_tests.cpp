#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <yoga/Yoga.h>

TEST_CASE( "Making sure yoga does what we want" ) {
    auto config = YGConfigNew();
    YGConfigSetUseWebDefaults(config, true);
    auto container = YGNodeNewWithConfig(config);
    auto node = YGNodeNewWithConfig(config);
    YGNodeInsertChild(container, node, YGNodeGetChildCount(container));


    SECTION("specific margin overrides less specific") {
        YGNodeStyleSetWidth(node, 100);
        YGNodeStyleSetHeight(node, 100);

        YGNodeStyleSetMargin(node, YGEdgeAll, 48);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(node) == 48);
        REQUIRE(YGNodeLayoutGetMargin(node, YGEdgeLeft) == 48);

        YGNodeStyleSetMargin(node, YGEdgeHorizontal, 24);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(node) == 24);
        REQUIRE(YGNodeLayoutGetMargin(node, YGEdgeLeft) == 24);

        YGNodeStyleSetMargin(node, YGEdgeLeft, 12);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(node) == 12);
        REQUIRE(YGNodeLayoutGetMargin(node, YGEdgeLeft) == 12);
    }

    SECTION("specific padding overrides less specific") {
        YGNodeStyleSetWidth(node, 100);
        YGNodeStyleSetHeight(node, 100);

        YGNodeStyleSetPadding(container, YGEdgeAll, 48);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(node) == 48);
        REQUIRE(YGNodeLayoutGetPadding(container, YGEdgeLeft) == 48);

        YGNodeStyleSetPadding(container, YGEdgeHorizontal, 24);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(node) == 24);
        REQUIRE(YGNodeLayoutGetPadding(container, YGEdgeLeft) == 24);

        YGNodeStyleSetPadding(container, YGEdgeLeft, 12);
        YGNodeCalculateLayout(container, YGUndefined, YGUndefined, YGDirectionRTL);
        REQUIRE(YGNodeLayoutGetLeft(node) == 12);
        REQUIRE(YGNodeLayoutGetPadding(container, YGEdgeLeft) == 12);
    }

    YGNodeFreeRecursive(container);
    YGConfigFree(config);
}

TEST_CASE( "Wrapping container keeps the cross axis margin of flex-start items" ) {
    // Same settings as the config every Div uses (Div.cpp)
    auto config = YGConfigNew();
    YGConfigSetUseWebDefaults(config, true);
    YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
    YGConfigSetPointScaleFactor(config, 0.0f);

    auto container = YGNodeNewWithConfig(config);
    YGNodeStyleSetFlexDirection(container, YGFlexDirectionRow);
    YGNodeStyleSetFlexWrap(container, YGWrapWrap);
    YGNodeStyleSetAlignItems(container, YGAlignFlexStart);
    YGNodeStyleSetWidth(container, 300);

    // Two items with a margin on the first line, a third without one wraps onto the second
    YGNodeRef items[3];
    for (int i = 0; i < 3; ++i) {
        items[i] = YGNodeNewWithConfig(config);
        YGNodeStyleSetWidth(items[i], 100);
        YGNodeStyleSetHeight(items[i], 40);
        if (i < 2) {
            YGNodeStyleSetMargin(items[i], YGEdgeAll, 12);
        }
        YGNodeInsertChild(container, items[i], i);
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
