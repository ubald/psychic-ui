#include <memory>
#include <typeinfo>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Skin.hpp>
#include <psychic-ui/components/Splitter.hpp>
#include <psychic-ui/components/SplitterDivider.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/skins/base/SplitterDividerSkinBase.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    /**
     * A splitter 600 by 400 at the window's top left
     */
    struct Split {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 800, 600};
        Splitter                    &splitter{window->appContainer().add<Splitter>()};
        std::vector<float>          changes{};

        Split() {
            window->appContainer().alignItems(Align::Start);
            splitter.width(600).height(400).shrink(0);
            splitter.onChange([this](float position) { changes.push_back(position); });
        }

        void layout() { systemWindow.layout(); }

        static std::pair<int, int> center(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y, div.getWidth() / 2, div.getHeight() / 2);
            return {x, y};
        }

        void press(int x, int y, unsigned clicks = 1) {
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true, .clickCount = clicks});
        }

        void move(int x, int y) {
            window->handleMouseMove({.x = static_cast<float>(x), .y = static_cast<float>(y)}, false);
            layout();
        }

        void release(int x, int y, unsigned clicks = 1) {
            window->handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false, .clickCount = clicks});
            layout();
        }

        void key(Key key, Mod mod = Mod{}) {
            window->handleKeyPress(key, mod);
            layout();
        }

        void focusDivider() {
            window->requestFocus(&splitter.divider());
            layout();
        }
    };
}

TEST_CASE("Splitter: two panes and a divider over their boundary", "[components][splitter]") {
    Split fixture;
    fixture.splitter.defaultPosition(200);
    fixture.layout();

    CHECK(fixture.splitter.first().getWidth() == 200);
    CHECK(fixture.splitter.second().getWidth() == 400);
    CHECK(fixture.splitter.second().x() == 200);
    CHECK(fixture.splitter.first().getHeight() == 400);

    // Wider than the line it shows, centered on the boundary, on top of the panes
    const SplitterDivider &divider = fixture.splitter.divider();
    CHECK(divider.getWidth() == static_cast<int>(SplitterDivider::defaultReach));
    CHECK(divider.x() == 200 - (divider.getWidth() / 2));
    CHECK(divider.getHeight() == 400);
    CHECK(fixture.splitter.children().back().get() == &divider);
    CHECK(divider.classNames().contains("horizontal"));
    CHECK(divider.computedStyle().cursor() == Cursor::HResize);

    // The default set's skin: a hairline, centered
    const auto skin = divider.skin();
    REQUIRE(skin);
    const internal::SkinBase &base = *skin;
    CHECK(typeid(base) == typeid(SplitterDividerSkinBase));
    const Div &line = *skin->children().front();
    CHECK(line.classNames().contains("line"));
    CHECK(line.getWidth() == 1);
    CHECK(line.getHeight() == 400);
    CHECK(divider.x() + line.x() == 200); // On the second pane's first column
}

TEST_CASE("Splitter: a drag moves the divider within the panes' minimums", "[components][splitter]") {
    Split fixture;
    fixture.splitter.defaultPosition(200)
           .minimum(SplitterPane::First, 100)
           .minimum(SplitterPane::Second, 150);
    fixture.layout();

    auto [x, y] = Split::center(fixture.splitter.divider());
    // A press beside the line, on the second pane, still catches the divider
    x += 3;
    fixture.press(x, y);
    fixture.move(x + 50, y);
    CHECK(fixture.splitter.position() == 250);
    CHECK(fixture.splitter.first().getWidth() == 250);
    CHECK(fixture.changes == std::vector<float>{250});

    // Past the second pane's minimum, and outside the splitter: stops at it
    fixture.move(x + 500, y + 300);
    CHECK(fixture.splitter.position() == 450);
    CHECK(fixture.splitter.second().getWidth() == 150);
    CHECK(fixture.splitter.divider().x() == 450 - (fixture.splitter.divider().getWidth() / 2));

    // Back past the first's
    fixture.move(x - 400, y);
    CHECK(fixture.splitter.position() == 100);
    CHECK(fixture.splitter.first().getWidth() == 100);
    fixture.release(x - 400, y);
    CHECK(fixture.changes == std::vector<float>{250, 450, 100});

    // Released: the mouse no longer moves it
    fixture.move(x, y);
    CHECK(fixture.splitter.position() == 100);
}

