#pragma once

#include <SkPaint.h>
#include "Div.hpp"

namespace psychic_ui {

    /**
     * @class TextBase
     *
     * Base class for text displaying components
     */
    class TextBase : public Div {
    public:
        explicit TextBase(const std::string &text = "");
        const std::string &text() const;
        virtual void setText(const std::string &text);
        const bool lcdRender() const;
        TextBase *setLcdRender(bool lcdRender);
        const bool subpixelText() const;
        TextBase *setSubPixelText(bool subPixelText);
        void styleUpdated() override;

    protected:
        std::string _text;
        bool        _lcdRender{true};
        bool        _subPixelText{true};
        float       _fontSize{0.0f};
        float       _lineHeight{0.0f};
        SkPaint     _textPaint{};
    };
}

