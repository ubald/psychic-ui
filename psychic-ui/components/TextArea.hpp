#pragma once

#include <string>
#include "psychic-ui/Component.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/components/TextField.hpp"

namespace psychic_ui {
    class TextArea;

    class TextAreaSkin : public Skin<TextArea> {
    public:
        TextAreaSkin() : Skin<TextArea>() {
            tag("TextAreaSkin");
        }

        /**
         * The `.text` part, which the skin creates and owns inside its own scrolling viewport: the view showing the
         * text, the selection and the caret
         */
        virtual Text *textView() = 0;
    };

    /**
     * @class TextArea
     *
     * A text field on several lines: the text wraps, Enter inserts a new line, and the caret scrolls into view in
     * the skin's viewport.
     */
    class TextArea : public TextField<TextAreaSkin> {
    public:
        explicit TextArea(const std::string &text = "");
    };
}
