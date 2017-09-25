#pragma once

#include <string>
#include <SkPaint.h>
#include <SkTextBox.h>
#include <SkTextBlob.h>
#include "psychic-ui/Div.hpp"
#include "psychic-ui/utils/TextBox.hpp"

namespace psychic_ui {
    class Label: public Div {
    public:
        explicit Label(const std::string &text = "");

        const std::string &text() const;
        void setText(const std::string &text);

        const bool lcdRender() const;
        Label * setLcdRender( bool lcdRender);

        const bool subpixelText() const;
        Label * setSubPixelText(bool subPixelText);

        void styleUpdated() override;
        void layoutUpdated() override;

        YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) override;
        void draw(SkCanvas *canvas) override;

    protected:
        std::string _text;
        bool _lcdRender{true};
        bool _subPixelText{true};
        float _lineHeight{0.0f};
        SkPaint _textPaint{};
        TextBox _textBox{};
        std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob*)>> _blob{nullptr};
    };
}
