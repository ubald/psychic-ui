#include <algorithm>
#include <any>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "../backends/fake_system_window.hpp"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/DragAndDrop.hpp>
#include <psychic-ui/DragEvent.hpp>
#include <psychic-ui/DragPayload.hpp>
#include <psychic-ui/DragPreview.hpp>
#include <psychic-ui/Overlay.hpp>
#include <psychic-ui/Popup.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>

using namespace psychic_ui;
using namespace psychic_ui_tests;

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

TEST_CASE("a drag session holds its target weakly, while dragging only", "[drag]") {
    Session s;
    auto    target = std::make_shared<Div>();
    s.press(10, 10);
    s.session.setTarget(target); // Armed: not yet
    CHECK(s.session.target() == nullptr);
    REQUIRE(s.session.move(40, 40));
    s.session.setTarget(target);
    CHECK(s.session.target() == target);
    target.reset();
    CHECK(s.session.target() == nullptr);

    auto other = std::make_shared<Div>();
    s.session.setTarget(other);
    s.session.cancel();
    CHECK(s.session.target() == nullptr);
}

TEST_CASE("a drag event defaults to an unaccepted pointer event", "[drag]") {
    const DragEvent event{};
    CHECK(event.payload == nullptr);
    CHECK(event.source == InputMode::Pointer);
    CHECK_FALSE(event.accepted);
}

namespace {
    /**
     * A window with a list of two drag sources (`item0`, `item1`) on the left, and two drop targets: `warm` takes
     * the sources' payload type and holds a child that takes nothing (`inner`), `cold` takes another type only.
     * Every drag signal, and every click on the sources, is logged.
     */
    struct Dragging {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            sys{window, 800, 600};
        Div                        *list{nullptr};
        Div                        *item0{nullptr};
        Div                        *item1{nullptr};
        Div                        *warm{nullptr};
        Div                        *inner{nullptr};
        Div                        *cold{nullptr};
        std::vector<std::string>    log{};
        std::vector<DragEvent>      events{};   // Their payloads are only valid during the call: see `carried`
        std::vector<std::string>    carried{};  // Each event's payload's data
        int                         sourceCalls{0};
        int                         clicks{0};
        std::shared_ptr<Div>        previewDiv{}; // The sources' payloads' preview, when set

        static Div &box(Div &parent, float left, float top, float width, float height) {
            return parent.add<Div>().position(Position::Absolute).left(left).top(top).width(width).height(height);
        }

        Div &source(const std::string &name) {
            auto &item = list->add<Div>().height(40).shrink(0);
            item.dragSource([this, name](const MouseEvent &) -> std::optional<DragPayload> {
                ++sourceCalls;
                return DragPayload{.type = "test/chip", .data = std::any(name), .label = name, .preview = previewDiv};
            });
            item.onClick([this] { ++clicks; });
            listen(item, name);
            return item;
        }

        void listen(Div &div, const std::string &name) {
            const auto logged = [this, name](const std::string &what) {
                return [this, name, what](const DragEvent &event) {
                    log.push_back(name + " " + what + (what == "end" && event.accepted ? " accepted" : ""));
                    events.push_back(event);
                    carried.push_back(event.payload ? std::any_cast<std::string>(event.payload->data) : "");
                };
            };
            div.onDragStart(logged("start"))
                .onDragEnd(logged("end"))
                .onDragEnter(logged("enter"))
                .onDragMove(logged("move"))
                .onDragExit(logged("exit"))
                .onDrop(logged("drop"));
        }

        Dragging() {
            auto &app = window->appContainer();
            list      = &box(app, 0, 0, 100, 200).column();
            item0     = &source("item0");
            item1     = &source("item1");
            warm      = &box(app, 200, 0, 100, 100).dropTarget([](const DragPayload &payload) {
                return payload.type == "test/chip";
            });
            inner     = &box(*warm, 50, 50, 40, 40).dropTarget([](const DragPayload &) { return false; });
            cold      = &box(app, 400, 0, 100, 100).dropTarget([](const DragPayload &payload) {
                return payload.type == "other";
            });
            listen(*warm, "warm");
            listen(*inner, "inner");
            listen(*cold, "cold");
            sys.layout();
        }

