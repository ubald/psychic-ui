#include <chrono>
#include <functional>
#include <memory>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/animation/Animation.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace std::chrono_literals;
using Catch::Approx;

namespace {
    using P = Property;

    /**
     * A window with its own style manager and no skin set, laid out at 400 × 300
     */
    struct MotionWindow : Window {
        explicit MotionWindow(std::shared_ptr<StyleManager> manager = std::make_shared<StyleManager>()) :
            Window("Motion") {
            setStyleManager(std::move(manager));
            setWindowSize(400, 300);
        }
    };

    /**
     * A div that says whether its Yoga node is dirty
     */
    struct Probe : Div {
        using Div::isValid;
    };

    struct Scene {
        std::shared_ptr<StyleManager> manager = std::make_shared<StyleManager>();
        std::shared_ptr<MotionWindow> window  = std::make_shared<MotionWindow>(manager);
        Probe                         *box{nullptr};

        Scene() {
            box = &window->appContainer().add<Probe>();
            box->addClassName("box");
        }

        StyleManager &styles() { return *window->styleManager(); }

        /**
         * A frame at `time`: what drawAll does before painting
         */
        void frame(std::chrono::nanoseconds time) {
            window->updateAnimations(time);
            window->updateStylesAndLayout();
        }
    };

    std::uint32_t argbOf(const Div &div) { return div.computedStyle().backgroundColor().argb(); }
}

// region Timing

TEST_CASE("Easing curves are CSS's cubic Béziers", "[animation][ease]") {
    CHECK(Ease::Linear.at(0.3f) == Approx(0.3f));
    CHECK(Ease::Standard.at(0.0f) == 0.0f);
    CHECK(Ease::Standard.at(1.0f) == 1.0f);
    CHECK(Ease::Standard.at(0.5f) == Approx(0.8024f).margin(1e-3));
    CHECK(Ease::In.at(0.5f) == Approx(0.3154f).margin(1e-3));
    CHECK(Ease::Out.at(0.5f) == Approx(0.6846f).margin(1e-3));
    CHECK(Ease::InOut.at(0.5f) == Approx(0.5f).margin(1e-3));
    CHECK(Ease::InOut.at(0.25f) == Approx(0.1292f).margin(1e-3));
    // Pivotal's --ease-out-quick
    CHECK(Ease{0.16f, 1.0f, 0.3f, 1.0f}.at(0.25f) == Approx(0.8256f).margin(1e-3));
    // Outside 0-1, the ends
    CHECK(Ease::Out.at(-1.0f) == 0.0f);
    CHECK(Ease::Out.at(2.0f) == 1.0f);
}

TEST_CASE("A timing is a duration with a curve, a spring, or instant", "[animation][timing]") {
    CHECK(Timing{}.instant());
    CHECK(Timing{0ms}.instant());
    CHECK_FALSE(Timing{100ms}.instant());
    CHECK(Timing{100ms}.ease() == Ease::Standard);
    CHECK(Timing{100ms, Ease::Out}.ease() == Ease::Out);
    CHECK(Timing{Spring::Snappy}.isSpring());
    CHECK_FALSE(Timing{Spring::Snappy}.instant());
    CHECK(Spring::Gentle.dampingRatio() == Approx(1.0f).margin(0.01));
    CHECK(Spring::Snappy.dampingRatio() < 1.0f);
}

// endregion

// region Tween

TEST_CASE("A tween follows its curve at explicit times, and retargets from where it is", "[animation][tween]") {
    Tween tween{{0.0f}, 1};
    tween.retarget({100.0f}, Timing{100ms, Ease::Linear}, 1000ms);
    CHECK(tween.running());

    CHECK(tween.advance(1000ms));
    CHECK(tween.value()[0] == Approx(0.0f));
    CHECK(tween.advance(1050ms));
    CHECK(tween.value()[0] == Approx(50.0f));

    // Mid-way, back to 0: from 50, over the whole duration again
    tween.retarget({0.0f}, Timing{100ms, Ease::Linear}, 1050ms);
    CHECK(tween.advance(1100ms));
    CHECK(tween.value()[0] == Approx(25.0f));
    CHECK_FALSE(tween.advance(1150ms));
    CHECK(tween.value()[0] == 0.0f);
    CHECK_FALSE(tween.running());
}

TEST_CASE("A tween started between frames starts with the next one", "[animation][tween]") {
    Tween tween{{0.0f}, 1};
    tween.retarget({10.0f}, Timing{100ms, Ease::Linear}, std::nullopt);
    CHECK(tween.advance(5000ms)); // Starts here
    CHECK(tween.value()[0] == 0.0f);
    tween.advance(5050ms);
    CHECK(tween.value()[0] == Approx(5.0f));
}

