#include <chrono>
#include <memory>
#include <string>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/MenuButton.hpp>
#include <psychic-ui/components/Select.hpp>
#include <psychic-ui/components/Toast.hpp>
#include <psychic-ui/components/Tooltip.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;
using namespace std::chrono_literals;
using Catch::Approx;

namespace {
    using P = Property;

    constexpr Motion motion = themes::pivotalDark.metrics.motion;

    /**
     * A row of buttons at the top of a window that moves (the shared TestWindow reduces motion), driven by frames at
     * explicit times
     */
    struct Motions {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 800, 600};
        Div                         &bar{window->appContainer().add<Div>()};
        Button                      &first{bar.add<Button>("First")};
        Button                      &second{bar.add<Button>("Second")};
        int                         firstClicks{0};

        Motions() {
            window->reducedMotion(false);
            bar.direction(Direction::Row).padding(20).gap(10);
            first.onClick([this]() { ++firstClicks; });
            frame(0ms);
        }

        /**
         * What drawAll does at `time`, then a raster render
         */
        void frame(std::chrono::nanoseconds time) {
            window->updatePopups(time);
            window->updateAnimations(time);
            systemWindow.layout();
        }

        void press(int x, int y, MouseButton button = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = button, .down = true});
        }

        void release(int x, int y, MouseButton button = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = button, .down = false});
        }

        void clickAt(int x, int y) {
            press(x, y);
            release(x, y);
        }

        void click(const Div &div) {
            const auto [x, y] = center(div);
            clickAt(x, y);
        }

        void moveTo(int x, int y) {
            window->handleMouseMove({.x = static_cast<float>(x), .y = static_cast<float>(y)}, false);
        }

        void hover(const Div &div) {
            const auto [x, y] = center(div);
            moveTo(x, y);
        }

        static std::pair<int, int> center(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y);
            return {x - div.scrollX() + (div.getWidth() / 2), y - div.scrollY() + (div.getHeight() / 2)};
        }

        std::size_t inOverlay() const { return window->overlay().children().size(); }
    };

    bool hasClass(const Div &div, const std::string &name) {
        return div.classNames().contains(name);
    }

    float opacityOf(const Div &div) { return div.computedStyle().opacity(); }
}

// region Entering

TEST_CASE("A menu enters from its anchor: its entering style first, then settled", "[popup-motion][menu]") {
    Motions f{};
    const Menu &menu = f.window->openMenu({Menu::item("One", []() {}), Menu::item("Two", []() {})}, f.first);
    // Its first style has the starting class
    CHECK(hasClass(menu, "entering"));
    CHECK(opacityOf(menu) == 0.0f);

    f.frame(1000ms);
    // Placed, on its side, and let go before its first paint: transitioning from the entering style
    CHECK_FALSE(hasClass(menu, "entering"));
    CHECK(hasClass(menu, "side-bottom"));
    CHECK(opacityOf(menu) == 0.0f);
    CHECK(menu.computedStyle().scale() == Approx(0.97f));
    CHECK(menu.targetStyle().opacity() == 1.0f);
    CHECK(menu.transitioning(P::opacity));
    CHECK(f.window->needsFrame());

    // Its origin is the anchor's middle, on its edge facing it
    CHECK(menu.computedStyle().transformOriginX().value == Approx(static_cast<float>(f.first.getWidth()) / 2.0f).margin(0.5f));
    CHECK(menu.computedStyle().transformOriginY().value == 0.0f);

    f.frame(1000ms + motion.medium / 2);
    CHECK(opacityOf(menu) > 0.5f);
    CHECK(opacityOf(menu) < 1.0f);
    CHECK(menu.computedStyle().scale() > 0.97f);

    f.frame(1000ms + motion.medium);
    CHECK(opacityOf(menu) == 1.0f);
    CHECK(menu.computedStyle().scale() == 1.0f);
    CHECK_FALSE(menu.subtreeTransitioning());
    CHECK_FALSE(f.window->needsFrame());
}

