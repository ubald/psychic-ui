#include <chrono>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/animation/Timing.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/Toast.hpp>
#include <psychic-ui/components/Tree.hpp>
#include <psychic-ui/skins/float/FloatSkins.hpp> // IWYU pragma: keep: skinSet<FloatSkins>(), which include-cleaner misses
#include <psychic-ui/style/Length.hpp>
#include <psychic-ui/style/Properties.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace std::chrono_literals;
using Catch::Approx;

namespace {
    constexpr Timing slide{100ms, Ease::Linear};

    struct LayoutWindow : Window {
        LayoutWindow() : Window("Layout") {
            setStyleManager(std::make_shared<StyleManager>());
            setWindowSize(400, 300);
        }

        void paint(SkCanvas *canvas) { render(canvas); }
    };

    /**
     * A div that notes when it is painted
     */
    struct Painted : Div {
        std::vector<const Div *> &log;

        explicit Painted(std::vector<const Div *> &log) : log(log) {}

        void draw(SkCanvas *canvas) override {
            Div::draw(canvas);
            log.push_back(this);
        }
    };

    /**
     * A column of three 20 px rows whose container slides them (`layoutTransition`), in a window of its own laid out at
     * 400 × 300, driven by frames at explicit times
     */
    struct Column {
        std::shared_ptr<LayoutWindow> window = std::make_shared<LayoutWindow>();
        Div                     *list{nullptr};
        Div                     *a{nullptr};
        Div                     *b{nullptr};
        Div                     *c{nullptr};
        std::vector<const Div *> painted{};

        Column() {
            list = &window->appContainer().add<Div>();
            list->layoutTransition(slide);
            a = &list->add<Painted>(painted).height(20).shrink(0);
            b = &list->add<Painted>(painted).height(20).shrink(0);
            c = &list->add<Painted>(painted).height(20).shrink(0);
            frame(0ms);
        }

        /**
         * The order its rows are painted in
         */
        std::vector<const Div *> paint() {
            painted.clear();
            const auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(400, 300));
            window->paint(surface->getCanvas());
            return painted;
        }

        /**
         * What drawAll does at `time`, before painting
         */
        void frame(std::chrono::nanoseconds time) {
            window->updateAnimations(time);
            window->updateStylesAndLayout();
        }

        /**
         * Where it is painted in its container
         */
        static float paintedY(const Div &div) { return static_cast<float>(div.y()) + div.layoutOffsetY(); }

        /**
         * Move a child to another place in the container (a sort)
         */
        void move(Div &child, std::size_t index) const {
            auto keep = child.shared_from_this();
            list->remove(child);
            list->add(index, keep);
        }

        [[nodiscard]] bool settled() const {
            return !window->animating() && !window->needsFrame();
        }
    };
}

TEST_CASE("Nothing slides on the first layout", "[animation][layout-transition]") {
    const Column fixture{};
    CHECK(fixture.b->y() == 20);
    CHECK(fixture.b->layoutOffsetY() == 0.0f);
    CHECK_FALSE(fixture.b->layoutSliding());
    CHECK(fixture.settled());
}

TEST_CASE("A child that a layout moves slides from where it was painted, and the window idles once it is there", "[animation][layout-transition]") {
    Column fixture{};
    fixture.a->height(50);
    fixture.frame(0ms);
    // Laid out once, at its new place, and painted where it was
    CHECK(fixture.b->y() == 50);
    CHECK(fixture.b->layoutOffsetY() == Approx(-30.0f));
    CHECK(fixture.c->layoutOffsetY() == Approx(-30.0f));
    CHECK(fixture.a->layoutOffsetY() == 0.0f); // It grew, it didn't move
    CHECK(fixture.b->layoutSliding());
    CHECK(fixture.window->animating());

    fixture.frame(50ms);
    CHECK(Column::paintedY(*fixture.b) == Approx(35.0f));
    fixture.frame(100ms);
    CHECK(fixture.b->layoutOffsetY() == 0.0f);
    CHECK(fixture.c->layoutOffsetY() == 0.0f);
    CHECK_FALSE(fixture.b->layoutSliding());
    CHECK(fixture.settled());
}

