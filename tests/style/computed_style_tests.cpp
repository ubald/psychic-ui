#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <yoga/Yoga.h>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/style/StyleManager.hpp>

using namespace psychic_ui;

namespace {
    // Exposes the derived state and the Yoga node
    class TestDiv : public Div {
    public:
        using Div::_yogaNode;
        using Div::_radiusTopLeft;
        using Div::_radiusTopRight;
        using Div::_radiusBottomLeft;
        using Div::_radiusBottomRight;
        using Div::_drawBackground;
        using Div::layoutUpdated;
    };

    struct Tree {
        std::shared_ptr<StyleManager> styleManager = std::make_shared<StyleManager>();
        std::shared_ptr<TestDiv>      root         = std::make_shared<TestDiv>();
        std::shared_ptr<TestDiv>      div          = std::make_shared<TestDiv>();

        Tree() {
            root->setStyleManager(styleManager);
            root->width(200).height(200);
            root->add(div);
            layout();
        }

        void restyle() const {
            root->updateDirtyStyles();
        }

        void layout() const {
            restyle();
            YGNodeCalculateLayout(root->_yogaNode, YGUndefined, YGUndefined, YGDirectionLTR);
            root->layoutUpdated();
        }
    };
}

TEST_CASE("derived state follows the computed style both ways", "[style][computed]") {
    Tree t;
    t.styleManager->style(".hidden").visible(false);
    t.styleManager->style(".round").borderRadius(12);
    t.styleManager->style(".corner").borderRadiusTopLeft(4);
    t.styleManager->style(".tinted").backgroundColor(rgb(0xFF0000));
    t.styleManager->style(".box:hover").borderRadius(6);
    t.restyle();

    SECTION("visibility comes back when the rule hiding it stops matching") {
        t.div->addClassName("hidden");
        t.restyle();
        REQUIRE_FALSE(t.div->visible());
        t.div->removeClassName("hidden");
        t.restyle();
        REQUIRE(t.div->visible());
    }

    SECTION("radii go back to 0 when the rules setting them stop matching") {
        t.div->addClassName("round");
        t.div->addClassName("corner");
        t.restyle();
        REQUIRE(t.div->_radiusTopLeft == 4);
        REQUIRE(t.div->_radiusBottomRight == 12);

        t.div->removeClassName("corner");
        t.restyle();
        REQUIRE(t.div->_radiusTopLeft == 12);

        t.div->removeClassName("round");
        t.restyle();
        REQUIRE(t.div->_radiusTopLeft == 0);
        REQUIRE(t.div->_radiusTopRight == 0);
        REQUIRE(t.div->_radiusBottomLeft == 0);
        REQUIRE(t.div->_radiusBottomRight == 0);
    }

    SECTION("a :hover radius goes away with the hover") {
        t.div->addClassName("box");
        t.div->setMouseOver(true);
        t.restyle();
        REQUIRE(t.div->_radiusTopLeft == 6);
        t.div->setMouseOver(false);
        t.restyle();
        REQUIRE(t.div->_radiusTopLeft == 0);
    }

    SECTION("the background goes away with the rule") {
        t.div->addClassName("tinted");
        t.restyle();
        REQUIRE(t.div->_drawBackground);
        t.div->removeClassName("tinted");
        t.restyle();
        REQUIRE_FALSE(t.div->_drawBackground);
    }
}

TEST_CASE("only the changed layout properties reach Yoga", "[style][computed][layout]") {
    Tree t;
    t.styleManager->style(".tinted").backgroundColor(rgb(0xFF0000)).borderRadius(4).color(rgb(0x00FF00));
    t.styleManager->style(".tall").height(40);
    t.div->width(100);
    t.layout();
    REQUIRE(t.div->getWidth() == 100);
    REQUIRE_FALSE(YGNodeIsDirty(t.root->_yogaNode));

    // Change the node behind the style's back: a property pushed again would undo this
    YGNodeStyleSetWidth(t.div->_yogaNode, 77);
    YGNodeCalculateLayout(t.root->_yogaNode, YGUndefined, YGUndefined, YGDirectionLTR);
    REQUIRE_FALSE(YGNodeIsDirty(t.root->_yogaNode));

    SECTION("a paint-only change doesn't touch the Yoga node") {
        t.div->addClassName("tinted");
        t.restyle();
        REQUIRE(t.div->computedStyle().backgroundColor() == rgb(0xFF0000));
        REQUIRE_FALSE(YGNodeIsDirty(t.div->_yogaNode));
        REQUIRE_FALSE(YGNodeIsDirty(t.root->_yogaNode));
        REQUIRE(YGNodeStyleGetWidth(t.div->_yogaNode).value == 77);
    }

    SECTION("a layout change pushes that property only") {
        t.div->addClassName("tall");
        t.restyle();
        REQUIRE(YGNodeIsDirty(t.root->_yogaNode));
        REQUIRE(YGNodeStyleGetHeight(t.div->_yogaNode).value == 40);
        REQUIRE(YGNodeStyleGetWidth(t.div->_yogaNode).value == 77);
    }

    SECTION("removing a layout rule pushes the default back") {
        t.div->addClassName("tall");
        t.layout();
        REQUIRE(t.div->getHeight() == 40);
        t.div->removeClassName("tall");
        t.layout();
        REQUIRE(YGNodeStyleGetHeight(t.div->_yogaNode).unit == YGUnitAuto);
        REQUIRE(t.div->getHeight() == 0);
    }
}

TEST_CASE("lengths reach Yoga in their unit", "[style][computed][layout]") {
    Tree t;
    t.root->padding(10);

    SECTION("percent is 0-100") {
        t.div->width(50_pct).height(25_pct);
        t.layout();
        REQUIRE(YGNodeStyleGetWidth(t.div->_yogaNode).unit == YGUnitPercent);
        REQUIRE(YGNodeStyleGetWidth(t.div->_yogaNode).value == 50);
        REQUIRE(t.div->getWidth() == 90);
        REQUIRE(t.div->getHeight() == 45);
    }

    SECTION("auto margins center") {
        t.div->width(80).height(20).marginHorizontal(Auto);
        t.layout();
        REQUIRE(t.div->x() == 60);
    }

    SECTION("the most specific edge wins, an unset edge falls back") {
        t.div->height(20).margin(8).marginLeft(2);
        t.layout();
        REQUIRE(t.div->x() == 12);
        REQUIRE(t.div->y() == 18);
    }
}
