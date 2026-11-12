#pragma once

#include <tuple>
#include <SkPaint.h>
#include <SkFont.h>
#include "Div.hpp"

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

        bool lcdRender() const;

        template<class Self>
        Self &lcdRender(this Self &self, detail::BoolArg lcdRender) {
            static_cast<TextBase &>(self).updateTextFlags(lcdRender.value, static_cast<TextBase &>(self)._subPixelText);
            return self;
        }

        bool subPixelText() const;

        template<class Self>
        Self &subPixelText(this Self &self, detail::BoolArg subPixelText) {
            static_cast<TextBase &>(self).updateTextFlags(static_cast<TextBase &>(self)._lcdRender, subPixelText.value);
            return self;
        }

        unsigned int getLineHeight() const;

    protected:
        bool    _lcdRender{true};
        bool    _subPixelText{true};
        float   _fontSize{0.0f};
        float   _lineHeight{0.0f};
        SkPaint _textPaint{};
        void styleUpdated() override;
        virtual void updateText(const std::string &text) = 0;
        void updateTextFlags(bool lcdRender, bool subPixelText);

        /**
         * Height of the given number of lines, never less than the font spacing
         * so that a single line's glyphs fit even when the line height is smaller
         * @param lines Line count
         * @return Height in pixels
         */
        float linesHeight(size_t lines) const;

    private:
        /**
         * What changes the text metrics: typeface, size, letter spacing, line height, subpixel positioning
         */
        using FontKey = std::tuple<const SkTypeface *, float, float, float, bool>;
        FontKey _fontKey{};
    };
}