TEST_CASE("Splitter: the kept pane keeps its size when the splitter resizes", "[components][splitter]") {
    Split fixture;
    fixture.splitter.kept(SplitterPane::Second)
           .defaultPosition(280)
           .minimum(SplitterPane::First, 200);
    fixture.layout();
    CHECK(fixture.splitter.second().getWidth() == 280);
    CHECK(fixture.splitter.first().getWidth() == 320);

    fixture.splitter.width(780);
    fixture.layout();
    CHECK(fixture.splitter.second().getWidth() == 280);
    CHECK(fixture.splitter.first().getWidth() == 500);
    CHECK(fixture.splitter.divider().x() == 500 - (fixture.splitter.divider().getWidth() / 2));

    // Too narrow for both: the other pane's minimum holds, and the kept size comes back with the room
    fixture.splitter.width(400);
    fixture.layout();
    CHECK(fixture.splitter.first().getWidth() == 200);
    CHECK(fixture.splitter.second().getWidth() == 200);
    CHECK(fixture.splitter.position() == 200);
    fixture.splitter.width(700);
    fixture.layout();
    CHECK(fixture.splitter.second().getWidth() == 280);
    CHECK(fixture.changes.empty()); // Only the user's changes are emitted

    // Dragging toward the end shrinks the kept second pane
    auto [x, y] = Split::center(fixture.splitter.divider());
    fixture.press(x, y);
    fixture.move(x + 30, y);
    fixture.release(x + 30, y);
    CHECK(fixture.splitter.position() == 250);
    CHECK(fixture.splitter.second().getWidth() == 250);
}

TEST_CASE("Splitter: a double-click on the divider goes back to the default position", "[components][splitter]") {
    Split fixture;
    fixture.splitter.defaultPosition(200).position(320);
    fixture.layout();
    CHECK(fixture.splitter.first().getWidth() == 320);

    const auto [x, y] = Split::center(fixture.splitter.divider());
    fixture.press(x, y, 1);
    fixture.release(x, y, 1);
    fixture.press(x, y, 2);
    fixture.release(x, y, 2);
    CHECK(fixture.splitter.position() == 200);
    CHECK(fixture.splitter.first().getWidth() == 200);
    CHECK(fixture.changes == std::vector<float>{200});
}

TEST_CASE("Splitter: the arrows move the focused divider", "[components][splitter]") {
    Split fixture;
    fixture.splitter.defaultPosition(200).minimum(SplitterPane::First, 150);
    fixture.layout();
    fixture.focusDivider();
    REQUIRE(fixture.splitter.divider().hasFocus());

    fixture.key(Key::RIGHT);
    CHECK(fixture.splitter.position() == 200 + Splitter::keyStep);
    fixture.key(Key::LEFT, Mod{.shift = true});
    CHECK(fixture.splitter.position() == 150); // Ten steps back, stopped by the minimum
    fixture.key(Key::UP); // Across a horizontal splitter: nothing
    CHECK(fixture.splitter.position() == 150);
    CHECK(fixture.changes == std::vector<float>{210, 150});
    CHECK(fixture.splitter.first().getWidth() == 150);

    // Stacked: up and down, and the second pane kept moves the other way
    fixture.splitter.orientation(Orientation::Vertical).kept(SplitterPane::Second).position(100);
    fixture.layout();
    CHECK(fixture.splitter.divider().classNames().contains("vertical"));
    CHECK(fixture.splitter.divider().computedStyle().cursor() == Cursor::VResize);
    CHECK(fixture.splitter.second().getHeight() == 100);
    CHECK(fixture.splitter.first().getWidth() == 600);
    fixture.key(Key::UP);
    CHECK(fixture.splitter.position() == 110);
    CHECK(fixture.splitter.second().getHeight() == 110);
    CHECK(fixture.splitter.divider().y() == 290 - (fixture.splitter.divider().getHeight() / 2));
}
