#include <chrono>
#include <memory>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Window.hpp>

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