TEST_CASE("Children have the `sliding` class while they slide, and paint over those that don't", "[animation][layout-transition]") {
    Column fixture{};
    CHECK(fixture.paint() == std::vector<const Div *>{fixture.a, fixture.b, fixture.c});

    fixture.move(*fixture.a, 1); // b, a, c: b goes up, a down, c stays
    fixture.frame(0ms);
    // Styled with it before the paint that starts the slide
    CHECK(fixture.b->classNames().contains("sliding"));
    CHECK(fixture.a->classNames().contains("sliding"));
    CHECK_FALSE(fixture.c->classNames().contains("sliding"));
    CHECK(fixture.window->styleManager()->valid());
    CHECK_FALSE(fixture.window->styleDirty());
    // Those that don't slide first, then those that do, each in document order
    CHECK(fixture.paint() == std::vector<const Div *>{fixture.c, fixture.b, fixture.a});

    fixture.frame(50ms);
    CHECK(fixture.a->classNames().contains("sliding"));
    fixture.frame(100ms);
    CHECK_FALSE(fixture.a->classNames().contains("sliding"));
    CHECK_FALSE(fixture.b->classNames().contains("sliding"));
    CHECK(fixture.paint() == std::vector<const Div *>{fixture.b, fixture.a, fixture.c});
    CHECK(fixture.settled());
}

TEST_CASE("A slide retargeted midway continues from where it is painted", "[animation][layout-transition]") {
    Column fixture{};
    fixture.a->height(50);
    fixture.frame(0ms);
    fixture.frame(50ms);
    REQUIRE(Column::paintedY(*fixture.b) == Approx(35.0f));

    // Back: from 35 to 20, without a jump
    fixture.a->height(20);
    fixture.frame(50ms);
    CHECK(fixture.b->y() == 20);
    CHECK(Column::paintedY(*fixture.b) == Approx(35.0f));
    fixture.frame(100ms);
    CHECK(Column::paintedY(*fixture.b) == Approx(27.5f));
    fixture.frame(150ms);
    CHECK(Column::paintedY(*fixture.b) == Approx(20.0f));
    CHECK(fixture.settled());
}

TEST_CASE("Reordered children slide to their new places; one added appears in place", "[animation][layout-transition]") {
    Column fixture{};
    fixture.move(*fixture.c, 0); // c, a, b
    auto &added = fixture.list->add<Div>().height(20);
    fixture.frame(0ms);
    CHECK(fixture.c->y() == 0);
    CHECK(fixture.c->layoutOffsetY() == Approx(40.0f));
    CHECK(fixture.a->layoutOffsetY() == Approx(-20.0f));
    CHECK(fixture.b->layoutOffsetY() == Approx(-20.0f));
    CHECK(added.y() == 60);
    CHECK(added.layoutOffsetY() == 0.0f);
    fixture.frame(100ms);
    CHECK(fixture.c->layoutOffsetY() == 0.0f);
    CHECK(fixture.settled());

    // One removed while the others slide: they close the gap
    fixture.list->remove(*fixture.a);
    fixture.frame(100ms);
    CHECK(fixture.b->layoutOffsetY() == Approx(20.0f));
    CHECK(added.layoutOffsetY() == Approx(20.0f));
}

TEST_CASE("A child shown again appears in place", "[animation][layout-transition]") {
    Column fixture{};
    fixture.b->display(Display::None);
    fixture.frame(0ms);
    fixture.frame(100ms);
    fixture.b->display(Display::Flex);
    fixture.frame(100ms);
    CHECK(fixture.b->layoutOffsetY() == 0.0f); // Shown again: in place
    CHECK(fixture.c->layoutOffsetY() == Approx(-20.0f));
}

TEST_CASE("Scrolling never slides anything", "[animation][layout-transition]") {
    Column fixture{};
    fixture.list->height(30).overflow(Overflow::Scroll);
    fixture.frame(0ms);
    fixture.frame(100ms);
    REQUIRE(fixture.settled());
    // Scrolled and laid out again in the same frame: only the layout's move slides
    fixture.list->scrollY(-20);
    fixture.a->height(30);
    fixture.frame(100ms);
    CHECK(fixture.b->layoutOffsetY() == Approx(-10.0f));
    CHECK(fixture.a->layoutOffsetY() == 0.0f);
}

TEST_CASE("Slides compose: a child doesn't slide with its container twice", "[animation][layout-transition]") {
    Column fixture{};
    // b holds a column of its own that slides too
    fixture.b->height(Auto);
    fixture.b->layoutTransition(slide);
    auto &inner = fixture.b->add<Div>().height(10);
    auto &next  = fixture.b->add<Div>().height(10);
    fixture.frame(0ms);
    fixture.frame(100ms);
    REQUIRE(fixture.settled());

    fixture.a->height(40);
    fixture.frame(100ms);
    CHECK(fixture.b->layoutOffsetY() == Approx(-20.0f));
    // It moved with b, which carries it
    CHECK(inner.layoutOffsetY() == 0.0f);
    CHECK(next.layoutOffsetY() == 0.0f);

    // Midway, something moves inside b too: only that slides, from where it is painted
    fixture.frame(150ms);
    inner.height(20);
    fixture.frame(150ms);
    CHECK(next.layoutOffsetY() == Approx(-10.0f));
    CHECK(inner.layoutOffsetY() == 0.0f);
}

