#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include "DragPayload.hpp"
#include "psychic-ui.hpp"

namespace psychic_ui {

    class Div;

    /**
     * @class DragAndDrop
     *
     * A window's drag session, driven by the window's pointer input: idle, then armed by a press on a drag source,
     * then dragging once the pointer moves past `dragThreshold` from the press (or at once with `start`, as a long
     * press does) and the source gives a payload, then idle again on the release or `cancel`.
     *
     * Holds its source weakly: a source removed before the drag starts starts nothing, and one removed while
     * dragging ends the drag at the next move.
     */
    class DragAndDrop {
    public:
        enum class State : std::uint8_t { Idle, Armed, Dragging };

        /**
         * How far, in window units, the pointer moves from the press before a drag starts
         */
        static constexpr float dragThreshold = 4.0f;

        [[nodiscard]] State state() const { return _state; }

        /**
         * Whether a drag is live
         */
        [[nodiscard]] bool active() const { return _state == State::Dragging; }

        /**
         * What the live drag carries, null when no drag is live
         */
        [[nodiscard]] const DragPayload *payload() const;

        /**
         * The div the drag starts or started from, null when idle or once it's gone
         */
        [[nodiscard]] std::shared_ptr<Div> source() const;

        /**
         * What presses and drags: the mouse, a finger or a pen
         */
        [[nodiscard]] InputMode inputMode() const { return _inputMode; }

        /**
         * The pointer, in window coordinates, as the last press or move left it
         */
        [[nodiscard]] float windowX() const { return _windowX; }
        [[nodiscard]] float windowY() const { return _windowY; }

        /**
         * Arms the session for a press on `source`, which `sourceFn` asks for a payload when the drag starts.
         * `press` is in the source's coordinates, with the pointer's window coordinates. A press while dragging
         * changes nothing.
         */
        void press(const std::weak_ptr<Div> &source, const MouseEvent &press, DragSourceFn sourceFn);

        /**
         * Moves the pointer: an armed session starts the drag once past `dragThreshold`
         * @return whether a drag is live after the move, which then belongs to it
         */
        bool move(float windowX, float windowY);

        /**
         * Starts the armed drag now, whatever the distance: asks the source for its payload
         * @return whether a drag started (false when not armed, or when the source is gone or refuses)
         */
        bool start();

        /**
         * Releases the pointer, back to idle
         * @return whether a drag was live, which the release then ends (no click); false when only armed
         */
        bool release(float windowX, float windowY);

        /**
         * Ends a live drag without a drop, or disarms the session
         */
        void cancel();

    private:
        State                      _state{State::Idle};
        std::weak_ptr<Div>         _source{};
        DragSourceFn               _sourceFn{};
        MouseEvent                 _press{};
        std::optional<DragPayload> _payload{};
        InputMode                  _inputMode{InputMode::Pointer};
        float                      _windowX{0};
        float                      _windowY{0};

        /**
         * Back to idle, forgetting the source and the payload
         */
        void reset();
    };

}