TEST_CASE("A tooltip enters from a few pixels toward its anchor", "[popup-motion][tooltip]") {
    Motions f{};
    f.first.tooltip("First");
    f.frame(0ms);
    f.hover(f.first);
    f.frame(1000ms);
    f.frame(1000ms + Window::tooltipDelay);
    const Tooltip *tooltip = f.window->tooltip();
    REQUIRE(tooltip != nullptr);
    // Below its anchor (no room above): it starts up, toward it
    CHECK(tooltip->placedSide() == Side::Bottom);
    CHECK(hasClass(*tooltip, "side-bottom"));
    CHECK(opacityOf(*tooltip) == 0.0f);
    CHECK(tooltip->computedStyle().translateY().value < 0.0f);
    f.frame(1000ms + Window::tooltipDelay + motion.medium);
    CHECK(opacityOf(*tooltip) == 1.0f);
    CHECK(tooltip->computedStyle().translateY().value == 0.0f);
}

TEST_CASE("A dialog's scrim fades in and its panel scales up", "[popup-motion][dialog]") {
    Motions f{};
    const Dialog &dialog = f.window->overlay().open<Dialog>("Rename");
    f.frame(0ms);
    CHECK(opacityOf(dialog) == 0.0f);
    CHECK(dialog.panel().computedStyle().scale() == Approx(0.97f));
    f.frame(motion.medium / 2);
    CHECK(opacityOf(dialog) > 0.0f);
    CHECK(dialog.panel().computedStyle().scale() > 0.97f);
    f.frame(motion.medium);
    CHECK(opacityOf(dialog) == 1.0f);
    CHECK(dialog.panel().computedStyle().scale() == 1.0f);
}

// endregion

// region Leaving

TEST_CASE("A closed menu stays until its exit ends, closed for everything else", "[popup-motion][menu]") {
    Motions f{};
    f.window->requestFocus(&f.first, true);
    int closed = 0;
    f.window->onMenuClose([&]() { ++closed; });
    Menu &menu = f.window->openMenu({Menu::item("One", []() {})}, f.second);
    f.frame(0ms);
    f.frame(motion.medium);
    REQUIRE(menu.hasFocus());
    REQUIRE(f.window->overlay().modal());

    const auto keep = menu.shared_from_this();
    menu.close();
    // Closed now: `closed` emitted, the focus back, not open, not modal, no mouse, not the overlay's
    CHECK(closed == 1);
    CHECK(f.first.hasFocus());
    CHECK(f.window->focusVisible());
    CHECK_FALSE(menu.isOpen());
    CHECK(menu.exiting());
    CHECK(hasClass(menu, "exiting"));
    CHECK_FALSE(menu.modal());
    CHECK_FALSE(f.window->overlay().modal());
    CHECK(f.window->overlay().focusScope() == nullptr);
    CHECK(f.window->overlay().top<Menu>() == nullptr);
    CHECK(f.window->overlay().popups().empty());
    CHECK_FALSE(f.window->overlay().dismissTop());
    // But still painted, on its way out
    CHECK(f.inOverlay() == 1);

    // Its exit starts at the time of the last frame (`Window::animationClock`)
    const auto t = std::chrono::nanoseconds{motion.medium};
    f.frame(t);
    CHECK(f.inOverlay() == 1);
    CHECK(opacityOf(menu) == 1.0f);
    f.frame(t + motion.fast / 2);
    CHECK(opacityOf(menu) < 1.0f);
    CHECK(opacityOf(menu) > 0.0f);

    // The mouse goes through it, to what is under it
    const auto [x, y] = Motions::center(menu);
    CHECK(f.window->overlay().popupAt(x, y) == nullptr);
    f.click(f.first);
    CHECK(f.firstClicks == 1);

    // Gone once it is over, and nothing more to draw
    f.frame(t + motion.fast);
    CHECK(f.inOverlay() == 0);
    CHECK_FALSE(menu.exiting());
    CHECK_FALSE(hasClass(menu, "exiting"));
    CHECK(closed == 1);
    f.frame(t + 2 * motion.fast);
    CHECK_FALSE(f.window->needsFrame());
}

TEST_CASE("A popup closed before its first frame, or without exit transitions, leaves at once", "[popup-motion]") {
    Motions f{};
    SECTION("never painted") {
        Menu &menu = f.window->openMenu({Menu::item("One", []() {})}, f.first);
        menu.close();
        CHECK(f.inOverlay() == 0);
    }
    SECTION("no rule for its exit") {
        auto &popup = f.window->overlay().open<Popup>();
        popup.size(20, 20);
        f.frame(0ms);
        f.frame(500ms);
        popup.close();
        CHECK(f.inOverlay() == 0);
    }
}

