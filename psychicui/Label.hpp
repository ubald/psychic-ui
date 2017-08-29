#pragma once

#include <string>
#include <SkPaint.h>
#include <SkTextBox.h>
#include "Component.hpp"

namespace psychicui {
    class Label: public Component {
    public:
        explicit Label(const std::string &text);

        std::string text();
        void setText(const std::string &text);

        YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) override;
        void draw(SkCanvas *canvas) override;

    protected:
        std::string _text;
        SkPaint _paint;
        SkTextBox _textBox;
    };
}