        void press(float x, float y) const {
            window->handleMouseButtonChange({.x = x, .y = y, .button = MouseButton::LEFT, .down = true});
        }

        void move(float x, float y) const {
            window->handleMouseMove({.x = x, .y = y}, false);
        }

        void release(float x, float y) const {
            window->handleMouseButtonChange({.x = x, .y = y, .button = MouseButton::LEFT, .down = false});
        }

        /**
         * Presses on the first item and drags past the threshold
         */
        void startDrag() {
            press(50, 20);
            move(60, 20);
            REQUIRE(window->dragAndDrop().active());
        }

        /**
         * Where the div's content starts, in window coordinates
         */
        static std::pair<float, float> origin(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y);
            return {static_cast<float>(x), static_cast<float>(y)};
        }

        static bool has(const Div &div, const std::string &className) {
            return div.classNames().contains(className);
        }
    };
}

TEST_CASE("dragging past the threshold starts a drag and marks the source", "[drag]") {
    Dragging drag;
    drag.press(50, 20);
    drag.move(53, 20); // Not past the threshold
    CHECK_FALSE(drag.window->dragAndDrop().active());
    CHECK_FALSE(Dragging::has(*drag.item0, "dragging"));
    CHECK(drag.sourceCalls == 0);

    drag.move(60, 22);
    CHECK(drag.window->dragAndDrop().active());
    CHECK(drag.sourceCalls == 1);
    CHECK(Dragging::has(*drag.item0, "dragging"));
    CHECK_FALSE(Dragging::has(*drag.item1, "dragging"));
    REQUIRE(drag.log == std::vector<std::string>{"item0 start"});
    const DragEvent &started = drag.events.back();
    const auto [item0X, item0Y] = Dragging::origin(*drag.item0);
    CHECK(started.x == 60.0f - item0X);
    CHECK(started.y == 22.0f - item0Y);
    CHECK(started.windowX == 60.0f);
    CHECK(drag.carried.back() == "item0");
    CHECK(drag.window->dragAndDrop().source().get() == drag.item0);

    drag.release(60, 22);
    CHECK_FALSE(drag.window->dragAndDrop().active());
    CHECK_FALSE(Dragging::has(*drag.item0, "dragging"));
    CHECK(drag.log == std::vector<std::string>{"item0 start", "item0 end"});
    CHECK(drag.clicks == 0); // The press's click is the drag's

    // A drag from the second item: its coordinates are its own
    drag.events.clear();
    drag.press(50, 60);
    drag.move(50, 70);
    REQUIRE(drag.window->dragAndDrop().active());
    CHECK(drag.window->dragAndDrop().source().get() == drag.item1);
    CHECK(drag.events.back().y == 70.0f - Dragging::origin(*drag.item1).second);
    CHECK(drag.events.back().windowY == 70.0f);
    CHECK(drag.carried.back() == "item1");
}

TEST_CASE("a press that doesn't move is still a click", "[drag]") {
    const Dragging drag;
    drag.press(50, 20);
    drag.move(52, 21);
    drag.release(52, 21);
    CHECK(drag.clicks == 1);
    CHECK(drag.sourceCalls == 0);
    CHECK(drag.log.empty());
    CHECK_FALSE(drag.window->dragAndDrop().active());
    CHECK(drag.window->dragAndDrop().state() == DragAndDrop::State::Idle);
}

