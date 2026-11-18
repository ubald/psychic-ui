#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Popup.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/MenuButton.hpp> // IWYU pragma: keep: a MenuButton passed as a Div, which include-cleaner misses
#include <psychic-ui/components/Select.hpp>
#include <psychic-ui/components/Toast.hpp>
#include <psychic-ui/components/Tooltip.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Properties.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
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

        [[nodiscard]] std::size_t inOverlay() const { return window->overlay().children().size(); }
    };

    bool hasClass(const Div &div, const std::string &name) {
        return div.classNames().contains(name);
    }

    float opacityOf(const Div &div) { return div.computedStyle().opacity(); }
}

// region Entering

TEST_CASE("A menu enters from its anchor: its entering style first, then settled", "[popup-motion][menu]") {
    Motions fixture{};
    const Menu &menu = fixture.window->openMenu({Menu::item("One", []() {}), Menu::item("Two", []() {})}, fixture.first);
    // Its first style has the starting class
    CHECK(hasClass(menu, "entering"));
    CHECK(opacityOf(menu) == 0.0f);

    fixture.frame(1000ms);
    // Placed, on its side, and let go before its first paint: transitioning from the entering style
    CHECK_FALSE(hasClass(menu, "entering"));
    CHECK(hasClass(menu, "side-bottom"));
    CHECK(opacityOf(menu) == 0.0f);
    CHECK(menu.computedStyle().scale() == Approx(0.97f));
    CHECK(menu.targetStyle().opacity() == 1.0f);
    CHECK(menu.transitioning(P::opacity));
    CHECK(fixture.window->needsFrame());

    // Its origin is the anchor's middle, on its edge facing it
    CHECK(menu.computedStyle().transformOriginX().value == Approx(static_cast<float>(fixture.first.getWidth()) / 2.0f).margin(0.5f));
    CHECK(menu.computedStyle().transformOriginY().value == 0.0f);

    fixture.frame(1000ms + motion.medium / 2);
    CHECK(opacityOf(menu) > 0.5f);
    CHECK(opacityOf(menu) < 1.0f);
    CHECK(menu.computedStyle().scale() > 0.97f);

    fixture.frame(1000ms + motion.medium);
    CHECK(opacityOf(menu) == 1.0f);
    CHECK(menu.computedStyle().scale() == 1.0f);
    CHECK_FALSE(menu.subtreeTransitioning());
    CHECK_FALSE(fixture.window->needsFrame());
}

TEST_CASE("A tooltip enters from a few pixels toward its anchor", "[popup-motion][tooltip]") {
    Motions fixture{};
    fixture.first.tooltip("First");
    fixture.frame(0ms);
    fixture.hover(fixture.first);
    fixture.frame(1000ms);
    fixture.frame(1000ms + Window::tooltipDelay);
    const Tooltip *tooltip = fixture.window->tooltip();
    REQUIRE(tooltip != nullptr);
    // Below its anchor (no room above): it starts up, toward it
    CHECK(tooltip->placedSide() == Side::Bottom);
    CHECK(hasClass(*tooltip, "side-bottom"));
    CHECK(opacityOf(*tooltip) == 0.0f);
    CHECK(tooltip->computedStyle().translateY().value < 0.0f);
    fixture.frame(1000ms + Window::tooltipDelay + motion.medium);
    CHECK(opacityOf(*tooltip) == 1.0f);
    CHECK(tooltip->computedStyle().translateY().value == 0.0f);
}

TEST_CASE("A dialog's scrim fades in and its panel scales up", "[popup-motion][dialog]") {
    Motions fixture{};
    const Dialog &dialog = fixture.window->overlay().open<Dialog>("Rename");
    fixture.frame(0ms);
    CHECK(opacityOf(dialog) == 0.0f);
    CHECK(dialog.panel().computedStyle().scale() == Approx(0.97f));
    fixture.frame(motion.medium / 2);
    CHECK(opacityOf(dialog) > 0.0f);
    CHECK(dialog.panel().computedStyle().scale() > 0.97f);
    fixture.frame(motion.medium);
    CHECK(opacityOf(dialog) == 1.0f);
    CHECK(dialog.panel().computedStyle().scale() == 1.0f);
}

// endregion

// region Leaving