TEST_CASE("Opening a menu while another leaves", "[popup-motion][menu]") {
    Motions f{};
    Menu &one = f.window->openMenu({Menu::item("One", []() {})}, f.first);
    f.frame(0ms);
    f.frame(motion.medium);
    one.close();
    Menu &two = f.window->openMenu({Menu::item("Two", []() {})}, f.second);
    f.frame(motion.medium + motion.fast / 2);
    CHECK(f.inOverlay() == 2);
    CHECK(f.window->overlay().top<Menu>() == &two);
    // Closing "the open menu" leaves the leaving one alone
    f.window->closeMenu();
    CHECK(two.exiting());
    f.frame(motion.medium + motion.fast / 2 + motion.fast);
    CHECK(f.inOverlay() == 0);
}

TEST_CASE("Hovering back onto a submenu's row while it leaves brings it back", "[popup-motion][menu]") {
    Motions f{};
    const Menu &menu = f.window->openMenu(
        {Menu::item("Recent", {Menu::item("A", []() {}), Menu::item("B", []() {})}), Menu::item("Other", []() {})},
        f.first
    );
    f.frame(0ms);
    f.frame(motion.medium);
    f.hover(*menu.rows()[0]);
    f.frame(2 * motion.medium);
    f.frame(3 * motion.medium);
    const Menu *sub = menu.submenu();
    REQUIRE(sub != nullptr);
    CHECK(opacityOf(*sub) == 1.0f);

    // Onto the other row: it leaves
    f.hover(*menu.rows()[1]);
    CHECK(menu.submenu() == nullptr);
    CHECK(sub->exiting());
    const auto t = std::chrono::nanoseconds{3 * motion.medium};
    f.frame(t + motion.fast / 2);
    const float faded = opacityOf(*sub);
    CHECK(faded < 1.0f);

    // Back: the same one, from where it is
    f.hover(*menu.rows()[0]);
    CHECK(menu.submenu() == sub);
    CHECK_FALSE(sub->exiting());
    CHECK(sub->isOpen());
    CHECK(f.window->overlay().children().back().get() == sub);
    f.frame(t + motion.fast / 2);
    CHECK(opacityOf(*sub) == Approx(faded));
    f.frame(t + motion.fast / 2 + motion.medium);
    CHECK(opacityOf(*sub) == 1.0f);
    CHECK(f.inOverlay() == 2);
}

TEST_CASE("The tooltip of the same div comes back while it fades out", "[popup-motion][tooltip]") {
    Motions f{};
    f.first.tooltip("First");
    f.frame(0ms);
    f.hover(f.first);
    f.frame(1000ms);
    f.frame(1000ms + Window::tooltipDelay);
    f.frame(2000ms);
    Tooltip *tooltip = f.window->tooltip();
    REQUIRE(tooltip != nullptr);
    const auto keep = tooltip->shared_from_this();

    // Off it, warm: it fades out; back on it at once: the same one comes back
    f.moveTo(700, 500);
    CHECK(f.window->tooltip() == nullptr);
    CHECK(tooltip->exiting());
    f.frame(2000ms + motion.fast / 2);
    f.hover(f.first);
    f.frame(2000ms + motion.fast / 2);
    CHECK(f.window->tooltip() == tooltip);
    CHECK_FALSE(tooltip->exiting());
    CHECK(f.inOverlay() == 1);
}

TEST_CASE("A select's list leaving doesn't count as open", "[popup-motion][select]") {
    Motions f{};
    auto &select = f.window->appContainer().add<Select>(std::vector<std::string>{"A", "B"});
    f.frame(0ms);
    f.click(select);
    f.frame(100ms);
    f.frame(100ms + motion.medium);
    REQUIRE(select.isOpen());
    REQUIRE(hasClass(select, "open"));
    f.window->closeMenu();
    CHECK_FALSE(select.isOpen());
    CHECK_FALSE(hasClass(select, "open"));
    // Opens a new list while the old one fades
    f.window->requestFocus(&select);
    f.window->handleKeyPress(Key::DOWN, Mod{});
    CHECK(select.isOpen());
    const auto t = 100ms + motion.medium;
    f.frame(t + motion.fast / 2);
    CHECK(f.inOverlay() == 2);
    f.frame(t + motion.medium);
    CHECK(f.inOverlay() == 1);
}