TEST_CASE("only accepting targets enter, move, exit and drop", "[drag]") {
    Dragging drag;
    drag.startDrag();
    drag.log.clear();

    drag.move(450, 50); // Over the target that refuses
    CHECK(drag.log.empty());

    drag.move(220, 30); // Over the one that accepts
    CHECK(drag.log == std::vector<std::string>{"warm enter", "warm move"});
    const auto [warmX, warmY] = Dragging::origin(*drag.warm);
    CHECK(drag.events.back().x == 220.0f - warmX);
    CHECK(drag.events.back().y == 30.0f - warmY);
    CHECK(Dragging::has(*drag.warm, "drag-over"));

    drag.move(230, 30);
    drag.move(260, 60); // Over its child, which refuses: still the accepting one
    CHECK(drag.log == std::vector<std::string>{"warm enter", "warm move", "warm move", "warm move"});
    CHECK_FALSE(Dragging::has(*drag.inner, "drag-over"));

    drag.move(450, 50);
    CHECK(drag.log.back() == "warm exit");
    CHECK_FALSE(Dragging::has(*drag.warm, "drag-over"));

    drag.log.clear();
    drag.move(220, 30);
    drag.release(222, 30);
    CHECK(drag.log == std::vector<std::string>{
        "warm enter", "warm move", "warm move", "warm drop", "item0 end accepted"
    });
    CHECK_FALSE(Dragging::has(*drag.warm, "drag-over"));
    const DragEvent &drop = drag.events[drag.events.size() - 2];
    CHECK(drop.x == 222.0f - warmX);
    CHECK(drag.carried[drag.carried.size() - 2] == "item0");
    CHECK(drag.events.back().accepted);
    CHECK(drag.clicks == 0);
}

TEST_CASE("accepting targets show drop-target while the drag lasts", "[drag]") {
    Dragging drag;
    CHECK_FALSE(Dragging::has(*drag.warm, "drop-target"));
    drag.startDrag();
    CHECK(Dragging::has(*drag.warm, "drop-target"));
    CHECK_FALSE(Dragging::has(*drag.inner, "drop-target"));
    CHECK_FALSE(Dragging::has(*drag.cold, "drop-target"));
    drag.move(220, 30);
    CHECK(Dragging::has(*drag.warm, "drop-target"));
    drag.release(600, 300);
    CHECK_FALSE(Dragging::has(*drag.warm, "drop-target"));
}

TEST_CASE("dropping outside a target ends without a drop", "[drag]") {
    Dragging drag;
    drag.startDrag();
    drag.move(220, 30);
    drag.log.clear();
    drag.move(600, 300);
    drag.release(600, 300);
    CHECK(drag.log == std::vector<std::string>{"warm exit", "item0 end"});
    CHECK_FALSE(drag.events.back().accepted);
    CHECK_FALSE(drag.window->dragAndDrop().active());
    CHECK(drag.clicks == 0);

    // Dropped on the target that refuses: no drop either
    drag.log.clear();
    drag.startDrag();
    drag.move(450, 50);
    drag.release(450, 50);
    CHECK(drag.log == std::vector<std::string>{"item0 start", "item0 end"});
}

TEST_CASE("escape cancels a drag", "[drag]") {
    Dragging drag;
    drag.startDrag();
    drag.move(220, 30);
    drag.log.clear();

    SECTION("escape") {
        CHECK(drag.window->handleKeyPress(Key::ESCAPE, Mod{}));
    }

    SECTION("the window losing the focus") {
        drag.window->windowDeactivated();
    }

    CHECK_FALSE(drag.window->dragAndDrop().active());
    CHECK(drag.log == std::vector<std::string>{"warm exit", "item0 end"});
    CHECK_FALSE(Dragging::has(*drag.warm, "drag-over"));
    CHECK_FALSE(Dragging::has(*drag.warm, "drop-target"));
    CHECK_FALSE(Dragging::has(*drag.item0, "dragging"));

    // The rest of the press drags nothing and clicks nothing
    drag.move(230, 30);
    drag.release(230, 30);
    drag.move(52, 20);
    drag.release(52, 20);
    CHECK(drag.log.size() == 2);
    CHECK(drag.clicks == 0);
}