TEST_CASE("A closed menu stays until its exit ends, closed for everything else", "[popup-motion][menu]") {
    Motions fixture{};
    fixture.window->requestFocus(&fixture.first, true);
    int closed = 0;
    fixture.window->onMenuClose([&]() { ++closed; });
    Menu &menu = fixture.window->openMenu({Menu::item("One", []() {})}, fixture.second);
    fixture.frame(0ms);
    fixture.frame(motion.medium);
    REQUIRE(menu.hasFocus());
    REQUIRE(fixture.window->overlay().modal());

    const auto keep = menu.shared_from_this();
    menu.close();
    // Closed now: `closed` emitted, the focus back, not open, not modal, no mouse, not the overlay's
    CHECK(closed == 1);
    CHECK(fixture.first.hasFocus());
    CHECK(fixture.window->focusVisible());
    CHECK_FALSE(menu.isOpen());
    CHECK(menu.exiting());
    CHECK(hasClass(menu, "exiting"));
    CHECK_FALSE(menu.modal());
    CHECK_FALSE(fixture.window->overlay().modal());
    CHECK(fixture.window->overlay().focusScope() == nullptr);
    CHECK(fixture.window->overlay().top<Menu>() == nullptr);
    CHECK(fixture.window->overlay().popups().empty());
    CHECK_FALSE(fixture.window->overlay().dismissTop());
    // But still painted, on its way out
    CHECK(fixture.inOverlay() == 1);

    // Its exit starts at the time of the last frame (`Window::animationClock`)
    const auto t = std::chrono::nanoseconds{motion.medium};
    fixture.frame(t);
    CHECK(fixture.inOverlay() == 1);
    CHECK(opacityOf(menu) == 1.0f);
    fixture.frame(t + motion.fast / 2);
    CHECK(opacityOf(menu) < 1.0f);
    CHECK(opacityOf(menu) > 0.0f);

    // The mouse goes through it, to what is under it
    const auto [x, y] = Motions::center(menu);
    CHECK(fixture.window->overlay().popupAt(x, y) == nullptr);
    fixture.click(fixture.first);
    CHECK(fixture.firstClicks == 1);

    // Gone once it is over, and nothing more to draw
    fixture.frame(t + motion.fast);
    CHECK(fixture.inOverlay() == 0);
    CHECK_FALSE(menu.exiting());
    CHECK_FALSE(hasClass(menu, "exiting"));
    CHECK(closed == 1);
    fixture.frame(t + 2 * motion.fast);
    CHECK_FALSE(fixture.window->needsFrame());
}

TEST_CASE("A popup closed before its first frame, or without exit transitions, leaves at once", "[popup-motion]") {
    Motions fixture{};
    SECTION("never painted") {
        Menu &menu = fixture.window->openMenu({Menu::item("One", []() {})}, fixture.first);
        menu.close();
        CHECK(fixture.inOverlay() == 0);
    }
    SECTION("no rule for its exit") {
        auto &popup = fixture.window->overlay().open<Popup>();
        popup.size(20, 20);
        fixture.frame(0ms);
        fixture.frame(500ms);
        popup.close();
        CHECK(fixture.inOverlay() == 0);
    }
}

TEST_CASE("Opening a menu while another leaves", "[popup-motion][menu]") {
    Motions fixture{};
    Menu &one = fixture.window->openMenu({Menu::item("One", []() {})}, fixture.first);
    fixture.frame(0ms);
    fixture.frame(motion.medium);
    one.close();
    Menu &two = fixture.window->openMenu({Menu::item("Two", []() {})}, fixture.second);
    fixture.frame(motion.medium + motion.fast / 2);
    CHECK(fixture.inOverlay() == 2);
    CHECK(fixture.window->overlay().top<Menu>() == &two);
    // Closing "the open menu" leaves the leaving one alone
    fixture.window->closeMenu();
    CHECK(two.exiting());
    fixture.frame(motion.medium + motion.fast / 2 + motion.fast);
    CHECK(fixture.inOverlay() == 0);
}

