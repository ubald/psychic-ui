#include <SkRegion.h>
#include "psychic-ui/utils/StringUtils.hpp"
#include "psychic-ui/Window.hpp"
#include "Text.hpp"

namespace psychic_ui {
    Text::Text(const std::string &text) :
        TextBase::TextBase(text) {
        setTag("Text");

        _textBox.setPaint(_textPaint);
        setText(text);

        if (_selectable) {
            subscribeToSelection();
        }

        if (_editable) {
            subscribeToSelection();
        }
    }

    std::string Text::text() const {
        std::string str{};
        _text.toUTF8String(str);
        return str;
    }

    void Text::setText(const std::string &text) {
        _text = UnicodeString::fromUTF8(text);
        _textBox.setText(_text);
        _caret       = 0;
        _selectBegin = 0;
        _selectEnd   = 0;
    }

    bool Text::selectable() const {
        return _selectable;
    }

    Text *Text::setSelectable(const bool selectable) {
        if (_selectable != selectable) {
            if (_selectable) {
                unsubscribeFromSelection();
            }
            _selectable = selectable;
            if (_selectable) {
                subscribeToSelection();
            }
        }
        return this;
    }

    bool Text::editable() const {
        return _editable;
    }

    Text *Text::setEditable(const bool editable) {
        if (_editable != editable) {
            if (_editable) {
                unsubscribeFromEdition();
            }
            _editable = editable;
            if (_editable) {
                subscribeToEdition();
            }
        }
        return this;
    }

    bool Text::multiline() const {
        return _multiline;
    }

    Text *Text::setMultiline(const bool multiline) {
        if (_multiline != multiline) {
            _multiline = multiline;
        }
        return this;
    }
    
    std::pair<unsigned int, unsigned int> Text::getSelection() const {
        return std::make_pair(_selectBegin, _selectEnd);
    };

    Text *Text::setSelection(std::pair<unsigned int, unsigned int> selection) {
        _selectBegin = std::min(selection.first, static_cast<unsigned int>(_text.length()));
        _selectEnd   = std::min(selection.second, static_cast<unsigned int>(_text.length()));
        if (_selectBegin > _selectEnd) {
            std::swap(_selectBegin, _selectEnd);
        }

        onCaret(_selectEnd);
        onSelection(_selectBegin, _selectEnd);

        return this;
    }

    unsigned int Text::getCaret() const {
        return _caret;
    };

    Text *Text::setCaret(unsigned int caret) {
        _caret       = std::min(caret, static_cast<unsigned int>(_text.length()));
        _selectBegin = _caret;
        _selectEnd   = _caret;
        onCaret(_caret);
        return this;
    }

    void Text::subscribeToSelection() {
        _onMouseDown = onMouseDown(
            [this](const int mouseX, const int mouseY, const int button, const int modifiers) {
                unsigned int initialBegin = _textBox.indexFromPos(mouseX, mouseY);
                setCaret(initialBegin);
                _onMouseMove = window()->onMouseMove(
                    [this, initialBegin](const int mouseX, const int mouseY, int button, int modifiers) {
                        int lx = 0;
                        int ly = 0;
                        globalToLocal(lx, ly, mouseX, mouseY);
                        _caret = _textBox.indexFromPos(lx, ly);
                        if (_caret >= initialBegin) {
                            _selectBegin = initialBegin;
                            _selectEnd   = _caret;
                        } else {
                            _selectBegin = _caret;
                            _selectEnd   = initialBegin;
                        }
                        onCaret(_caret);
                        onSelection(_selectBegin, _selectEnd);
                    }
                );
            }
        );

        _onMouseUp = onMouseUp(
            [this](const int mouseX, const int mouseY, int button, int modifiers) {
                if (_onMouseMove) {
                    _onMouseMove->disconnect();
                    _onMouseMove = nullptr;
                }
            }
        );
    }

    void Text::unsubscribeFromSelection() {
        if (_onMouseDown) {
            _onMouseDown->disconnect();
            _onMouseDown = nullptr;
        }
        if (_onMouseUp) {
            _onMouseUp->disconnect();
            _onMouseUp = nullptr;
        }
    }

    void Text::subscribeToEdition() {
        _onKeyDown   = onKeyDown([this](Key key) { handleKey(key); });
        _onKeyRepeat = onKeyRepeat([this](Key key) { handleKey(key); });
        _onCharacter = onCharacter(
            [this](unsigned int codepoint) {
                UnicodeString uni_str(static_cast<UChar32>(codepoint));
                if (_selectBegin != _selectEnd) {
                    _text.replace(_selectBegin, _selectEnd - _selectBegin, uni_str);
                    setCaret(_selectBegin + 1);
                } else {
                    _text.insert(_caret, uni_str);
                    setCaret(++_caret);
                }
                updateDisplay();
            }
        );
    }

    void Text::unsubscribeFromEdition() {
        if (_onKeyDown) {
            _onKeyDown->disconnect();
            _onKeyDown = nullptr;
        }
        if (_onKeyRepeat) {
            _onKeyRepeat->disconnect();
            _onKeyRepeat = nullptr;
        }
        if (_onCharacter) {
            _onCharacter->disconnect();
            _onCharacter = nullptr;
        }
    }