TEST_CASE("Nothing slides with reduced motion, a new window size or new rules", "[animation][layout-transition]") {
    SECTION("Reduced motion") {
        Column fixture{};
        fixture.window->reducedMotion(true);
        fixture.a->height(50);
        fixture.frame(0ms);
        CHECK(fixture.b->layoutOffsetY() == 0.0f);
        CHECK(fixture.settled());
    }
    SECTION("Reduced motion ends a slide") {
        Column fixture{};
        fixture.a->height(50);
        fixture.frame(0ms);
        REQUIRE(fixture.b->layoutSliding());
        fixture.window->reducedMotion(true);
        CHECK(fixture.b->layoutOffsetY() == 0.0f);
        CHECK_FALSE(fixture.b->layoutSliding());
    }
    SECTION("A window resize") {
        Column fixture{};
        fixture.list->direction(Direction::Row).wrap(Wrap::Wrap);
        fixture.a->width(150);
        fixture.b->width(150);
        fixture.c->width(150);
        fixture.frame(0ms);
        fixture.frame(100ms);
        REQUIRE(fixture.c->y() == 20); // Wrapped
        fixture.window->setWindowSize(500, 300);
        fixture.frame(100ms);
        REQUIRE(fixture.c->y() == 0);
        CHECK(fixture.c->layoutOffsetX() == 0.0f);
        CHECK(fixture.c->layoutOffsetY() == 0.0f);
    }
    SECTION("New rules: a theme") {
        Column fixture{};
        fixture.a->height(50);
        fixture.window->styleManager()->invalidate();
        fixture.frame(0ms);
        REQUIRE(fixture.b->y() == 50);
        CHECK(fixture.b->layoutOffsetY() == 0.0f);
    }
    SECTION("A container without a layout transition") {
        Column fixture{};
        fixture.list->layoutTransition(Timing{});
        fixture.a->height(50);
        fixture.frame(0ms);
        CHECK(fixture.b->layoutOffsetY() == 0.0f);
    }
}

// region Components

namespace {
    constexpr Motion motion = themes::pivotalDark.metrics.motion;

    /**
     * The default skin set's layout transitions, with motion, driven by frames at explicit times
     */
    struct Components {
        std::shared_ptr<psychic_ui_tests::TestWindow> window{std::make_shared<psychic_ui_tests::TestWindow>()};
        psychic_ui_tests::FakeSystemWindow            systemWindow{window, 800, 600};

        Components() {
            window->reducedMotion(false);
        }

        /**
         * What drawAll does at `time`, then a raster render
         */
        void frame(std::chrono::nanoseconds time) {
            window->updatePopups(time);
            window->updateAnimations(time);
            systemWindow.layout();
        }

        [[nodiscard]] bool settled() const {
            return !window->animating() && !window->needsFrame();
        }

        /**
         * Whether it paints a background
         */
        static bool filled(const Div &div) {
            return div.computedStyle().has(Property::backgroundColor) && div.computedStyle().backgroundColor().alpha() > 0;
        }

        static int windowY(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y, 0, 0);
            return y;
        }
    };

    bool hasClass(const Div &div, const std::string &name) {
        return div.classNames().contains(name);
    }
}

TEST_CASE("A sorted grid's rows slide to their new places, keeping the selection and the focus", "[animation][layout-transition][datagrid]") {
    Components fixture{};
    auto &grid = fixture.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {"Size", 80}});
    auto &rowC = grid.row({"c", "3"});
    auto &rowB = grid.row({"b", "2"});
    auto &rowA = grid.row({"a", "1"});
    fixture.frame(0ms);
    fixture.frame(500ms);
    REQUIRE(fixture.settled());
    grid.select(&rowB);
    fixture.window->requestFocus(&rowB);
    const int rowCY = Components::windowY(rowC);
    const int rowAY = Components::windowY(rowA);
    fixture.frame(500ms);

    grid.sort(0, SortOrder::Ascending);
    fixture.frame(500ms);
    // Laid out in their new places, painted in their old ones
    CHECK(Components::windowY(rowA) == rowCY);
    CHECK(rowA.layoutOffsetY() == Approx(static_cast<float>(rowAY - rowCY)));
    CHECK(rowC.layoutOffsetY() == Approx(static_cast<float>(rowCY - rowAY)));
    CHECK(rowB.layoutOffsetY() == 0.0f); // In the middle, where it was
    CHECK(grid.selectedRow() == &rowB);
    CHECK(rowB.hasFocus());

    // At the slow step, since they travel across the grid: still going when a list's rows would be there
    fixture.frame(500ms + motion.slow / 2);
    CHECK(std::abs(rowA.layoutOffsetY()) > 0.0f);
    CHECK(std::abs(rowA.layoutOffsetY()) < static_cast<float>(rowAY - rowCY));
    fixture.frame(500ms + motion.medium);
    CHECK(rowA.layoutSliding());
    fixture.frame(500ms + motion.slow);
    CHECK(rowA.layoutOffsetY() == 0.0f);
    CHECK(rowC.layoutOffsetY() == 0.0f);
    // Their opaque background blends back to nothing over the page, which looks the same
    fixture.frame(500ms + motion.slow + motion.fast);
    CHECK(fixture.settled());
}

