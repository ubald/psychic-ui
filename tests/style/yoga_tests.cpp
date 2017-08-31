#include "catch.hpp"
#include <iostream>
#include <yoga/Yoga.h>

TEST_CASE( "Making sure yoga does what we want" ) {
    YGConfigSetUseWebDefaults(YGConfigGetDefault(), true);
    auto container = YGNodeNew();
    auto node = YGNodeNew();
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
}