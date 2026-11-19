#include <any>
#include <memory>
#include <optional>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/DragAndDrop.hpp>
#include <psychic-ui/DragEvent.hpp>
#include <psychic-ui/DragPayload.hpp>
#include <psychic-ui/psychic-ui.hpp>

using namespace psychic_ui;

namespace {
    /**
     * A session with a source whose function counts its calls and gives a payload, or refuses
     */
    struct Session {
        DragAndDrop          session{};
        std::shared_ptr<Div> source{std::make_shared<Div>()};
        int                  calls{0};
        bool                 refuse{false};
        MouseEvent           seen{};

        DragSourceFn sourceFn() {
            return [this](const MouseEvent &press) -> std::optional<DragPayload> {
                ++calls;
                seen = press;
                if (refuse) {
                    return std::nullopt;
                }
                return DragPayload{.type = "test/chip", .data = std::any(42), .label = "Chip"};
            };
        }

        void press(float x, float y, InputMode mode = InputMode::Pointer) {
            MouseEvent event{.x = x - 5, .y = y - 5, .windowX = x, .windowY = y};
            event.button = MouseButton::LEFT;
            event.down   = true;
            event.source = mode;
            session.press(source, event, sourceFn());
        }
    };
}

TEST_CASE("a drag session is idle until a press on a source", "[drag]") {
    const DragAndDrop session;
    CHECK(session.state() == DragAndDrop::State::Idle);
    CHECK_FALSE(session.active());
    CHECK(session.payload() == nullptr);
    CHECK(session.source() == nullptr);
    CHECK(DragAndDrop::dragThreshold == 4.0f);
}

TEST_CASE("a press arms the session, which drags only past the threshold", "[drag]") {
    Session s;
    s.press(10, 10);
    CHECK(s.session.state() == DragAndDrop::State::Armed);
    CHECK_FALSE(s.session.active());
    CHECK(s.session.payload() == nullptr);
    CHECK(s.session.source() == s.source);

    // 4 px away is not past the threshold
    CHECK_FALSE(s.session.move(14, 10));
    CHECK_FALSE(s.session.move(10, 6));
    CHECK(s.session.state() == DragAndDrop::State::Armed);
    CHECK(s.calls == 0);

    // About 4.24 px away is
    CHECK(s.session.move(13, 13));
    CHECK(s.session.state() == DragAndDrop::State::Dragging);
    CHECK(s.session.active());
    CHECK(s.calls == 1);
    CHECK(s.seen.x == 5.0f); // The press as the source got it
    CHECK(s.seen.windowX == 10.0f);
    REQUIRE(s.session.payload() != nullptr);
    CHECK(s.session.payload()->type == "test/chip");
    CHECK(std::any_cast<int>(s.session.payload()->data) == 42);
    CHECK(s.session.payload()->label == "Chip");
    CHECK(s.session.windowX() == 13.0f);
    CHECK(s.session.windowY() == 13.0f);
    CHECK(s.session.inputMode() == InputMode::Pointer);

    // Moves while dragging are the drag's, and the source is asked once
    CHECK(s.session.move(11, 11));
    CHECK(s.session.windowX() == 11.0f);
    CHECK(s.calls == 1);
}

TEST_CASE("a release before the threshold leaves no drag", "[drag]") {
    Session s;
    s.press(10, 10);
    CHECK_FALSE(s.session.move(12, 11));
    CHECK_FALSE(s.session.release(12, 11)); // Not the drag's: the click happens
    CHECK(s.session.state() == DragAndDrop::State::Idle);
    CHECK(s.session.source() == nullptr);
    CHECK(s.calls == 0);

    // A move after the release starts nothing
    CHECK_FALSE(s.session.move(40, 40));
    CHECK(s.session.state() == DragAndDrop::State::Idle);
    CHECK(s.calls == 0);
}

TEST_CASE("a release while dragging ends the drag", "[drag]") {
    Session s;
    s.press(10, 10);
    REQUIRE(s.session.move(30, 10));
    CHECK(s.session.release(32, 10)); // The drag's: no click
    CHECK(s.session.state() == DragAndDrop::State::Idle);
    CHECK_FALSE(s.session.active());
    CHECK(s.session.payload() == nullptr);
    CHECK(s.session.source() == nullptr);
}

TEST_CASE("cancel returns the session to idle", "[drag]") {
    Session s;

    SECTION("armed") {
        s.press(10, 10);
        s.session.cancel();
        CHECK(s.session.state() == DragAndDrop::State::Idle);
        CHECK_FALSE(s.session.move(40, 40));
        CHECK(s.calls == 0);
    }

    SECTION("dragging") {
        s.press(10, 10);
        REQUIRE(s.session.move(40, 40));
        s.session.cancel();
        CHECK(s.session.state() == DragAndDrop::State::Idle);
        CHECK(s.session.payload() == nullptr);
        CHECK_FALSE(s.session.release(40, 40));
    }

    SECTION("idle") {
        s.session.cancel();
        CHECK(s.session.state() == DragAndDrop::State::Idle);
    }
}

TEST_CASE("a source that gives no payload refuses the drag", "[drag]") {
    Session s;
    s.refuse = true;
    s.press(10, 10);
    CHECK_FALSE(s.session.move(40, 40));
    CHECK(s.calls == 1);
    CHECK(s.session.state() == DragAndDrop::State::Idle);
    CHECK_FALSE(s.session.release(40, 40));
}

TEST_CASE("a drag session doesn't keep its source alive", "[drag]") {
    Session s;

    SECTION("removed while armed: no drag starts") {
        s.press(10, 10);
        s.source.reset();
        CHECK_FALSE(s.session.move(40, 40));
        CHECK(s.session.state() == DragAndDrop::State::Idle);
        CHECK(s.calls == 0);
    }

    SECTION("removed while dragging: the drag ends") {
        s.press(10, 10);
        REQUIRE(s.session.move(40, 40));
        s.source.reset();
        CHECK(s.session.source() == nullptr);
        CHECK_FALSE(s.session.move(50, 50));
        CHECK(s.session.state() == DragAndDrop::State::Idle);
        CHECK(s.session.payload() == nullptr);
    }
}

TEST_CASE("start drags an armed session at once, as a long press does", "[drag]") {
    Session s;
    CHECK_FALSE(s.session.start()); // Nothing armed
    s.press(10, 10, InputMode::Touch);
    CHECK(s.session.start());
    CHECK(s.session.active());
    CHECK(s.session.inputMode() == InputMode::Touch);
    CHECK(s.calls == 1);
    CHECK_FALSE(s.session.start()); // Already dragging
    CHECK(s.calls == 1);
}

TEST_CASE("a press while dragging doesn't restart the drag", "[drag]") {
    Session s;
    s.press(10, 10);
    REQUIRE(s.session.move(40, 40));
    s.press(60, 60);
    CHECK(s.session.active());
    CHECK(s.session.windowX() == 40.0f);
    CHECK(s.calls == 1);
}

TEST_CASE("a drag event defaults to an unaccepted pointer event", "[drag]") {
    const DragEvent event{};
    CHECK(event.payload == nullptr);
    CHECK(event.source == InputMode::Pointer);
    CHECK_FALSE(event.accepted);
}