TEST_CASE("Rows sliding are opaque in the page's color, the selection in its own fill", "[animation][layout-transition][datagrid]") {
    Components fixture{};
    const Palette &palette = themes::pivotalDark.palette;
    auto &grid = fixture.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {"Size", 80}});
    auto &rowC = grid.row({"c", "3"});
    auto &rowB = grid.row({"b", "2"});
    auto &rowA = grid.row({"a", "1"});
    fixture.frame(0ms);
    fixture.frame(500ms);
    grid.select(&rowA);
    fixture.frame(500ms);
    fixture.frame(1000ms);
    REQUIRE(fixture.settled());
    REQUIRE_FALSE(Components::filled(rowC));

    grid.sort(0, SortOrder::Ascending);
    fixture.frame(1000ms);
    REQUIRE(rowC.layoutSliding());
    CHECK(rowC.classNames().contains("sliding"));
    CHECK(rowC.computedStyle().backgroundColor() == palette.page); // At once, from the first frame
    CHECK(rowA.computedStyle().backgroundColor() == palette.raised.over(palette.page));
    CHECK(rowA.computedStyle().borderLeftColor() == palette.accent); // Its accent bar stays
    CHECK_FALSE(rowB.layoutSliding());
    CHECK_FALSE(Components::filled(rowB));

    // There: the class goes, the fill blends back to nothing over the page
    fixture.frame(2000ms);
    CHECK_FALSE(rowC.classNames().contains("sliding"));
    fixture.frame(2000ms + motion.fast);
    CHECK_FALSE(Components::filled(rowC));
    CHECK(fixture.settled());
    CHECK(rowA.computedStyle().backgroundColor() == palette.raised);
}

TEST_CASE("Float's selection, translucent, is made opaque while it slides", "[animation][layout-transition][datagrid][float]") {
    Components fixture{};
    fixture.window->skinSet<FloatSkins>();
    const Palette &palette = fixture.window->styleManager()->theme().palette;
    REQUIRE(palette.accentSoft.alpha() < 255);
    auto &grid = fixture.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {"Size", 80}});
    auto &rowC = grid.row({"c", "3"});
    grid.row({"b", "2"});
    auto &rowA = grid.row({"a", "1"});
    grid.select(&rowA);
    fixture.frame(0ms);
    fixture.frame(1000ms);
    REQUIRE(fixture.settled());

    grid.sort(0, SortOrder::Ascending);
    fixture.frame(1000ms);
    REQUIRE(rowA.layoutSliding());
    CHECK(rowA.computedStyle().backgroundColor() == palette.accentSoft.over(palette.page));
    CHECK(rowA.computedStyle().backgroundColor().alpha() == 255);
    CHECK(rowC.computedStyle().backgroundColor() == palette.page);
}

TEST_CASE("A tree's nodes sliding are opaque in the page's color, over the children fading in", "[animation][layout-transition][tree]") {
    Components fixture{};
    auto &tree = fixture.window->appContainer().add<Tree>();
    auto &src  = tree.node("src", "folder");
    src.node("main.cpp", "file");
    auto &readme = tree.node("README.md", "file");
    fixture.frame(0ms);
    fixture.frame(500ms);
    REQUIRE(fixture.settled());

    src.expanded(true);
    fixture.frame(500ms);
    REQUIRE(readme.layoutSliding());
    CHECK(readme.computedStyle().backgroundColor() == themes::pivotalDark.palette.page);
    CHECK_FALSE(src.classNames().contains("sliding"));
    CHECK_FALSE(Components::filled(src));
}

