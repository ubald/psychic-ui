#include <chrono>
#include <cmath>
#include <memory>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/Toast.hpp>
#include <psychic-ui/components/Tree.hpp>
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

        Column() {
            list = &window->appContainer().add<Div>();
            list->layoutTransition(slide);
            a = &list->add<Div>().height(20).shrink(0);
            b = &list->add<Div>().height(20).shrink(0);
            c = &list->add<Div>().height(20).shrink(0);
            frame(0ms);
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
        void move(Div &child, std::size_t index) {
            auto keep = child.shared_from_this();
            list->remove(child);
            list->add(index, keep);
        }

        bool settled() const {
            return !window->animating() && !window->needsFrame();
        }
    };
}

TEST_CASE("Nothing slides on the first layout", "[animation][layout-transition]") {
    Column f{};
    CHECK(f.b->y() == 20);
    CHECK(f.b->layoutOffsetY() == 0.0f);
    CHECK_FALSE(f.b->layoutSliding());
    CHECK(f.settled());
}

TEST_CASE("A child that a layout moves slides from where it was painted, and the window idles once it is there", "[animation][layout-transition]") {
    Column f{};
    f.a->height(50);
    f.frame(0ms);
    // Laid out once, at its new place, and painted where it was
    CHECK(f.b->y() == 50);
    CHECK(f.b->layoutOffsetY() == Approx(-30.0f));
    CHECK(f.c->layoutOffsetY() == Approx(-30.0f));
    CHECK(f.a->layoutOffsetY() == 0.0f); // It grew, it didn't move
    CHECK(f.b->layoutSliding());
    CHECK(f.window->animating());

    f.frame(50ms);
    CHECK(Column::paintedY(*f.b) == Approx(35.0f));
    f.frame(100ms);
    CHECK(f.b->layoutOffsetY() == 0.0f);
    CHECK(f.c->layoutOffsetY() == 0.0f);
    CHECK_FALSE(f.b->layoutSliding());
    CHECK(f.settled());
}

TEST_CASE("A slide retargeted midway continues from where it is painted", "[animation][layout-transition]") {
    Column f{};
    f.a->height(50);
    f.frame(0ms);
    f.frame(50ms);
    REQUIRE(Column::paintedY(*f.b) == Approx(35.0f));

    // Back: from 35 to 20, without a jump
    f.a->height(20);
    f.frame(50ms);
    CHECK(f.b->y() == 20);
    CHECK(Column::paintedY(*f.b) == Approx(35.0f));
    f.frame(100ms);
    CHECK(Column::paintedY(*f.b) == Approx(27.5f));
    f.frame(150ms);
    CHECK(Column::paintedY(*f.b) == Approx(20.0f));
    CHECK(f.settled());
}

TEST_CASE("Reordered children slide to their new places; one added appears in place", "[animation][layout-transition]") {
    Column f{};
    f.move(*f.c, 0); // c, a, b
    auto &d = f.list->add<Div>().height(20);
    f.frame(0ms);
    CHECK(f.c->y() == 0);
    CHECK(f.c->layoutOffsetY() == Approx(40.0f));
    CHECK(f.a->layoutOffsetY() == Approx(-20.0f));
    CHECK(f.b->layoutOffsetY() == Approx(-20.0f));
    CHECK(d.y() == 60);
    CHECK(d.layoutOffsetY() == 0.0f);
    f.frame(100ms);
    CHECK(f.c->layoutOffsetY() == 0.0f);
    CHECK(f.settled());

    // One removed while the others slide: they close the gap
    f.list->remove(*f.a);
    f.frame(100ms);
    CHECK(f.b->layoutOffsetY() == Approx(20.0f));
    CHECK(d.layoutOffsetY() == Approx(20.0f));
}

TEST_CASE("A child shown again appears in place", "[animation][layout-transition]") {
    Column f{};
    f.b->display(Display::None);
    f.frame(0ms);
    f.frame(100ms);
    f.b->display(Display::Flex);
    f.frame(100ms);
    CHECK(f.b->layoutOffsetY() == 0.0f); // Shown again: in place
    CHECK(f.c->layoutOffsetY() == Approx(-20.0f));
}

TEST_CASE("Scrolling never slides anything", "[animation][layout-transition]") {
    Column f{};
    f.list->height(30).overflow(Overflow::Scroll);
    f.frame(0ms);
    f.frame(100ms);
    REQUIRE(f.settled());
    // Scrolled and laid out again in the same frame: only the layout's move slides
    f.list->scrollY(-20);
    f.a->height(30);
    f.frame(100ms);
    CHECK(f.b->layoutOffsetY() == Approx(-10.0f));
    CHECK(f.a->layoutOffsetY() == 0.0f);
}

