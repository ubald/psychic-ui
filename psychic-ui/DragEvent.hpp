#pragma once

#include "psychic-ui.hpp"

namespace psychic_ui {

    struct DragPayload;

    /**
     * What the drag signals pass, to the source and the targets. Like a MouseEvent, each div gets it in its own
     * coordinates.
     */
    struct DragEvent {
        /**
         * The pointer, in the receiving div's content coordinates
         */
        float              x{0};
        float              y{0};
        /**
         * The pointer, in window coordinates, whichever div receives it
         */
        float              windowX{0};
        float              windowY{0};
        /**
         * What is dragged, valid during the call
         */
        const DragPayload *payload{nullptr};
        /**
         * What drags it: the mouse, a finger or a pen
         */
        InputMode          source{InputMode::Pointer};
        /**
         * For the end of a drag: whether a target took the drop
         */
        bool               accepted{false};
    };

}
