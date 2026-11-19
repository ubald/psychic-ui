#pragma once

#include "DragPayload.hpp"
#include "Popup.hpp"
#include "psychic-ui.hpp"

namespace psychic_ui {

    class Label;

    /**
     * @class DragPreview
     *
     * What a drag shows under the pointer while it lasts: the payload's `preview` div when it has one, else a chip
     * with its `label`. Placed beside the pointer, below and right of it, or above a finger (where the finger doesn't
     * hide it). It takes no input and leaves the focus where it is. The window opens one when a drag starts, moves it
     * with every move and closes it when the drag ends (`Window::dragPreview`).
     */
    class DragPreview : public Popup {
    public:
        explicit DragPreview(const DragPayload &payload);

        /**
         * How far right of and below the pointer it goes
         */
        static constexpr float pointerOffset = 12.0f;

        /**
         * How far above a finger it goes
         */
        static constexpr float fingerOffset = 32.0f;

        /**
         * Place it at the pointer, in window coordinates, for what points: above a finger, beside the mouse or a pen
         */
        template<class Self>
        Self &pointer(this Self &self, float x, float y, InputMode mode) {
            static_cast<DragPreview &>(self).updatePointer(x, y, mode);
            return self;
        }

        /**
         * What it shows: the payload's preview, or its label
         */
        Div *content() const { return _content; }

        /**
         * The label chip, `.label`, or nullptr when it shows the payload's preview
         */
        Label *label() const { return _label; }

    protected:
        Div   *_content{nullptr}; // A child: our child list owns it
        Label *_label{nullptr};

        void updatePointer(float x, float y, InputMode mode);
    };
}