TEST_CASE("the cursor shows whether a drop is possible", "[drag]") {
    Dragging drag;
    drag.move(50, 20);
    const Cursor before = drag.sys.cursor;
    drag.startDrag();
    CHECK(drag.sys.cursor == Cursor::NotAllowed);
    drag.move(220, 30);
    CHECK(drag.sys.cursor == Cursor::Move);
    drag.move(450, 50);
    CHECK(drag.sys.cursor == Cursor::NotAllowed);
    drag.move(220, 30);
    drag.release(220, 30);
    CHECK(drag.sys.cursor == before);
}

TEST_CASE("a target removed mid-drag stops receiving", "[drag]") {
    Dragging drag;
    const auto keep = drag.warm->shared_from_this(); // Still alive, out of the window
    drag.startDrag();
    drag.move(220, 30);
    drag.log.clear();
    drag.window->appContainer().remove(*drag.warm);
    drag.sys.layout();

    drag.move(230, 30);
    drag.move(240, 30);
    drag.release(240, 30);
    CHECK(drag.log == std::vector<std::string>{"item0 end"});
    CHECK_FALSE(drag.events.back().accepted);
    CHECK_FALSE(Dragging::has(*keep, "drag-over"));
    CHECK_FALSE(Dragging::has(*keep, "drop-target"));
}

TEST_CASE("the source removed mid-drag cancels the drag", "[drag]") {
    Dragging drag;
    drag.startDrag();
    drag.move(220, 30);
    drag.log.clear();
    drag.list->remove(*drag.item0); // Gone: nothing else holds it
    drag.sys.layout();

    drag.move(230, 30);
    CHECK_FALSE(drag.window->dragAndDrop().active());
    CHECK(drag.log == std::vector<std::string>{"warm exit"});
    CHECK_FALSE(Dragging::has(*drag.warm, "drag-over"));
    CHECK_FALSE(Dragging::has(*drag.warm, "drop-target"));
    drag.release(230, 30);
    CHECK(drag.log == std::vector<std::string>{"warm exit"});
}

TEST_CASE("targets under a modal popup aren't reachable", "[drag][modal]") {
    Dragging drag;
    drag.startDrag();
    drag.window->overlay().open<Dialog>("Modal");
    drag.sys.layout();
    REQUIRE(drag.window->overlay().modal());
    drag.log.clear();

    drag.move(220, 30);
    CHECK(drag.log.empty());
    CHECK(drag.sys.cursor == Cursor::NotAllowed);
    drag.release(220, 30);
    CHECK(drag.log == std::vector<std::string>{"item0 end"});
}

TEST_CASE("hover doesn't change under a drag", "[drag]") {
    Dragging drag;
    int coldEntered = 0;
    int coldMoved   = 0;
    drag.cold->onMouseEnter([&] { ++coldEntered; }).onMouseMove([&] { ++coldMoved; });
    drag.move(50, 20);
    CHECK(drag.item0->mouseOver());
    drag.startDrag();

    drag.move(450, 50);
    drag.move(460, 50);
    CHECK_FALSE(drag.cold->mouseOver());
    CHECK(coldEntered == 0);
    CHECK(coldMoved == 0);
    CHECK(drag.item0->mouseOver());

    // Once it ends, the next move hovers again
    drag.release(460, 50);
    drag.move(461, 50);
    CHECK(drag.cold->mouseOver());
    CHECK_FALSE(drag.item0->mouseOver());
    CHECK(coldEntered == 1);
}

namespace {
    /**
     * The drag previews in the window's overlay, open or leaving
     */
    std::size_t previews(const TestWindow &window) {
        return static_cast<std::size_t>(std::ranges::count_if(window.overlay().allPopups(), [](const auto &popup) {
            return dynamic_cast<const DragPreview *>(popup.get()) != nullptr;
        }));
    }

    /**
     * Where the popup is, in window coordinates
     */
    std::pair<float, float> placed(const TestWindow &window, const Popup &popup) {
        int x = 0;
        int y = 0;
        window.overlay().localToGlobal(x, y);
        return {static_cast<float>(x + popup.x()), static_cast<float>(y + popup.y())};
    }
}