    void Text::updateDisplay() {
        // TODO: This should be optimized as it copies the text into the textbox
        _textBox.recalculate();
        invalidate();
    }

    void Text::handleKey(Key key) {
        switch (key) {
            case Key::HOME:
                // TODO: ctrl+home goes to start of doc, home goes to end of line
                setCaret(0);
                break;

            case Key::END:
                // TODO: ctrl+end goes to end of doc, end goes to end of line
                setCaret(static_cast<unsigned int>(_text.length()));
                break;

            case Key::UP: {
                auto pos = _textBox.posFromIndex(_caret);
                if (pos.first == 0) {
                    // On the first line, so reach beginning of line
                    setCaret(0);
                } else {
                    // Otherwise go one line up
                    auto index = _textBox.indexFromPos(pos.second, static_cast<unsigned int>((pos.first - 1) * _lineHeight));
                    setCaret(index);
                }
                break;
            }

            case Key::DOWN: {
                auto pos = _textBox.posFromIndex(_caret);
                if (pos.first == _textBox.lineCount() - 1) {
                    // On the last line, so reach end of line
                    setCaret(static_cast<unsigned int>(_text.length()));
                } else {
                    // Otherwise go one line down
                    auto index = _textBox.indexFromPos(pos.second, static_cast<unsigned int>((pos.first + 1) * _lineHeight));
                    setCaret(index);
                }
                break;
            }

            case Key::LEFT:
                if (_caret > 0) {
                    setCaret(--_caret);
                }
                break;

            case Key::RIGHT:
                if (_caret < _text.length()) {
                    setCaret(++_caret);
                }
                break;

            case Key::BACKSPACE:
                if (_caret > 0) {
                    if (_selectBegin != _selectEnd) {
                        _text.remove(_selectBegin, _selectEnd - _selectBegin);
                        setCaret(_selectBegin);
                    } else {
                        _text.remove(_caret - 1, 1);
                        setCaret(--_caret);
                    }
                    updateDisplay();
                }
                break;

            case Key::DELETE:
                if (_caret < _text.length()) {
                    if (_selectBegin != _selectEnd) {
                        _text.remove(_selectBegin, _selectEnd - _selectBegin);
                        setCaret(_selectBegin);
                    } else {
                        _text.remove(_caret, 1);
                    }
                    updateDisplay();
                }
                break;

            case Key::ENTER:
                if (_multiline) {
                    UnicodeString uni_str(static_cast<UChar32>('\n'));
                    if (_selectBegin != _selectEnd) {
                        _text.replace(_selectBegin, _selectEnd - _selectBegin, uni_str);
                        setCaret(_selectBegin + 1);
                    } else {
                        _text.insert(_caret, uni_str);
                        setCaret(++_caret);
                    }
                    updateDisplay();
                }
                break;

            default:
                break;
        }
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
        if (_text.isEmpty()) {
            return size;
        }

        // TODO: If we only measure using the text box we can probably skip this conversion
        std::string str;
        _text.toUTF8String(str);

        if (widthMode == YGMeasureModeUndefined) {
            // Don't care about setWidth so measure the longest line
            auto              lines         = string_utils::split(str, '\n');
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
            float w = _textPaint.measureText(str.c_str(), str.size());
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
        if (!_text.isEmpty() && _blob) {
            canvas->drawTextBlob(_blob.get(), _paddedRect.fLeft, _paddedRect.fTop, _textPaint);

            if (_selectable && _focused && _selectBegin != _selectEnd) {
                auto begin = _textBox.posFromIndex(_selectBegin);
                auto end   = _textBox.posFromIndex(_selectEnd);

                SkPath   path{};
                for (int i = begin.first; i <= end.first; ++i) {
                    path.addRect(
                        SkRect{
                            _paddedRect.fLeft + (i == begin.first ? begin.second : 0.0f),
                            _paddedRect.fTop + (i * _lineHeight),
                            _paddedRect.fLeft + (i == end.first ? end.second : _width),
                            _paddedRect.fTop + ((i + 1.0f) * _lineHeight),
                        }
                    );
                }

                canvas->drawPath(path, _selectionBackgroundPaint);
                // TODO: Doing like this is probably not performant at all, it all depends on how skia draws clipped text blobs
                canvas->save();
                canvas->clipPath(path);
                canvas->drawTextBlob(_blob.get(), _paddedRect.fLeft, _paddedRect.fTop, _selectionPaint);
                canvas->restore();
            }
        }

        if (_editable && _focused) {
            auto   caret = _textBox.posFromIndex(_caret);
            SkRect caretRect{
                _paddedRect.fLeft + caret.second - 1.0f,
                _paddedRect.fTop + (caret.first * _lineHeight),
                _paddedRect.fLeft + caret.second + 1.0f,
                _paddedRect.fTop + ((caret.first + 1.0f) * _lineHeight),
            };
            canvas->drawRect(caretRect, _textPaint);
        }
    }
}