TEST_CASE("An instant timing jumps", "[animation][tween]") {
    Tween tween{{0.0f}, 1};
    tween.retarget({10.0f}, Timing{}, 0ms);
    CHECK_FALSE(tween.running());
    CHECK(tween.value()[0] == 10.0f);
}

TEST_CASE("A spring settles on its target, overshooting when underdamped", "[animation][spring]") {
    Tween tween{{0.0f}, 1};
    tween.retarget({100.0f}, Timing{Spring::Snappy}, 0ms);
    float most = 0.0f;
    auto  time = 0ms;
    while (tween.advance(time) && time < 5s) {
        most = std::max(most, tween.value()[0]);
        time += 16ms;
    }
    CHECK_FALSE(tween.running());
    CHECK(time < 1s);
    CHECK(tween.value()[0] == 100.0f);
    CHECK(most > 100.0f); // Snappy overshoots a little

    // Critically damped, it doesn't
    Tween gentle{{0.0f}, 1};
    gentle.retarget({100.0f}, Timing{Spring::Gentle}, 0ms);
    most = 0.0f;
    for (auto t = 0ms; t < 3s && gentle.advance(t); t += 16ms) {
        most = std::max(most, gentle.value()[0]);
    }
    CHECK_FALSE(gentle.running());
    CHECK(most <= 100.0f + 0.1f);
}

TEST_CASE("A spring is the same whatever the frame rate", "[animation][spring]") {
    Tween a{{0.0f}, 1};
    Tween b{{0.0f}, 1};
    a.retarget({100.0f}, Timing{Spring::Bouncy}, 0ms);
    b.retarget({100.0f}, Timing{Spring::Bouncy}, 0ms);
    for (auto t = 0ms; t <= 120ms; t += 8ms) {
        a.advance(t);
    }
    b.advance(120ms);
    CHECK(a.value()[0] == Approx(b.value()[0]).margin(1e-3));
}

TEST_CASE("A spring keeps its velocity when retargeted; a curve starts from rest", "[animation][spring]") {
    Tween spring{{0.0f}, 1};
    spring.retarget({100.0f}, Timing{Spring::Snappy}, 0ms);
    spring.advance(50ms);
    const float at = spring.value()[0];
    CHECK(spring.velocity()[0] > 0.0f);

    // Sent back: it still moves forward for a moment, then turns
    spring.retarget({0.0f}, Timing{Spring::Snappy}, 50ms);
    spring.advance(55ms);
    CHECK(spring.value()[0] > at);
    for (auto t = 60ms; t < 3s && spring.advance(t); t += 16ms) {}
    CHECK(spring.value()[0] == 0.0f);

    Tween curve{{0.0f}, 1};
    curve.retarget({100.0f}, Timing{100ms, Ease::Linear}, 0ms);
    curve.advance(50ms);
    curve.retarget({0.0f}, Timing{100ms, Ease::Linear}, 50ms);
    curve.advance(55ms);
    CHECK(curve.value()[0] < 50.0f);
}

TEST_CASE("Colors interpolate premultiplied", "[animation][color]") {
    const Channels red = Interpolation<Color>::channels(rgb(0xFF0000));
    const Channels none{};
    // Half-way from transparent: half the alpha, the same red
    Channels half{};
    for (std::size_t i = 0; i < 4; ++i) {
        half[i] = (red[i] + none[i]) / 2.0f;
    }
    CHECK(Interpolation<Color>::value(half, {}).argb() == 0x80FF0000u);
    CHECK(Interpolation<Color>::value(Interpolation<Color>::channels(argb(0x40123456)), {}).argb() == 0x40123456u);
    // A length changes unit at once
    CHECK(Interpolation<Length>::compatible(Length{10}, Length{20}));
    CHECK_FALSE(Interpolation<Length>::compatible(Length{10}, 50_pct));
    CHECK_FALSE(Interpolation<Length>::compatible(Length{10}, Auto));
}

// endregion

// region Animated and animate

TEST_CASE("Animated moves with its owner's window, and jumps without one", "[animation][animated]") {
    Scene scene;
    Animated<float> value{scene.box, 0.0f, Timing{100ms, Ease::Linear}};

    scene.window->updateAnimations(1000ms);
    value.to(10.0f); // In a frame: starts at its time
    CHECK(value.animating());
    CHECK(value.target() == 10.0f);
    CHECK(scene.window->animating());
    scene.window->updateAnimations(1050ms);
    CHECK(value.value() == Approx(5.0f));
    scene.window->updateAnimations(1100ms);
    CHECK(value.value() == 10.0f);
    CHECK_FALSE(value.animating());
    CHECK_FALSE(scene.window->animating());

    Div detached;
    Animated<Color> color{&detached, rgb(0x000000), Timing{100ms}};
    color.to(rgb(0xFFFFFF));
    CHECK(color.value() == rgb(0xFFFFFF));
    CHECK_FALSE(color.animating());
}

