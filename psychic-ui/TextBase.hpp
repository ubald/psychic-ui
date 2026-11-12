#pragma once

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
    };
}