TEST_CASE("Hovering back onto a submenu's row while it leaves brings it back", "[popup-motion][menu]") {
    Motions fixture{};
    const Menu &menu = fixture.window->openMenu(
        {Menu::item("Recent", {Menu::item("A", []() {}), Menu::item("B", []() {})}), Menu::item("Other", []() {})},
        fixture.first
    );
    fixture.frame(0ms);
    fixture.frame(motion.medium);
    fixture.hover(*menu.rows()[0]);
    fixture.frame(2 * motion.medium);
    fixture.frame(3 * motion.medium);
    const Menu *sub = menu.submenu();
    REQUIRE(sub != nullptr);
    CHECK(opacityOf(*sub) == 1.0f);

    // Onto the other row: it leaves
    fixture.hover(*menu.rows()[1]);
    CHECK(menu.submenu() == nullptr);
    CHECK(sub->exiting());
    const auto t = std::chrono::nanoseconds{3 * motion.medium};
    fixture.frame(t + motion.fast / 2);
    const float faded = opacityOf(*sub);
    CHECK(faded < 1.0f);

    // Back: the same one, from where it is
    fixture.hover(*menu.rows()[0]);
    CHECK(menu.submenu() == sub);
    CHECK_FALSE(sub->exiting());
    CHECK(sub->isOpen());
    CHECK(fixture.window->overlay().children().back().get() == sub);
    fixture.frame(t + motion.fast / 2);
    CHECK(opacityOf(*sub) == Approx(faded));
    fixture.frame(t + motion.fast / 2 + motion.medium);
    CHECK(opacityOf(*sub) == 1.0f);
    CHECK(fixture.inOverlay() == 2);
}

TEST_CASE("The tooltip of the same div comes back while it fades out", "[popup-motion][tooltip]") {
    Motions fixture{};
    fixture.first.tooltip("First");
    fixture.frame(0ms);
    fixture.hover(fixture.first);
    fixture.frame(1000ms);
    fixture.frame(1000ms + Window::tooltipDelay);
    fixture.frame(2000ms);
    Tooltip *tooltip = fixture.window->tooltip();
    REQUIRE(tooltip != nullptr);
    const auto keep = tooltip->shared_from_this();

    // Off it, warm: it fades out; back on it at once: the same one comes back
    fixture.moveTo(700, 500);
    CHECK(fixture.window->tooltip() == nullptr);
    CHECK(tooltip->exiting());
    fixture.frame(2000ms + motion.fast / 2);
    fixture.hover(fixture.first);
    fixture.frame(2000ms + motion.fast / 2);
    CHECK(fixture.window->tooltip() == tooltip);
    CHECK_FALSE(tooltip->exiting());
    CHECK(fixture.inOverlay() == 1);
}

TEST_CASE("A select's list leaving doesn't count as open", "[popup-motion][select]") {
    Motions fixture{};
    auto &select = fixture.window->appContainer().add<Select>(std::vector<std::string>{"A", "B"});
    fixture.frame(0ms);
    fixture.click(select);
    fixture.frame(100ms);
    fixture.frame(100ms + motion.medium);
    REQUIRE(select.isOpen());
    REQUIRE(hasClass(select, "open"));
    fixture.window->closeMenu();
    CHECK_FALSE(select.isOpen());
    CHECK_FALSE(hasClass(select, "open"));
    // Opens a new list while the old one fades
    fixture.window->requestFocus(&select);
    fixture.window->handleKeyPress(Key::DOWN, Mod{});
    CHECK(select.isOpen());
    const auto t = 100ms + motion.medium;
    fixture.frame(t + motion.fast / 2);
    CHECK(fixture.inOverlay() == 2);
    fixture.frame(t + motion.medium);
    CHECK(fixture.inOverlay() == 1);
}

TEST_CASE("A dialog leaves: no focus trap nor backdrop meanwhile", "[popup-motion][dialog]") {
    Motions fixture{};
    fixture.window->requestFocus(&fixture.first);
    auto &dialog = fixture.window->overlay().open<Dialog>("Rename");
    fixture.frame(0ms);
    fixture.frame(motion.medium);
    REQUIRE(fixture.window->overlay().focusScope() == &dialog);
    fixture.window->handleKeyPress(Key::ESCAPE, Mod{});
    CHECK(dialog.exiting());
    CHECK(fixture.first.hasFocus());
    CHECK(fixture.window->overlay().focusScope() == nullptr);
    fixture.frame(motion.medium + motion.fast / 2);
    CHECK(opacityOf(dialog) < 1.0f);
    CHECK(dialog.panel().computedStyle().scale() < 1.0f);
    fixture.click(fixture.first);
    CHECK(fixture.firstClicks == 1);
    fixture.frame(motion.medium + motion.fast);
    CHECK(fixture.inOverlay() == 0);
}

