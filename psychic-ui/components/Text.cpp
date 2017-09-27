#include <SkRegion.h>
#include "psychic-ui/utils/StringUtils.hpp"
#include "psychic-ui/Window.hpp"
#include "Text.hpp"

namespace psychic_ui {
    Text::Text(const std::string &text) :
        TextBase::TextBase(text) {
        setTag("Text");
        _textBox.setPaint(_textPaint);
        _textBox.setText(_text);

        onMouseDown(
            [this](const int mouseX, const int mouseY, const int button, const int modifiers) {
                int initialBegin = _textBox.indexFromPos(mouseX, mouseY);
                _selectBegin = initialBegin;
                _selectEnd   = _selectBegin;
                _onMouseMove = window()->onMouseMove(
                    [this, initialBegin](const int mouseX, const int mouseY, int button, int modifiers) {
                        int lx = 0;
                        int ly = 0;
                        globalToLocal(lx, ly, mouseX, mouseY);
                        int tmp = _textBox.indexFromPos(lx, ly);
                        if (tmp >= initialBegin) {
                            _selectBegin = initialBegin;
                            _selectEnd   = tmp;
                        } else {
                            _selectBegin = tmp;
                            _selectEnd   = initialBegin;
                        }
                    }
                );
            }
        );

        onMouseUp(
            [this](const int mouseX, const int mouseY, int button, int modifiers) {
                if (_onMouseMove) {
                    _onMouseMove->disconnect();
                    _onMouseMove = nullptr;
                }
            }
        );

        onBlur(
            [this]() {
                _selectBegin = -1;
                _selectEnd   = -1;
            }
        );
    }

    void Text::setText(const std::string &text) {
        if (_text != text) {
            TextBase::setText(text);
            _textBox.setText(_text);
        }
    }

    const bool Text::selectable() const {
        return _selectable;
    }

    Text *Text::setSelectable(const bool selectable) {
        if (_selectable != selectable) {
            _selectable = selectable;
        }
        return this;
    }

    void Text::styleUpdated() {
        TextBase::styleUpdated();

        _textBox.setSpacing(_fontSize / _textPaint.getFontSpacing(), _lineHeight - _fontSize);

        _selectionPaint.setStyle(SkPaint::kFill_Style);
        _selectionPaint.setColor(_computedStyle->get(selectionColor));

        _selectionBackgroundPaint = _textPaint;
        _selectionBackgroundPaint.setColor(_computedStyle->get(selectionBackgroundColor));
    }

    YGSize Text::measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
        YGSize size{0.0f, _lineHeight};
        if (_text.empty()) {
            return size;
        }

        if (widthMode == YGMeasureModeUndefined) {
            // Don't care about setWidth so measure the longest line
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
            _textBox.setMode(TextBoxMode::OneLine);
            // TODO: This doesn't seem to take the right padding into account
            size.width  = std::ceil(_textPaint.measureText(lines[longestIndex].c_str(), longestLength));
            size.height = lines.size() * _lineHeight;
        } else {
            float w = _textPaint.measureText(_text.c_str(), _text.length());
            if (w > width) {
                _textBox.setMode(TextBoxMode::LineBreak);
                // The passed sizes consider padding, which is different than when we draw
                _textBox.setBox(0, 0, width, height);
                // size.getHeight = _textBox.getTextHeight();
                // TextBox doesn't measure the same way it draws, we have to set the spacing manually
                size.height = _textBox.countLines() * _lineHeight;
            } else {
                _textBox.setMode(TextBoxMode::OneLine);
                size.width  = std::ceil(w);
                size.height = _lineHeight;//_textPaint.getFontSpacing();
            }
        }

        return size;
    }

    void Text::layoutUpdated() {
        TextBase::layoutUpdated();
        _textBox.setBox(0.0f, 0.0f, _width, _height);
        _blob = _textBox.snapshotTextBlob();
    }

    void Text::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        if (!_text.empty() && _blob) {
            canvas->drawTextBlob(_blob.get(), _paddedRect.fLeft, _paddedRect.fTop, _textPaint);

            SkPath path{};
            if (_selectBegin != _selectEnd) {
                auto begin = _textBox.posFromIndex(_selectBegin);
                auto end   = _textBox.posFromIndex(_selectEnd);

                for (int i = begin.first; i <= end.first; ++i) {
                    path.addRect(
                        SkRect{
                            static_cast<SkScalar>(_paddedRect.fLeft + i == begin.first ? begin.second : 0),
                            static_cast<SkScalar>(_paddedRect.fTop + i * _lineHeight),
                            static_cast<SkScalar>(_paddedRect.fLeft + i == end.first ? end.second : _width),
                            static_cast<SkScalar>(_paddedRect.fTop + (i + 1) * _lineHeight),
                        }
                    );
                }

                canvas->drawPath(path, _selectionBackgroundPaint);
            }

            // TODO: Doing like this is probably not performant at all, it all depends on how ski draws clipped text
            if (_selectBegin != _selectEnd) {
                canvas->clipPath(path);
                canvas->drawTextBlob(_blob.get(), _paddedRect.fLeft, _paddedRect.fTop, _selectionPaint);
            }
        }
    }
}