TEST_CASE("the preview follows the pointer and closes at the end", "[drag]") {
    Dragging drag;
    CHECK(drag.window->dragPreview() == nullptr);
    drag.startDrag();
    drag.sys.layout();

    const DragPreview *preview = drag.window->dragPreview();
    REQUIRE(preview != nullptr);
    CHECK(preview->isOpen());
    CHECK(previews(*drag.window) == 1);
    CHECK_FALSE(preview->mouseEnabled());
    CHECK_FALSE(preview->mouseChildren());
    // The payload's label, beside the pointer
    REQUIRE(preview->label() != nullptr);
    CHECK(preview->label()->text() == "item0");
    CHECK(preview->content() == preview->label());
    CHECK(placed(*drag.window, *preview) == std::pair{60.0f + DragPreview::pointerOffset, 20.0f + DragPreview::pointerOffset});

    drag.move(300, 200);
    drag.sys.layout();
    CHECK(drag.window->dragPreview() == preview);
    CHECK(placed(*drag.window, *preview) == std::pair{300.0f + DragPreview::pointerOffset, 200.0f + DragPreview::pointerOffset});

    drag.release(300, 200);
    drag.sys.layout();
    CHECK(drag.window->dragPreview() == nullptr);
    CHECK(previews(*drag.window) == 0);

    // A cancelled drag closes it too
    drag.startDrag();
    drag.sys.layout();
    CHECK(previews(*drag.window) == 1);
    drag.window->cancelDrag();
    drag.sys.layout();
    CHECK(drag.window->dragPreview() == nullptr);
    CHECK(previews(*drag.window) == 0);
}

TEST_CASE("the preview shows the payload's own div when given", "[drag]") {
    Dragging drag;
    drag.previewDiv = std::make_shared<Div>();
    drag.previewDiv->size(30, 20);
    drag.startDrag();
    drag.sys.layout();

    const DragPreview *preview = drag.window->dragPreview();
    REQUIRE(preview != nullptr);
    CHECK(preview->label() == nullptr);
    CHECK(preview->content() == drag.previewDiv.get());
    CHECK(drag.previewDiv->parent() == preview);
    CHECK(preview->children().size() == 1);

    drag.release(60, 20);
    drag.sys.layout();
    CHECK(previews(*drag.window) == 0);
}

TEST_CASE("a pen drags like a mouse", "[drag]") {
    Dragging drag;
    const auto pen = [&drag](float x, float y, std::optional<bool> down) {
        MouseEvent event{.x = x, .y = y};
        event.source = InputMode::Pen;
        if (down) {
            event.button = MouseButton::LEFT;
            event.down   = *down;
            drag.window->handleMouseButtonChange(event);
        } else {
            drag.window->handleMouseMove(event, false);
        }
    };
    pen(50, 20, true);
    pen(53, 20, std::nullopt); // Not past the threshold
    CHECK_FALSE(drag.window->dragAndDrop().active());
    pen(60, 20, std::nullopt);
    REQUIRE(drag.window->dragAndDrop().active());
    CHECK(drag.window->dragAndDrop().inputMode() == InputMode::Pen);
    drag.sys.layout();
    REQUIRE(drag.window->dragPreview() != nullptr);
    CHECK(placed(*drag.window, *drag.window->dragPreview())
          == std::pair{60.0f + DragPreview::pointerOffset, 20.0f + DragPreview::pointerOffset});

    pen(220, 30, std::nullopt);
    pen(220, 30, false);
    CHECK(drag.log == std::vector<std::string>{
        "item0 start", "warm enter", "warm move", "warm move", "warm drop", "item0 end accepted"
    });
    CHECK(drag.events.back().source == InputMode::Pen);
    CHECK(drag.clicks == 0);
}

