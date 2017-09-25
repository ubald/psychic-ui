#include "Text.hpp"

namespace psychic_ui {
    Text::Text(const std::string &text) :
        _text(&text) {
        setTag("Text");
        setMeasurable();
        _defaultStyle
            ->set(shrink, 0)
            ->set(grow, 0);
        _textBox.setPaint(_textPaint);
    }

    const std::string &Text::getText() const {
        return *_text;
    }

    void Text::setText(const std::string &text) {
        _text = &text;
    }

    void Text::reflow() {
        invalidate();
    }
}