TEST_CASE("Slides compose: a child doesn't slide with its container twice", "[animation][layout-transition]") {
    Column f{};
    // b holds a column of its own that slides too
    f.b->height(Auto);
    f.b->layoutTransition(slide);
    auto &inner = f.b->add<Div>().height(10);
    auto &next  = f.b->add<Div>().height(10);
    f.frame(0ms);
    f.frame(100ms);
    REQUIRE(f.settled());

    f.a->height(40);
    f.frame(100ms);
    CHECK(f.b->layoutOffsetY() == Approx(-20.0f));
    // It moved with b, which carries it
    CHECK(inner.layoutOffsetY() == 0.0f);
    CHECK(next.layoutOffsetY() == 0.0f);

    // Midway, something moves inside b too: only that slides, from where it is painted
    f.frame(150ms);
    inner.height(20);
    f.frame(150ms);
    CHECK(next.layoutOffsetY() == Approx(-10.0f));
    CHECK(inner.layoutOffsetY() == 0.0f);
}

TEST_CASE("Nothing slides with reduced motion, a new window size or new rules", "[animation][layout-transition]") {
    SECTION("Reduced motion") {
        Column f{};
        f.window->reducedMotion(true);
        f.a->height(50);
        f.frame(0ms);
        CHECK(f.b->layoutOffsetY() == 0.0f);
        CHECK(f.settled());
    }
    SECTION("Reduced motion ends a slide") {
        Column f{};
        f.a->height(50);
        f.frame(0ms);
        REQUIRE(f.b->layoutSliding());
        f.window->reducedMotion(true);
        CHECK(f.b->layoutOffsetY() == 0.0f);
        CHECK_FALSE(f.b->layoutSliding());
    }
    SECTION("A window resize") {
        Column f{};
        f.list->direction(Direction::Row).wrap(Wrap::Wrap);
        f.a->width(150);
        f.b->width(150);
        f.c->width(150);
        f.frame(0ms);
        f.frame(100ms);
        REQUIRE(f.c->y() == 20); // Wrapped
        f.window->setWindowSize(500, 300);
        f.frame(100ms);
        REQUIRE(f.c->y() == 0);
        CHECK(f.c->layoutOffsetX() == 0.0f);
        CHECK(f.c->layoutOffsetY() == 0.0f);
    }
    SECTION("New rules: a theme") {
        Column f{};
        f.a->height(50);
        f.window->styleManager()->invalidate();
        f.frame(0ms);
        REQUIRE(f.b->y() == 50);
        CHECK(f.b->layoutOffsetY() == 0.0f);
    }
    SECTION("A container without a layout transition") {
        Column f{};
        f.list->layoutTransition(Timing{});
        f.a->height(50);
        f.frame(0ms);
        CHECK(f.b->layoutOffsetY() == 0.0f);
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

        bool settled() const {
            return !window->animating() && !window->needsFrame();
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
    Components f{};
    auto &grid = f.window->appContainer().add<DataGrid>()
                  .height(300)
                  .columns({{"Name"}, {"Size", 80}});
    auto &c = grid.row({"c", "3"});
    auto &b = grid.row({"b", "2"});
    auto &a = grid.row({"a", "1"});
    f.frame(0ms);
    f.frame(500ms);
    REQUIRE(f.settled());
    grid.select(&b);
    f.window->requestFocus(&b);
    const int cY = Components::windowY(c);
    const int aY = Components::windowY(a);
    f.frame(500ms);

    grid.sort(0, SortOrder::Ascending);
    f.frame(500ms);
    // Laid out in their new places, painted in their old ones
    CHECK(Components::windowY(a) == cY);
    CHECK(a.layoutOffsetY() == Approx(static_cast<float>(aY - cY)));
    CHECK(c.layoutOffsetY() == Approx(static_cast<float>(cY - aY)));
    CHECK(b.layoutOffsetY() == 0.0f); // In the middle, where it was
    CHECK(grid.selectedRow() == &b);
    CHECK(b.hasFocus());

    f.frame(500ms + motion.medium / 2);
    CHECK(std::abs(a.layoutOffsetY()) > 0.0f);
    CHECK(std::abs(a.layoutOffsetY()) < static_cast<float>(aY - cY));
    f.frame(500ms + motion.medium);
    CHECK(a.layoutOffsetY() == 0.0f);
    CHECK(c.layoutOffsetY() == 0.0f);
    CHECK(f.settled());
}

TEST_CASE("A tree's rows below a node slide down as it expands, up as it collapses", "[animation][layout-transition][tree]") {
    Components f{};
    auto &tree   = f.window->appContainer().add<Tree>();
    auto &src    = tree.node("src", "folder");
    auto &main   = src.node("main.cpp", "file");
    src.node("util.cpp", "file");
    auto &readme = tree.node("README.md", "file");
    const Div &children = *src.children()[1];
    f.frame(0ms);
    f.frame(500ms);
    REQUIRE(f.settled());
    const int before = Components::windowY(readme);

    src.expanded(true);
    f.frame(500ms);
    const int after = Components::windowY(readme);
    REQUIRE(after > before);
    CHECK(readme.layoutOffsetY() == Approx(static_cast<float>(before - after)));
    // The children appear in place, fading in
    CHECK(main.layoutOffsetY() == 0.0f);
    CHECK(children.computedStyle().opacity() == 0.0f);
    f.frame(500ms + motion.medium / 2);
    CHECK(readme.layoutOffsetY() < 0.0f);
    CHECK(children.computedStyle().opacity() > 0.0f);
    f.frame(500ms + motion.medium);
    CHECK(readme.layoutOffsetY() == 0.0f);
    CHECK(children.computedStyle().opacity() == 1.0f);
    CHECK(f.settled());

    // Collapsed: they go at once, the rows below slide up
    src.expanded(false);
    f.frame(1000ms);
    CHECK(Components::windowY(readme) == before);
    CHECK(readme.layoutOffsetY() == Approx(static_cast<float>(after - before)));
    f.frame(1000ms + motion.medium);
    CHECK(readme.layoutOffsetY() == 0.0f);
    f.frame(1000ms + motion.medium);
    CHECK(f.settled());

    // Expanded again: its children appear in place again
    src.expanded(true);
    f.frame(2000ms);
    CHECK(main.layoutOffsetY() == 0.0f);
    CHECK(readme.layoutOffsetY() < 0.0f);
}

TEST_CASE("Toasts slide into the place of one that leaves, which fades where it is", "[animation][layout-transition][toast]") {
    Components f{};
    Toast &first  = f.window->toast({.title = "First", .timeout = 0s});
    Toast &second = f.window->toast({.title = "Second", .timeout = 0s});
    Toast &third  = f.window->toast({.title = "Third", .timeout = 0s});
    f.frame(0ms);
    f.frame(500ms);
    REQUIRE(f.settled());
    const int firstY  = Components::windowY(first);
    const int secondY = Components::windowY(second);

    // From the middle: it fades where it is, out of the flow, and the one above slides down into its place
    second.dismiss();
    CHECK(hasClass(second, "in-place"));
    f.frame(500ms);
    CHECK(Components::windowY(second) == secondY);
    CHECK(Components::windowY(first) == secondY);
    CHECK(first.layoutOffsetY() == Approx(static_cast<float>(firstY - secondY)));
    CHECK(third.layoutOffsetY() == 0.0f);
    f.frame(500ms + motion.medium / 2);
    CHECK(second.computedStyle().translateY().resolve(100) == 0.0f);
    CHECK(second.computedStyle().opacity() < 1.0f);
    CHECK(first.layoutOffsetY() < 0.0f);
    f.frame(500ms + motion.medium);
    f.frame(500ms + motion.medium);
    CHECK(f.window->toasts().children().size() == 2);
    CHECK(first.layoutOffsetY() == 0.0f);
    CHECK(f.settled());

    // The bottom one slides back under the edge, and the one above follows it down (dismissed at 1000 ms: what it
    // starts starts then)
    f.frame(1000ms);
    const int thirdY = Components::windowY(third);
    third.dismiss();
    CHECK_FALSE(hasClass(third, "in-place"));
    f.frame(1000ms);
    CHECK(Components::windowY(first) == thirdY);
    CHECK(first.layoutOffsetY() < 0.0f);
    f.frame(1000ms + motion.medium / 2);
    CHECK(third.computedStyle().translateY().value > 0.0f);

    // A new one pushes the others up: they slide
    f.frame(2000ms);
    f.frame(2000ms);
    REQUIRE(f.window->toasts().children().size() == 1);
    const int lastY = Components::windowY(first);
    f.window->toast({.title = "Fourth", .timeout = 0s});
    f.frame(2000ms);
    CHECK(Components::windowY(first) < lastY);
    CHECK(first.layoutOffsetY() == Approx(static_cast<float>(lastY - Components::windowY(first))));
    f.frame(3000ms);
    CHECK(f.settled());
}

// endregion
