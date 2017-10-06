#include <SkRegion.h>
#include "psychic-ui/utils/StringUtils.hpp"
#include "psychic-ui/Window.hpp"
#include "Text.hpp"

namespace psychic_ui {
    Text::Text(const std::string &text) :
        TextBase::TextBase() {
        setTag("Text");

        _textBox.setPaint(_textPaint);
        _textBox.setMode(_multiline ? TextBoxMode::LineBreak : TextBoxMode::OneLine);

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

    Text *Text::setText(const std::string &text) {
        _text = UnicodeString::fromUTF8(text);
        _textBox.setText(_text);
        _caret       = 0;
        _selectBegin = 0;
        _selectEnd   = 0;
        return this;
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
            _textBox.setMode(_multiline ? TextBoxMode::LineBreak : TextBoxMode::OneLine);
        }
        return this;
    }

    std::pair<unsigned int, unsigned int> Text::getSelection() const {
        return std::make_pair(_selectBegin, _selectEnd);
    };

    Text *Text::setSelection(unsigned int selectionBegin, unsigned int selectionEnd) {
        _selectBegin = std::min(selectionBegin, static_cast<unsigned int>(_text.length()));
        _selectEnd   = std::min(selectionEnd, static_cast<unsigned int>(_text.length()));
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

    Text *Text::setCaret(unsigned int caret, bool saveX) {
        _caret       = std::min(caret, static_cast<unsigned int>(_text.length()));
        _selectBegin = _caret;
        _selectEnd   = _caret;
        if (saveX) {
            _targetXPos = _textBox.posFromIndex(_caret).second;
        }
        onCaret(_caret);
        return this;
    }

    unsigned int Text::getCaretLine() const {
        return _textBox.lineFromIndex(_caret);
    }

    unsigned int Text::getLineY(unsigned int line) const {
        return static_cast<unsigned int>(std::ceil(line * _lineHeight));
    }

    void Text::subscribeToSelection() {
        _onMouseDown = onMouseDown(
            [this](const int mouseX, const int mouseY, const int /*button*/, const Mod /*modifiers*/) {
                unsigned int initialBegin = _textBox.indexFromPos(mouseX, mouseY);
                setCaret(initialBegin);
                _onMouseMove = window()->onMouseMove(
                    [this, initialBegin](const int mouseX, const int mouseY, int /*button*/, Mod /*modifiers*/) {
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
            [this](const int /*mouseX*/, const int /*mouseY*/, int /*button*/, Mod /*modifiers*/) {
                if (_onMouseMove) {
                    _onMouseMove->disconnect();
                    _onMouseMove = nullptr;
                }
            }
        );

        _onDoubleClick = onDoubleClick(
            [this](const unsigned int clickCount) {
                if (clickCount == 2) {
                    auto word = _textBox.wordAtIndex(_caret);
                    _selectBegin = word.first;
                    _selectEnd   = word.second;
                } else if (clickCount == 3) {
                    auto sentence = _textBox.sentenceAtIndex(_caret);
                    _selectBegin = sentence.first;
                    _selectEnd   = sentence.second;
                } else { // if (clickCount == 4) {
                    _selectBegin = 0;
                    _selectEnd   = static_cast<unsigned int>(_text.length());
                }
                onSelection(_selectBegin, _selectEnd);
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
        if (_onDoubleClick) {
            _onDoubleClick->disconnect();
            _onDoubleClick = nullptr;
        }
    }

    void Text::subscribeToEdition() {
        _onFocus     = onFocus([this]() { window()->startTextInput(); });
        _onBlur      = onBlur([this]() { window()->stopTextInput(); });
        _onKeyDown   = onKeyDown([this](const Key key, const Mod mod) { handleKey(key, mod); });
        _onKeyRepeat = onKeyRepeat([this](const Key key, const Mod mod) { handleKey(key, mod); });
        _onCharacter = onCharacter(
            [this](UnicodeString character) {
                if (_selectBegin != _selectEnd) {
                    _text.replace(_selectBegin, _selectEnd - _selectBegin, character);
                    setCaret(_selectBegin + 1);
                } else {
                    _text.insert(_caret, character);
                    setCaret(++_caret);
                }
                updateDisplay();
            }
        );
    }

    void Text::unsubscribeFromEdition() {
        if (_onFocus) {
            _onFocus->disconnect();
            _onFocus = nullptr;
        }
        if (_onBlur) {
            _onBlur->disconnect();
            _onBlur = nullptr;
        }
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
        _textBox.recalculate();
        invalidate();
    }

    void Text::handleKey(Key key, Mod mod) {
        switch (key) {
            case Key::A:
                if (mod.ctrl or mod.super) {
                    _selectBegin = 0;
                    _selectEnd   = static_cast<unsigned int>(_text.length());
                    onSelection(_selectBegin, _selectEnd);
                }
                break;

            case Key::HOME:
                if (mod.ctrl) {
                    setCaret(0);
                } else {
                    setCaret(_textBox.lineStart(_textBox.lineFromIndex(_caret)));
                }
                break;

            case Key::END:
                if (mod.ctrl) {
                    setCaret(static_cast<unsigned int>(_text.length()));
                } else {
                    setCaret(_textBox.lineEnd(_textBox.lineFromIndex(_caret)));
                }
                break;

            case Key::UP: {
                auto pos = _textBox.posFromIndex(_caret);
                if (pos.first == 0) {
                    // On the first line, so reach beginning of line
                    setCaret(0);
                } else {
                    // Otherwise go one line up
                    auto index = _textBox.indexFromPos(_targetXPos, static_cast<unsigned int>((pos.first - 1) * _lineHeight));
                    setCaret(index, false);
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
                    auto index = _textBox.indexFromPos(_targetXPos, static_cast<unsigned int>((pos.first + 1) * _lineHeight));
                    setCaret(index, false);
                }
                break;
            }

            case Key::LEFT:
                if (_caret > 0) {
                    if (mod.ctrl) {
                        setCaret(_textBox.previousWordBoundary(_caret));
                    } else {
                        setCaret(--_caret);
                    }
                }
                break;

            case Key::RIGHT:
                if (_caret < _text.length()) {
                    if (mod.ctrl) {
                        setCaret(_textBox.nextWordBoundary(_caret));
                    } else {
                        setCaret(++_caret);
                    }
                }
                break;

            case Key::BACKSPACE:
                if (_caret > 0) {
                    if (_selectBegin != _selectEnd) {
                        // Remove selection
                        _text.remove(_selectBegin, _selectEnd - _selectBegin);
                        setCaret(_selectBegin);
                    } else if (mod.ctrl) {
                        // Remove preceding word
                        auto from = _textBox.previousWordBoundary(_caret);
                        _text.removeBetween(from, _caret);
                        setCaret(from);
                    } else {
                        // Remove preceding character
                        _text.remove(_caret - 1, 1);
                        setCaret(--_caret);
                    }
                    updateDisplay();
                }
                break;

            case Key::DELETE:
                if (_caret < _text.length()) {
                    if (_selectBegin != _selectEnd) {
                        // Delete selection
                        _text.remove(_selectBegin, _selectEnd - _selectBegin);
                        setCaret(_selectBegin);
                    } else if (mod.ctrl) {
                        // Delete following word
                        _text.removeBetween(_caret, _textBox.nextWordBoundary(_caret));
                    } else {
                        // Delete following character
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

        if (widthMode == YGMeasureModeUndefined) {
            // Don't care about setWidth so measure the longest line
            std::string str;
            _text.toUTF8String(str);
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
            //_textBox.setMode(TextBoxMode::OneLine);
            // TODO: This doesn't seem to take the right padding into account
            size.width  = std::ceil(_textPaint.measureText(lines[longestIndex].c_str(), longestLength));
            size.height = lines.size() * _lineHeight;
        } else {
            //_textBox.setMode(TextBoxMode::LineBreak);
            // The passed sizes consider padding, which is different than when we draw
            _textBox.setBox(0, 0, width, height);
            size.height = _textBox.countLines() * _lineHeight;
        }

        return size;
    }

    void Text::layoutUpdated() {
        TextBase::layoutUpdated();
        _textBox.setBox(0.0f, 0.0f, _paddedRect.width(), _paddedRect.height());
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
