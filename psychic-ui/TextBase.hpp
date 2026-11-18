#pragma once

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPaint.h"
#include "Div.hpp"
#include "psychic-ui/utils/TextLayout.hpp"

namespace psychic_ui {

    /**
     * @class TextBase
     *
     * Base class for text displaying components
     */
    class TextBase : public Div {
    public:
        TextBase();

        /**
         * Set the text (subclasses implement `updateText`)
         */
        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<TextBase &>(self).updateText(text);
            return self;
        }

        unsigned int getLineHeight() const;

    protected:
        float   _fontSize{0.0f};
        float   _lineHeight{0.0f};
        SkFont  _font{};      // The font family's primary font, for its metrics
        SkPaint _textPaint{};
        /**
         * The text's layout (SkParagraph). Mutable: its queries lay the text out on demand.
         */
        mutable TextLayout _textLayout{};

        void styleUpdated() override;
        virtual void updateText(const std::string &text) = 0;

        /**
         * Height of the given number of lines, never less than the font spacing
         * so that a single line's glyphs fit even when the line height is smaller
         * @param lines Line count
         * @return Height in pixels
         */
        float linesHeight(size_t lines) const;

    private:
        TextLayoutStyle _layoutStyle{}; // What the text was last measured with
    };
}

