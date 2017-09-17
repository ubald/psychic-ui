#include <string>
#include <iostream>
#include <cmath>
#include "psychic-ui/utils/StringUtils.hpp"
#include "Label.hpp"

namespace psychic_ui {

    Label::Label(const std::string &text) :
        Div::Div() {
        setTag("Label");

        _defaultStyle
            ->set(shrink, 0)
            ->set(grow, 0);

        setMeasurable();
        setText(text);

        #ifdef DEBUG_LAYOUT
        dashed = true;
        #endif
    }

    const std::string &Label::text() const {
        return _text;
    }

    void Label::setText(const std::string &text) {
        if (text == _text) {
            return;
        }
        _text = text;
        _textBox.setText(_text.c_str(), _text.length(), _textPaint);
        invalidate();
    }

    const bool Label::lcdRender() const {
        return _lcdRender;
    }

    Label *Label::setLcdRender(const bool lcdRender) {
        _lcdRender = lcdRender;
        invalidateStyle();
        return this;
    }

    const bool Label::subpixelText() const {
        return _subpixelText;
    }

    Label *Label::setSubpixelText(const bool subpixelText) {
        _subpixelText = subpixelText;
        invalidateStyle();
        return this;
    }

    void Label::styleUpdated() {
        Div::styleUpdated();

        float size = _computedStyle->get(fontSize);
        _antiAlias  = _computedStyle->get(textAntiAlias);
        _lineHeight = _computedStyle->get(lineHeight);
        if (std::isnan(_lineHeight)) {
            _lineHeight = size * 1.5f;
        }

        _textPaint.setAntiAlias(_antiAlias);
        _textPaint.setLCDRenderText(_antiAlias && _lcdRender);
        _textPaint.setSubpixelText(_antiAlias && _subpixelText);

        _textPaint.setTypeface(styleManager()->font(_computedStyle->get(fontFamily)));
        _textPaint.setTextSize(size);
        _textPaint.setColor(_computedStyle->get(color));

        // TextBox doesn't measure the same way it draws, we have to set the spacing manually
        _textBox.setSpacing(size / _textPaint.getFontSpacing(), _lineHeight - size);

        float mh = _computedStyle->get(minHeight);
        _defaultStyle->set(minHeight, std::isnan(mh) ? _lineHeight : std::max(mh, _lineHeight));
    }

    YGSize Label::measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
        YGSize size = Div::measure(width, widthMode, height, heightMode);

        if (_text.empty()) {
            size.width  = 0;
            size.height = _lineHeight;
            return size;
        }

        if (_computedStyle->get(multiline)) {

            if (widthMode == YGMeasureModeUndefined) {
                // Don't care about setWidth so do longest line
                auto              lines         = string_utils::split(_text, '\n');
                unsigned int      longestIndex  = 0;
                unsigned long     longestLength = 0;
                for (unsigned int i             = 0; i < lines.size(); ++i) {
                    unsigned long len = lines[i].size();
                    if (len > longestLength) {
                        longestIndex  = i;
                        longestLength = len;
                    }
                }
                _textBox.setMode(SkTextBox::kOneLine_Mode);
                // TODO: This doesn't seem to take the right padding into account
                size.width  = std::ceil(_textPaint.measureText(lines[longestIndex].c_str(), longestLength));
                size.height = lines.size() * _lineHeight;
            } else {
                float w = _textPaint.measureText(_text.c_str(), _text.length());
                if (w > width) {
                    _textBox.setMode(SkTextBox::kLineBreak_Mode);
                    // The passed sizes consider padding, which is different than when we draw
                    _textBox.setBox(0, 0, width, height);
                    // size.getHeight = _textBox.getTextHeight();
                    // TextBox doesn't measure the same way it draws, we have to set the spacing manually
                    size.height = _textBox.countLines() * _lineHeight;
                } else {
                    _textBox.setMode(SkTextBox::kOneLine_Mode);
                    size.width  = std::ceil(w);
                    size.height = _lineHeight;//_textPaint.getFontSpacing();
                }
            }

        } else {
            // We're not multiline, so remove line returns
            std::replace(_text.begin(), _text.end(), '\n', ' ');

            _textBox.setMode(SkTextBox::kOneLine_Mode);
            size.width  = std::ceil(_textPaint.measureText(_text.c_str(), _text.size()));
            size.height = _lineHeight;

            if (widthMode == YGMeasureModeUndefined) {
                std::cout << "???" << std::endl;
            } else {
                size.width = std::fmin(size.width, width);
            }
        }

        return size;
    }

    void Label::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        if (!_text.empty()) {
            _textBox.setBox(_paddedRect);
            _textBox.draw(canvas);
        }
    }
}
