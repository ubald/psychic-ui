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
        virtual TextBase *setText(const std::string &text) = 0;
        bool lcdRender() const;
        TextBase *setLcdRender(bool lcdRender);
        bool subPixelText() const;
        TextBase *setSubPixelText(bool subPixelText);
        unsigned int getLineHeight() const;

    protected:
        bool    _lcdRender{true};
        bool    _subPixelText{true};
        float   _fontSize{0.0f};
        float   _lineHeight{0.0f};
        SkPaint _textPaint{};
        void styleUpdated() override;

        /**
         * Fit a measured content size to what Yoga allows in a measure function
         * @param content Size the content wants
         * @param available Size passed by Yoga
         * @param mode Measure mode passed by Yoga
         * @return available when Exactly, the smallest of both when AtMost, content otherwise
         */
        static float fit(float content, float available, YGMeasureMode mode);

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