TEST_CASE("An Animated that goes stops being ticked", "[animation][animated]") {
    Scene scene;
    {
        Animated<float> value{scene.box, 0.0f, Timing{100ms}};
        scene.window->updateAnimations(0ms);
        value.to(1.0f);
        CHECK(scene.window->animating());
    }
    scene.window->updateAnimations(10ms);
    CHECK_FALSE(scene.window->animating());
}

TEST_CASE("Div::animate calls back each frame and ends with its div", "[animation][animate]") {
    Scene scene;
    std::vector<float> seen{};
    scene.window->updateAnimations(0ms);
    scene.box->animate(0.0f, 1.0f, Timing{100ms, Ease::Linear}, [&seen](float t) { seen.push_back(t); });
    REQUIRE(seen.size() == 1);
    CHECK(seen.back() == 0.0f);
    scene.window->updateAnimations(50ms);
    CHECK(seen.back() == Approx(0.5f));
    scene.window->updateAnimations(100ms);
    CHECK(seen.back() == 1.0f);
    CHECK_FALSE(scene.window->animating());

    // Removed and destroyed mid-way: no more calls
    seen.clear();
    scene.box->animate(0.0f, 1.0f, Timing{100ms}, [&seen](float t) { seen.push_back(t); });
    scene.window->appContainer().removeAll();
    scene.box = nullptr;
    scene.window->updateAnimations(150ms);
    CHECK(seen.size() == 1);
    CHECK_FALSE(scene.window->animating());
}

// endregion

// region Style transitions

TEST_CASE("A paint-only property transitions without touching the layout", "[animation][transition]") {
    Scene scene;
    scene.styles().style(".box").backgroundColor(rgb(0x000000)).width(100).height(100)
         .transition(P::backgroundColor, Timing{100ms, Ease::Linear});
    scene.styles().style(".box.on").backgroundColor(rgb(0xFF0000));

    // The first style doesn't transition
    scene.frame(0ms);
    CHECK(argbOf(*scene.box) == 0xFF000000u);
    CHECK_FALSE(scene.box->transitioning());

    scene.box->addClassName("on");
    scene.frame(1000ms);
    CHECK(scene.box->transitioning(P::backgroundColor));
    CHECK(argbOf(*scene.box) == 0xFF000000u); // Starts where it was
    CHECK(scene.box->targetStyle().backgroundColor() == rgb(0xFF0000));
    CHECK(scene.box->isValid()); // Yoga never saw it
    CHECK(scene.window->animating());

    scene.frame(1050ms);
    CHECK(argbOf(*scene.box) == 0xFF800000u);
    CHECK(scene.box->isValid());

    scene.frame(1100ms);
    CHECK(argbOf(*scene.box) == 0xFFFF0000u);
    CHECK_FALSE(scene.box->transitioning());
    CHECK_FALSE(scene.window->animating());
}

TEST_CASE("A layout property transitions through Yoga, and retargets mid-way", "[animation][transition][layout]") {
    Scene scene;
    scene.styles().style(".box").width(100).height(20).transition(P::width, Timing{100ms, Ease::Linear});
    scene.styles().style(".box.wide").width(200);
    scene.frame(0ms);
    CHECK(scene.box->getWidth() == 100);

    scene.box->addClassName("wide");
    scene.frame(1000ms);
    CHECK(scene.box->getWidth() == 100);
    scene.frame(1050ms);
    CHECK(scene.box->getWidth() == 150);

    // Back, from 150, over the whole duration
    scene.box->removeClassName("wide");
    scene.frame(1050ms);
    scene.frame(1100ms);
    CHECK(scene.box->getWidth() == 125);
    scene.frame(1150ms);
    CHECK(scene.box->getWidth() == 100);
    CHECK_FALSE(scene.box->transitioning());
}

TEST_CASE("Only the winning rule's transition applies; others change at once", "[animation][transition]") {
    Scene scene;
    scene.styles().style(".box").opacity(1.0f).borderRadius(0)
         .transition(P::opacity, Timing{100ms, Ease::Linear});
    scene.styles().style(".box.faded").opacity(0.0f).borderRadius(10);
    scene.styles().style(".box.faded.still").transition(Transition{});
    scene.frame(0ms);
    scene.box->addClassName("faded");
    scene.frame(0ms);
    scene.frame(50ms);
    CHECK(scene.box->computedStyle().opacity() == Approx(0.5f));
    CHECK(scene.box->computedStyle().borderRadius() == 10.0f); // Not in the transition

    // A rule that wins with no transition stops it where the new style says
    scene.box->addClassName("still");
    scene.frame(60ms);
    CHECK(scene.box->computedStyle().opacity() == 0.0f);
    CHECK_FALSE(scene.box->transitioning());
}