TEST_CASE("A dialog leaves: no focus trap nor backdrop meanwhile", "[popup-motion][dialog]") {
    Motions f{};
    f.window->requestFocus(&f.first);
    auto &dialog = f.window->overlay().open<Dialog>("Rename");
    f.frame(0ms);
    f.frame(motion.medium);
    REQUIRE(f.window->overlay().focusScope() == &dialog);
    f.window->handleKeyPress(Key::ESCAPE, Mod{});
    CHECK(dialog.exiting());
    CHECK(f.first.hasFocus());
    CHECK(f.window->overlay().focusScope() == nullptr);
    f.frame(motion.medium + motion.fast / 2);
    CHECK(opacityOf(dialog) < 1.0f);
    CHECK(dialog.panel().computedStyle().scale() < 1.0f);
    f.click(f.first);
    CHECK(f.firstClicks == 1);
    f.frame(motion.medium + motion.fast);
    CHECK(f.inOverlay() == 0);
}

// endregion

// region Toasts

TEST_CASE("Toasts slide in from under the stack's edge and out fading; those leaving don't count", "[popup-motion][toast]") {
    Motions f{};
    Toast &toast = f.window->toast({.title = "Saved", .timeout = 0s});
    f.frame(0ms);
    CHECK(toast.computedStyle().translateY().value == Approx(150.0f));
    CHECK(opacityOf(toast) == 0.0f);
    f.frame(motion.medium / 2);
    CHECK(toast.computedStyle().translateY().value < 150.0f);
    CHECK(toast.computedStyle().translateY().value > 0.0f);
    f.frame(motion.medium);
    CHECK(toast.computedStyle().translateY().resolve(100) == 0.0f);
    CHECK(opacityOf(toast) == 1.0f);

    int dismissed = 0;
    toast.onDismiss([&]() { ++dismissed; });
    toast.dismiss();
    CHECK(dismissed == 1); // As it starts leaving
    CHECK(toast.exiting());
    CHECK(f.window->toasts().toasts().empty());
    CHECK(f.window->toasts().children().size() == 1);
    toast.dismiss();
    CHECK(dismissed == 1);

    // The stack's limit doesn't count it
    for (int i = 0; i < 3; ++i) {
        f.window->toast({.title = "Toast " + std::to_string(i), .timeout = 0s});
    }
    CHECK(f.window->toasts().toasts().size() == ToastStack::limit);
    CHECK(f.window->toasts().children().size() == 4);
    f.frame(motion.medium + motion.medium / 2);
    CHECK(opacityOf(toast) < 1.0f);
    CHECK(toast.computedStyle().translateY().value > 0.0f);
    f.frame(motion.medium + motion.medium);
    CHECK(f.window->toasts().children().size() == 3);

    // One more: the oldest only fades, where it is
    f.frame(1000ms);
    const Toast *oldest = f.window->toasts().toasts().front();
    f.window->toast({.title = "Toast 3", .timeout = 0s});
    CHECK(oldest->exiting());
    CHECK(hasClass(*oldest, "limited"));
    f.frame(1000ms + motion.medium / 2);
    CHECK(oldest->computedStyle().translateY().value == 0.0f);
    CHECK(opacityOf(*oldest) < 1.0f);
}

// endregion

// region Reduced motion

TEST_CASE("With reduced motion, popups and toasts open and close at once", "[popup-motion][reduced-motion]") {
    Motions f{};
    f.window->reducedMotion(true);
    Menu &menu = f.window->openMenu({Menu::item("One", []() {})}, f.first);
    CHECK_FALSE(hasClass(menu, "entering"));
    f.frame(0ms);
    CHECK(opacityOf(menu) == 1.0f);
    CHECK_FALSE(menu.transitioning());
    menu.close();
    CHECK(f.inOverlay() == 0);

    Toast &toast = f.window->toast({.title = "Saved", .timeout = 0s});
    f.frame(100ms);
    CHECK(opacityOf(toast) == 1.0f);
    toast.dismiss();
    CHECK(f.window->toasts().children().empty());

    // Turned on while one leaves: it goes now
    f.window->reducedMotion(false);
    Menu &other = f.window->openMenu({Menu::item("One", []() {})}, f.first);
    f.frame(200ms);
    f.frame(200ms + motion.medium);
    other.close();
    f.frame(200ms + motion.medium + motion.fast / 2);
    REQUIRE(f.inOverlay() == 2); // The toasts' stack, and the menu leaving
    f.window->reducedMotion(true);
    f.frame(200ms + motion.medium + motion.fast / 2);
    CHECK(f.inOverlay() == 1);
}

// endregion