// endregion

// region Toasts

TEST_CASE("Toasts slide in from under the stack's edge and out fading; those leaving don't count", "[popup-motion][toast]") {
    Motions fixture{};
    Toast &toast = fixture.window->toast({.title = "Saved", .timeout = 0s});
    fixture.frame(0ms);
    CHECK(toast.computedStyle().translateY().value == Approx(150.0f));
    CHECK(opacityOf(toast) == 0.0f);
    fixture.frame(motion.medium / 2);
    CHECK(toast.computedStyle().translateY().value < 150.0f);
    CHECK(toast.computedStyle().translateY().value > 0.0f);
    fixture.frame(motion.medium);
    CHECK(toast.computedStyle().translateY().resolve(100) == 0.0f);
    CHECK(opacityOf(toast) == 1.0f);

    int dismissed = 0;
    toast.onDismiss([&]() { ++dismissed; });
    toast.dismiss();
    CHECK(dismissed == 1); // As it starts leaving
    CHECK(toast.exiting());
    CHECK(fixture.window->toasts().toasts().empty());
    CHECK(fixture.window->toasts().children().size() == 1);
    toast.dismiss();
    CHECK(dismissed == 1);

    // The stack's limit doesn't count it
    for (int i = 0; i < 3; ++i) {
        fixture.window->toast({.title = "Toast " + std::to_string(i), .timeout = 0s});
    }
    CHECK(fixture.window->toasts().toasts().size() == ToastStack::limit);
    CHECK(fixture.window->toasts().children().size() == 4);
    fixture.frame(motion.medium + motion.medium / 2);
    CHECK(opacityOf(toast) < 1.0f);
    CHECK(toast.computedStyle().translateY().value > 0.0f);
    fixture.frame(motion.medium + motion.medium);
    CHECK(fixture.window->toasts().children().size() == 3);

    // One more: the oldest only fades, where it is
    fixture.frame(1000ms);
    const Toast *oldest = fixture.window->toasts().toasts().front();
    fixture.window->toast({.title = "Toast 3", .timeout = 0s});
    CHECK(oldest->exiting());
    CHECK(hasClass(*oldest, "limited"));
    fixture.frame(1000ms + motion.medium / 2);
    CHECK(oldest->computedStyle().translateY().value == 0.0f);
    CHECK(opacityOf(*oldest) < 1.0f);
}

// endregion

// region Reduced motion

TEST_CASE("With reduced motion, popups and toasts open and close at once", "[popup-motion][reduced-motion]") {
    Motions fixture{};
    fixture.window->reducedMotion(true);
    Menu &menu = fixture.window->openMenu({Menu::item("One", []() {})}, fixture.first);
    CHECK_FALSE(hasClass(menu, "entering"));
    fixture.frame(0ms);
    CHECK(opacityOf(menu) == 1.0f);
    CHECK_FALSE(menu.transitioning());
    menu.close();
    CHECK(fixture.inOverlay() == 0);

    Toast &toast = fixture.window->toast({.title = "Saved", .timeout = 0s});
    fixture.frame(100ms);
    CHECK(opacityOf(toast) == 1.0f);
    toast.dismiss();
    CHECK(fixture.window->toasts().children().empty());

    // Turned on while one leaves: it goes now
    fixture.window->reducedMotion(false);
    Menu &other = fixture.window->openMenu({Menu::item("One", []() {})}, fixture.first);
    fixture.frame(200ms);
    fixture.frame(200ms + motion.medium);
    other.close();
    fixture.frame(200ms + motion.medium + motion.fast / 2);
    REQUIRE(fixture.inOverlay() == 2); // The toasts' stack, and the menu leaving
    fixture.window->reducedMotion(true);
    fixture.frame(200ms + motion.medium + motion.fast / 2);
    CHECK(fixture.inOverlay() == 1);
}

// endregion
