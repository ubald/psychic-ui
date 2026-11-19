#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp> // IWYU pragma: keep: skinSet<PivotalSkins>(), which include-cleaner misses
#include <psychic-ui/style/Length.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include "../../example/demo-stylesheet.hpp"
#include "../../example/demo/DragAndDrop.hpp"
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    /**
     * The demo's Drag and Drop page filling a window with a style manager of its own
     */
    struct Fixture {
        std::shared_ptr<TestWindow> window{make()};
        FakeSystemWindow            systemWindow{window, 1000, 800};
        DragAndDropPage             *page{nullptr};

        static std::shared_ptr<TestWindow> make() {
            auto window = std::make_shared<TestWindow>();
            window->setStyleManager(std::make_shared<StyleManager>());
            window->skinSet<PivotalSkins>();
            window->loadStyleSheet<DemoStyleSheet>();
            return window;
        }

        Fixture() {
            page = &window->appContainer().add<DragAndDropPage>().size(100_pct, 100_pct);
            systemWindow.layout();
        }

        /**
         * The first div under the page with this class whose label (if `text` isn't empty) says it, depth first
         */
        [[nodiscard]] Div *find(const std::string &className, const std::string &text = "") const {
            std::function<Div *(Div *)> search = [&](Div *div) -> Div * {
                for (const auto &child: div->children()) {
                    if (child->classNames().contains(className) && (text.empty() || says(*child, text))) {
                        return child.get();
                    }
                    // NOLINTNEXTLINE(misc-const-correctness): returned as a mutable pointer, which the check misses
                    if (Div *found = search(child.get())) {
                        return found;
                    }
                }
                return nullptr;
            };
            return search(page);
        }

        static bool says(const Div &div, const std::string &text) {
            return std::ranges::any_of(div.children(), [&text](const auto &child) {
                const auto *label = dynamic_cast<const Label *>(child.get());
                return label && label->text() == text;
            });
        }

        static std::pair<float, float> center(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y);
            return {static_cast<float>(x) + (static_cast<float>(div.getWidth()) / 2.0f),
                    static_cast<float>(y) + (static_cast<float>(div.getHeight()) / 2.0f)};
        }

        /**
         * Drags a chip by the mouse onto a div
         */
        void drag(const Div &chip, const Div &onto) {
            const auto [fromX, fromY] = center(chip);
            const auto [toX, toY]     = center(onto);
            window->handleMouseButtonChange({.x = fromX, .y = fromY, .button = MouseButton::LEFT, .down = true});
            window->handleMouseMove({.x = fromX + 10, .y = fromY}, false);
            window->handleMouseMove({.x = toX, .y = toY}, false);
            window->handleMouseButtonChange({.x = toX, .y = toY, .button = MouseButton::LEFT, .down = false});
            systemWindow.layout();
        }
    };
}

TEST_CASE("the demo's Drag and Drop page takes chips and zooms", "[demo][drag][zoom]") {
    Fixture fixture;
    const Div *red   = fixture.find("chip", "Red");
    const Div *blue  = fixture.find("chip", "Blue");
    const Div *every = fixture.find("drop-box", "Every colour");
    const Div *warm  = fixture.find("drop-box", "Warm colours only");
    REQUIRE(red != nullptr);
    REQUIRE(blue != nullptr);
    REQUIRE(every != nullptr);
    REQUIRE(warm != nullptr);
    Div *everyDropped = fixture.find("dropped");
    REQUIRE(everyDropped != nullptr);
    REQUIRE(everyDropped->parent() == every);
    const auto dropped = [](const Div &box) { return box.children().back()->children().size(); };

    // Blue goes in the box that takes every colour, not in the warm one
    fixture.drag(*blue, *warm);
    CHECK(dropped(*warm) == 0);
    fixture.drag(*blue, *every);
    CHECK(dropped(*every) == 1);
    fixture.drag(*red, *warm);
    CHECK(dropped(*warm) == 1);

    // The canvas zooms, within its bounds, and says by how much
    const Div *canvas = fixture.find("zoom-canvas");
    REQUIRE(canvas != nullptr);
    const auto [x, y] = Fixture::center(*canvas);
    fixture.window->handleZoom({.x = x, .y = y, .scale = 2.0f});
    CHECK(fixture.page->zoom() == 2.0f);
    const auto *scale = dynamic_cast<const Label *>(fixture.find("scale"));
    REQUIRE(scale != nullptr);
    CHECK(scale->text() == "200 %");
    fixture.window->handleZoom({.x = x, .y = y, .scale = 10.0f});
    CHECK(fixture.page->zoom() == DragAndDropPage::maximumZoom);
}
