#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/ToolBar.hpp>

using namespace psychic_ui;

// The window's frame without the canvas: `updateStylesAndLayout` is what `drawAll` runs before painting,
// so these tests need no GL context.

namespace {
    std::shared_ptr<Window> makeWindow(const std::shared_ptr<StyleManager> &styleManager) {
        auto window = std::make_shared<Window>("test");
        window->setStyleManager(styleManager);
        window->setWindowSize(800, 600);
        return window;
    }

    class CountingDiv : public Div {
    public:
        int styleUpdates{0};

    protected:
        void styleUpdated() override {
            Div::styleUpdated();
            ++styleUpdates;
        }
    };

    class AlwaysDirtyDiv : public Div {
    protected:
        void styleUpdated() override {
            Div::styleUpdated();
            invalidateStyle();
        }
    };
}

SCENARIO("styles are computed before layout, visible or not", "[layout]") {
    auto styleManager = std::make_shared<StyleManager>();
    styleManager->style(".tall").height(40.0f);
    styleManager->style(".box:hover").height(60.0f);
    auto window = makeWindow(styleManager);
    window->updateStylesAndLayout();

    GIVEN("a visible div") {
        auto &div = window->appContainer().add<Div>();
        div.addClassName("box");
        window->updateStylesAndLayout();
        REQUIRE(div.getHeight() == 0);

        WHEN("a class changes its height") {
            div.addClassName("tall");
            window->updateStylesAndLayout();

            THEN("the same pass lays it out with the new height") {
                REQUIRE(div.getHeight() == 40);
            }
        }

        WHEN("hovering changes its height") {
            div.setMouseOver(true);
            window->updateStylesAndLayout();

            THEN("the same pass lays it out with the new height") {
                REQUIRE(div.getHeight() == 60);
            }
        }
    }

    GIVEN("a hidden div followed by a sibling") {
        auto &hidden = window->appContainer().add<Div>();
        auto &sibling = window->appContainer().add<Div>();
        hidden.visible(false);
        sibling.height(10.0f);
        window->updateStylesAndLayout();
        REQUIRE(sibling.y() == 0);

        WHEN("a class changes the hidden div's height") {
            hidden.addClassName("tall");
            window->updateStylesAndLayout();

            THEN("it reaches the layout, since hidden divs still take part in it") {
                REQUIRE(hidden.getHeight() == 40);
                REQUIRE(sibling.y() == 40);
            }
        }
    }

    GIVEN("a div far outside the window") {
        auto &offscreen = window->appContainer().add<Div>();
        offscreen.position(Position::Absolute).left(5000.0f).top(5000.0f);
        window->updateStylesAndLayout();

        WHEN("a class changes its height") {
            offscreen.addClassName("tall");
            window->updateStylesAndLayout();

            THEN("it reaches the layout") {
                REQUIRE(offscreen.getHeight() == 40);
            }
        }
    }
}

SCENARIO("the style pass only visits invalidated subtrees", "[layout]") {
    auto styleManager = std::make_shared<StyleManager>();
    auto window       = makeWindow(styleManager);
    auto &left = window->appContainer().add<Div>();
    auto &counted = left.add<CountingDiv>();
    auto &right = window->appContainer().add<Div>();
    window->updateStylesAndLayout();
    const int updates = counted.styleUpdates;

    WHEN("an unrelated div is invalidated") {
        right.addClassName("other");
        window->updateStylesAndLayout();

        THEN("the clean subtree is not restyled") {
            REQUIRE(counted.styleUpdates == updates);
            REQUIRE_FALSE(window->styleDirty());
        }
    }

    WHEN("its parent is invalidated") {
        left.addClassName("other");
        window->updateStylesAndLayout();

        THEN("it is restyled with its parent") {
            REQUIRE(counted.styleUpdates == updates + 1);
        }
    }
}

TEST_CASE("a styleUpdated that always invalidates doesn't hang the frame", "[layout]") {
    auto styleManager = std::make_shared<StyleManager>();
    auto window       = makeWindow(styleManager);
    window->appContainer().add<AlwaysDirtyDiv>();
    window->updateStylesAndLayout();
    REQUIRE(window->styleDirty());
}

