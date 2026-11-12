#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/Div.hpp>

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
    styleManager->style(".tall")->set(height, 40.0f);
    styleManager->style(".box:hover")->set(height, 60.0f);
    auto window = makeWindow(styleManager);
    window->updateStylesAndLayout();

    GIVEN("a visible div") {
        auto div = window->appContainer()->add<Div>();
        div->addClassName("box");
        window->updateStylesAndLayout();
        REQUIRE(div->getHeight() == 0);

        WHEN("a class changes its height") {
            div->addClassName("tall");
            window->updateStylesAndLayout();

            THEN("the same pass lays it out with the new height") {
                REQUIRE(div->getHeight() == 40);
            }
        }

        WHEN("hovering changes its height") {
            div->setMouseOver(true);
            window->updateStylesAndLayout();

            THEN("the same pass lays it out with the new height") {
                REQUIRE(div->getHeight() == 60);
            }
        }
    }

    GIVEN("a hidden div followed by a sibling") {
        auto hidden  = window->appContainer()->add<Div>();
        auto sibling = window->appContainer()->add<Div>();
        hidden->setVisible(false);
        sibling->style()->set(height, 10.0f);
        window->updateStylesAndLayout();
        REQUIRE(sibling->y() == 0);

        WHEN("a class changes the hidden div's height") {
            hidden->addClassName("tall");
            window->updateStylesAndLayout();

            THEN("it reaches the layout, since hidden divs still take part in it") {
                REQUIRE(hidden->getHeight() == 40);
                REQUIRE(sibling->y() == 40);
            }
        }
    }

    GIVEN("a div far outside the window") {
        auto offscreen = window->appContainer()->add<Div>();
        offscreen->style()->set(position, "absolute")->set(left, 5000.0f)->set(top, 5000.0f);
        window->updateStylesAndLayout();

        WHEN("a class changes its height") {
            offscreen->addClassName("tall");
            window->updateStylesAndLayout();

            THEN("it reaches the layout") {
                REQUIRE(offscreen->getHeight() == 40);
            }
        }
    }
}

SCENARIO("the style pass only visits invalidated subtrees", "[layout]") {
    auto styleManager = std::make_shared<StyleManager>();
    auto window       = makeWindow(styleManager);
    auto left         = window->appContainer()->add<Div>();
    auto counted      = left->add<CountingDiv>();
    auto right        = window->appContainer()->add<Div>();
    window->updateStylesAndLayout();
    int updates = counted->styleUpdates;

    WHEN("an unrelated div is invalidated") {
        right->addClassName("other");
        window->updateStylesAndLayout();

        THEN("the clean subtree is not restyled") {
            REQUIRE(counted->styleUpdates == updates);
            REQUIRE_FALSE(window->styleDirty());
        }
    }

    WHEN("its parent is invalidated") {
        left->addClassName("other");
        window->updateStylesAndLayout();

        THEN("it is restyled with its parent") {
            REQUIRE(counted->styleUpdates == updates + 1);
        }
    }
}

TEST_CASE("a styleUpdated that always invalidates doesn't hang the frame", "[layout]") {
    auto styleManager = std::make_shared<StyleManager>();
    auto window       = makeWindow(styleManager);
    window->appContainer()->add<AlwaysDirtyDiv>();
    window->updateStylesAndLayout();
    REQUIRE(window->styleDirty());
}