namespace {
    /**
     * A source row, 300 x 40, holding a Range on its right half: its left half is plain
     */
    struct ControlInSource {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            sys{window, 800, 600};
        Div                        &row{window->appContainer().add<Div>()};
        Div                        &plain{row.add<Div>().width(150).height(40)};
        Range<float>               &range{row.add<Range<float>>(0.0f, 1.0f, 0.5f, nullptr).width(150)};
        int                         sourceCalls{0};
        int                         starts{0};

        ControlInSource() {
            row.direction(Direction::Row).width(300).height(40).alignItems(Align::Center);
            row.dragSource([this](const MouseEvent &) -> std::optional<DragPayload> {
                ++sourceCalls;
                return DragPayload{.type = "test/row", .label = "row"};
            });
            row.onDragStart([this] { ++starts; });
            sys.layout();
        }

        /**
         * The range's center, in window coordinates
         */
        [[nodiscard]] std::pair<float, float> rangeCenter() const {
            int x = 0;
            int y = 0;
            range.localToGlobal(x, y, range.getWidth() / 2, range.getHeight() / 2);
            return {static_cast<float>(x), static_cast<float>(y)};
        }
    };
}

TEST_CASE("a control inside a source keeps its press: no drag starts", "[drag]") {
    ControlInSource fixture;
    const auto [x, y] = fixture.rangeCenter();
    fixture.window->handleMouseButtonChange({.x = x, .y = y, .button = MouseButton::LEFT, .down = true});
    CHECK(fixture.window->dragAndDrop().state() == DragAndDrop::State::Idle);
    fixture.window->handleMouseMove({.x = x + 10, .y = y}, false);
    const float before = fixture.range.value();
    fixture.window->handleMouseMove({.x = x + 30, .y = y}, false);
    CHECK_FALSE(fixture.window->dragAndDrop().active());
    CHECK(fixture.sourceCalls == 0);
    CHECK(fixture.starts == 0);
    CHECK_FALSE(fixture.row.classNames().contains("dragging"));
    CHECK(fixture.range.value() > before); // The range followed the pointer
    fixture.window->handleMouseButtonChange({.x = x + 30, .y = y, .button = MouseButton::LEFT, .down = false});

    // A press on the row's plain part still drags
    fixture.window->handleMouseButtonChange({.x = 20, .y = 20, .button = MouseButton::LEFT, .down = true});
    fixture.window->handleMouseMove({.x = 30, .y = 20}, false);
    CHECK(fixture.window->dragAndDrop().active());
    CHECK(fixture.starts == 1);
    CHECK(fixture.row.classNames().contains("dragging"));
    fixture.window->handleMouseButtonChange({.x = 30, .y = 20, .button = MouseButton::LEFT, .down = false});
    CHECK_FALSE(fixture.window->dragAndDrop().active());
}

TEST_CASE("another button's press during a drag cancels it and goes nowhere", "[drag]") {
    Dragging drag;
    int contextRequests = 0;
    std::vector<MouseButton> buttons{};
    drag.item0->onContextMenuRequest([&contextRequests] { ++contextRequests; });
    drag.warm->onContextMenuRequest([&contextRequests] { ++contextRequests; });
    drag.warm->onMouseButtonChange([&buttons](const MouseEvent &event) { buttons.push_back(event.button); });

    for (const MouseButton other: {MouseButton::RIGHT, MouseButton::MIDDLE}) {
        drag.log.clear();
        drag.startDrag();
        drag.move(220, 30);
        drag.window->handleMouseButtonChange({.x = 220, .y = 30, .button = other, .down = true});
        CHECK_FALSE(drag.window->dragAndDrop().active());
        CHECK(drag.log == std::vector<std::string>{"item0 start", "warm enter", "warm move", "warm exit", "item0 end"});
        drag.window->handleMouseButtonChange({.x = 220, .y = 30, .button = other, .down = false});
        drag.release(220, 30); // The left button's release, after: no drop, no click
        CHECK(drag.log.size() == 5);
        CHECK(contextRequests == 0);
        CHECK(buttons.empty());
        CHECK(drag.clicks == 0);
    }
}