TEST_CASE("A tree's rows below a node slide down as it expands, up as it collapses", "[animation][layout-transition][tree]") {
    Components fixture{};
    auto &tree = fixture.window->appContainer().add<Tree>();
    auto &src  = tree.node("src", "folder");
    auto &main = src.node("main.cpp", "file");
    src.node("util.cpp", "file");
    auto &readme = tree.node("README.md", "file");
    const Div &children = *src.children()[1];
    fixture.frame(0ms);
    fixture.frame(500ms);
    REQUIRE(fixture.settled());
    const int before = Components::windowY(readme);

    src.expanded(true);
    fixture.frame(500ms);
    const int after = Components::windowY(readme);
    REQUIRE(after > before);
    CHECK(readme.layoutOffsetY() == Approx(static_cast<float>(before - after)));
    // The children appear in place, fading in
    CHECK(main.layoutOffsetY() == 0.0f);
    CHECK(children.computedStyle().opacity() == 0.0f);
    fixture.frame(500ms + motion.medium / 2);
    CHECK(readme.layoutOffsetY() < 0.0f);
    CHECK(children.computedStyle().opacity() > 0.0f);
    fixture.frame(500ms + motion.medium);
    CHECK(readme.layoutOffsetY() == 0.0f);
    CHECK(children.computedStyle().opacity() == 1.0f);
    CHECK(fixture.settled());

    // Collapsed: they go at once, the rows below slide up
    src.expanded(false);
    fixture.frame(1000ms);
    CHECK(Components::windowY(readme) == before);
    CHECK(readme.layoutOffsetY() == Approx(static_cast<float>(after - before)));
    fixture.frame(1000ms + motion.medium);
    CHECK(readme.layoutOffsetY() == 0.0f);
    fixture.frame(1000ms + motion.medium);
    CHECK(fixture.settled());

    // Expanded again: its children appear in place again
    src.expanded(true);
    fixture.frame(2000ms);
    CHECK(main.layoutOffsetY() == 0.0f);
    CHECK(readme.layoutOffsetY() < 0.0f);
}

TEST_CASE("Toasts slide into the place of one that leaves, which fades where it is", "[animation][layout-transition][toast]") {
    Components fixture{};
    const Toast &first  = fixture.window->toast({.title = "First", .timeout = 0s});
    Toast       &second = fixture.window->toast({.title = "Second", .timeout = 0s});
    Toast       &third  = fixture.window->toast({.title = "Third", .timeout = 0s});
    fixture.frame(0ms);
    fixture.frame(500ms);
    REQUIRE(fixture.settled());
    const int firstY  = Components::windowY(first);
    const int secondY = Components::windowY(second);

    // From the middle: it fades where it is, out of the flow, and the one above slides down into its place
    second.dismiss();
    CHECK(hasClass(second, "in-place"));
    fixture.frame(500ms);
    CHECK(Components::windowY(second) == secondY);
    CHECK(Components::windowY(first) == secondY);
    CHECK(first.layoutOffsetY() == Approx(static_cast<float>(firstY - secondY)));
    CHECK(third.layoutOffsetY() == 0.0f);
    fixture.frame(500ms + motion.medium / 2);
    CHECK(second.computedStyle().translateY().resolve(100) == 0.0f);
    CHECK(second.computedStyle().opacity() < 1.0f);
    CHECK(first.layoutOffsetY() < 0.0f);
    fixture.frame(500ms + motion.medium);
    fixture.frame(500ms + motion.medium);
    CHECK(fixture.window->toasts().children().size() == 2);
    CHECK(first.layoutOffsetY() == 0.0f);
    CHECK(fixture.settled());

    // The bottom one slides back under the edge, and the one above follows it down (dismissed at 1000 ms: what it
    // starts starts then)
    fixture.frame(1000ms);
    const int thirdY = Components::windowY(third);
    third.dismiss();
    CHECK_FALSE(hasClass(third, "in-place"));
    fixture.frame(1000ms);
    CHECK(Components::windowY(first) == thirdY);
    CHECK(first.layoutOffsetY() < 0.0f);
    fixture.frame(1000ms + motion.medium / 2);
    CHECK(third.computedStyle().translateY().value > 0.0f);

    // A new one pushes the others up: they slide
    fixture.frame(2000ms);
    fixture.frame(2000ms);
    REQUIRE(fixture.window->toasts().children().size() == 1);
    const int lastY = Components::windowY(first);
    fixture.window->toast({.title = "Fourth", .timeout = 0s});
    fixture.frame(2000ms);
    CHECK(Components::windowY(first) < lastY);
    CHECK(first.layoutOffsetY() == Approx(static_cast<float>(lastY - Components::windowY(first))));
    fixture.frame(3000ms);
    CHECK(fixture.settled());
}

// endregion
