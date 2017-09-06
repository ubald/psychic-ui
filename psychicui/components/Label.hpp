#pragma once

#include <string>
#include <SkPaint.h>
#include <SkTextBox.h>
#include "psychicui/Div.hpp"

namespace psychicui {
    class Label: public Div {
    public:
        explicit Label(const std::string &text = "");

        const std::string &text() const;
        void setText(const std::string &text);

        const bool lcdRender() const;
        Label * setLcdRender(const bool lcdRender);

        const bool subpixelText() const;
        Label * setSubpixelText(const bool subpixelText);

        void styleUpdated() override;

        YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) override;
        void draw(SkCanvas *canvas) override;

    protected:
        std::string _text;
        bool _antiAlias{true};
        bool _lcdRender{true};
        bool _subpixelText{true};
        float _lineHeight{0.0f};
        SkPaint _textPaint;
        SkTextBox _textBox;
    };
}