SCENARIO("the setters write the inline style, the only input of the layout", "[layout]") {
    auto styleManager = std::make_shared<StyleManager>();
    auto window       = makeWindow(styleManager);

    GIVEN("a div given a height with setHeight") {
        auto &div = window->appContainer().add<Div>();
        div.addClassName("box");
        div.height(48);
        window->updateStylesAndLayout();
        REQUIRE(div.getHeight() == 48);

        WHEN("its style is updated") {
            div.addClassName("other");
            window->updateStylesAndLayout();

            THEN("it keeps its height") {
                REQUIRE(div.getHeight() == 48);
            }
        }

        WHEN("a stylesheet gives it another height") {
            styleManager->style(".box").height(10.0f);
            window->updateStylesAndLayout();

            THEN("the inline height wins") {
                REQUIRE(div.getHeight() == 48);
            }
        }
    }

    GIVEN("a toolbar") {
        auto &toolBar = window->appContainer().add<ToolBar>();
        toolBar.addClassName("other");
        window->updateStylesAndLayout();

        THEN("it keeps its 48 px height through style updates") {
            REQUIRE(toolBar.getHeight() == 48);
        }
    }

    GIVEN("an absolute div placed with setX and setY") {
        auto &div = window->appContainer().add<Div>();
        div.position(Position::Absolute);
        div.size(10, 10);

        WHEN("the position is negative") {
            div.left(-5);
            div.top(-7);
            window->updateStylesAndLayout();

            THEN("the layout places it there") {
                REQUIRE(div.x() == -5);
                REQUIRE(div.y() == -7);
                REQUIRE(div.getWidth() == 10);
            }
        }
    }
}

SCENARIO("the root is laid out at the window size", "[layout]") {
    auto styleManager = std::make_shared<StyleManager>();
    auto window       = makeWindow(styleManager);
    auto &child = window->appContainer().add<Div>();
    child.height(100_pct);
    window->updateStylesAndLayout();
    REQUIRE(window->getWidth() == 800);
    REQUIRE(window->getHeight() == 600);
    REQUIRE(child.getWidth() == 800);

    WHEN("the window is resized") {
        window->setWindowSize(1024, 768);
        window->updateStylesAndLayout();

        THEN("the same pass lays out the root at the new size") {
            REQUIRE(window->getWidth() == 1024);
            REQUIRE(window->getHeight() == 768);
            REQUIRE(child.getHeight() == 768);
        }
    }

    WHEN("the window is resized and the stylesheet changes in the same frame") {
        window->setWindowSize(1024, 768);
        styleManager->style("#app").padding(4.0f);
        window->updateStylesAndLayout();

        THEN("the root is laid out at the new size with the new style") {
            REQUIRE(window->getWidth() == 1024);
            REQUIRE(window->getHeight() == 768);
            REQUIRE(window->appContainer().getWidth() == 1024);
            REQUIRE(child.x() == 4);
            REQUIRE(child.getWidth() == 1024 - 8);
        }
    }
}

TEST_CASE("a scrolling div's content ends with its end padding, as in CSS", "[layout]") {
    auto styleManager = std::make_shared<StyleManager>();
    auto window       = makeWindow(styleManager);
    auto &scroller    = window->appContainer().add<Div>()
                                .width(100.0f)
                                .height(100.0f)
                                .padding(10.0f)
                                .overflow(Overflow::Scroll);
    scroller.add<Div>().height(300.0f).shrink(0);
    window->updateStylesAndLayout();

    // 10 above, 300 of content, 10 below
    CHECK(scroller.contentHeight() == 320);
    scroller.scrollBy(0, -1000);
    CHECK(scroller.scrollY() == 100 - 320);

    // Without scrolling, the padding isn't added to what overflows
    scroller.overflow(Overflow::Hidden);
    window->updateStylesAndLayout();
    CHECK(scroller.contentHeight() == 310);
}