TEST_CASE("A color appearing fades in from transparent, and goes at the end", "[animation][transition]") {
    Scene scene;
    scene.styles().style(".box").width(10).height(10).transition(P::outlineColor, Timing{100ms, Ease::Linear});
    scene.styles().style(".box.ring").outlineColor(rgb(0x0000FF)).outlineWidth(2);
    scene.frame(0ms);
    CHECK_FALSE(scene.box->computedStyle().has(P::outlineColor));

    scene.box->addClassName("ring");
    scene.frame(0ms);
    CHECK(scene.box->computedStyle().outlineWidth() == 2.0f); // Snaps, not in the transition
    scene.frame(50ms);
    CHECK(scene.box->computedStyle().outlineColor().argb() == 0x800000FFu);

    scene.box->removeClassName("ring");
    scene.frame(50ms);
    CHECK(scene.box->computedStyle().has(P::outlineColor)); // Fading out
    scene.frame(200ms);
    CHECK_FALSE(scene.box->computedStyle().has(P::outlineColor));
    CHECK_FALSE(scene.box->transitioning());
}

TEST_CASE("Children inherit the blend", "[animation][transition]") {
    Scene scene;
    Div &child = scene.box->add<Div>();
    scene.styles().style(".box").color(rgb(0x000000)).transition(P::color, Timing{100ms, Ease::Linear});
    scene.styles().style(".box.on").color(rgb(0xFFFFFF));
    scene.frame(0ms);
    scene.box->addClassName("on");
    scene.frame(0ms);
    scene.frame(50ms);
    CHECK(child.computedStyle().color().argb() == 0xFF808080u);
    scene.frame(100ms);
    CHECK(child.computedStyle().color().argb() == 0xFFFFFFFFu);
}

TEST_CASE("New rules, reduced motion and a div's first style in a window don't transition", "[animation][transition]") {
    Scene scene;
    scene.styles().style(".box").backgroundColor(rgb(0x000000)).transition(P::backgroundColor, Timing{100ms});
    scene.frame(0ms);

    // A theme switch reloads the rules: everything changes at once
    scene.styles().theme(themes::pivotalLight);
    scene.styles().style(".box").backgroundColor(rgb(0xFFFFFF)).transition(P::backgroundColor, Timing{100ms});
    scene.frame(10ms);
    CHECK(argbOf(*scene.box) == 0xFFFFFFFFu);
    CHECK_FALSE(scene.box->transitioning());

    // Moved to another window: its first style there doesn't blend either
    scene.styles().style(".box.on").backgroundColor(rgb(0xFF0000));
    auto other = std::make_shared<MotionWindow>(scene.manager);
    auto moved = scene.window->appContainer().children().front();
    scene.window->appContainer().remove(moved);
    moved->addClassName("on");
    other->appContainer().add(moved);
    other->updateAnimations(20ms);
    other->updateStylesAndLayout();
    CHECK(argbOf(*moved) == 0xFFFF0000u);

    // Reduced motion: at once, and what moved ends
    moved->removeClassName("on");
    other->updateAnimations(30ms);
    other->updateStylesAndLayout();
    CHECK(moved->transitioning());
    other->reducedMotion(true);
    CHECK_FALSE(moved->transitioning());
    CHECK(argbOf(*moved) == 0xFFFFFFFFu);
    moved->addClassName("on");
    other->updateAnimations(40ms);
    other->updateStylesAndLayout();
    CHECK(argbOf(*moved) == 0xFFFF0000u);
    CHECK_FALSE(other->animating());
}

TEST_CASE("Transitions declared inline, a few timings", "[animation][transition]") {
    Transition transition{};
    transition.add(mask({P::color, P::opacity}), Timing{100ms});
    transition.add(mask({P::opacity}), Timing{Spring::Snappy});
    CHECK(*transition.timing(P::color) == Timing{100ms});
    CHECK(transition.timing(P::opacity)->isSpring());
    CHECK(transition.timing(P::width) == nullptr);
    CHECK(transition.properties() == mask({P::color, P::opacity}));

    Scene scene;
    scene.box->size(10, 10).opacity(1.0f).transition(P::opacity, Timing{100ms, Ease::Linear});
    scene.frame(0ms);
    scene.box->opacity(0.0f);
    scene.frame(0ms);
    scene.frame(25ms);
    CHECK(scene.box->computedStyle().opacity() == Approx(0.75f));
}

// endregion

