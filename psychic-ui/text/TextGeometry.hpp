#pragma once

#include "include/core/SkRect.h"

namespace psychic_ui {

    /**
     * @class TextGeometry
     *
     * The two questions the editing model (`TextEditing`) asks what shows its text, in that view's own coordinates;
     * `Text` answers them
     */
    class TextGeometry {
    public:
        virtual ~TextGeometry() = default;

        /**
         * The caret before the character at the index: a zero-width rectangle across its line
         */
        virtual SkRect caretRect(unsigned int index) const = 0;

        /**
         * The caret index closest to a point
         */
        virtual unsigned int indexAt(float x, float y) const = 0;
    };
}
