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
        TextGeometry() = default;
        TextGeometry(const TextGeometry &) = delete;
        TextGeometry &operator=(const TextGeometry &) = delete;
        TextGeometry(TextGeometry &&) = delete;
        TextGeometry &operator=(TextGeometry &&) = delete;
        virtual ~TextGeometry() = default;

        /**
         * The caret before the character at the index: a zero-width rectangle across its line
         */
        [[nodiscard]] virtual SkRect caretRect(unsigned int index) const = 0;

        /**
         * The caret index closest to a point
         */
        [[nodiscard]] virtual unsigned int indexAt(float x, float y) const = 0;
    };
}
