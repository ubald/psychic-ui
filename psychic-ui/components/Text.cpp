#include <cmath>
#include "include/core/SkPathBuilder.h"
#include "psychic-ui/Window.hpp"
#include "Text.hpp"

namespace psychic_ui {
    Text::Text(const std::string &text) :
        TextBase::TextBase() {
        tag("Text");

        _textLayout.setMultiline(_multiline);

        Text::updateText(text);

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

    void Text::updateText(const std::string &text) {
        _text = icu::UnicodeString::fromUTF8(text);
        _textLayout.setText(_text);
        _caret       = 0;
        _selectBegin = 0;
        _selectEnd   = 0;
        invalidate();
    }

    bool Text::selectable() const {
        return _selectable;
    }

    void Text::updateSelectable(const bool selectable) {
        if (_selectable != selectable) {
            _selectionSlots.clear();
            _selectable = selectable;
            if (_selectable) {
                subscribeToSelection();
            }
        }
    }

    bool Text::editable() const {
        return _editable;
    }

    void Text::updateEditable(const bool editable) {
        if (_editable != editable) {
            _editionSlots.clear();
            _editable = editable;
            if (_editable) {
                subscribeToEdition();
            }
        }
    }

    bool Text::multiline() const {
        return _multiline;
    }

    void Text::updateMultiline(const bool multiline) {
        if (_multiline != multiline) {
            _multiline = multiline;
            _textLayout.setMultiline(_multiline);
            invalidate();
        }
    }

    std::pair<unsigned int, unsigned int> Text::selection() const {
        return std::make_pair(_selectBegin, _selectEnd);
    };

    void Text::updateSelection(unsigned int selectionBegin, unsigned int selectionEnd) {
        _selectBegin = std::min(selectionBegin, static_cast<unsigned int>(_text.length()));
        _selectEnd   = std::min(selectionEnd, static_cast<unsigned int>(_text.length()));
        if (_selectBegin > _selectEnd) {
            std::swap(_selectBegin, _selectEnd);
        }

        caretMoved.emit(_selectEnd);
        selectionChanged.emit(_selectBegin, _selectEnd);

    }

    unsigned int Text::caret() const {
        return _caret;
    };

    void Text::moveCaret(unsigned int caret, bool saveX) {
        _caret       = std::min(caret, static_cast<unsigned int>(_text.length()));
        _selectBegin = _caret;
        _selectEnd   = _caret;
        if (saveX) {
            _targetXPos = _textLayout.posFromIndex(_caret).second;
        }
        if (isValid()) {
            caretMoved.emit(_caret);
        } else {
            // Wait until the layout is valid to notify,
            // otherwise the caret position won't be correct.
            _pendingCaretSignal = true;
        }
    }

    unsigned int Text::getCaretLine() const {
        return _textLayout.lineFromIndex(_caret);
    }

    unsigned int Text::getLineY(unsigned int line) const {
        return static_cast<unsigned int>(std::ceil(line * _lineHeight));
    }

    void Text::subscribeToSelection() {
        _selectionSlots.emplace_back(mousePressed.subscribe(
            [this](const MouseEvent &event) {
                unsigned int initialBegin = indexAt(static_cast<int>(event.x), static_cast<int>(event.y));
                moveCaret(initialBegin);
                _onMouseMove = window()->mouseMoved.subscribe(
                    [this, initialBegin](const MouseEvent &move) {
                        int lx = 0;
                        int ly = 0;
                        globalToLocal(lx, ly, static_cast<int>(move.windowX), static_cast<int>(move.windowY));
                        _caret = indexAt(lx, ly);
                        if (_caret >= initialBegin) {
                            _selectBegin = initialBegin;
                            _selectEnd   = _caret;
                        } else {
                            _selectBegin = _caret;
                            _selectEnd   = initialBegin;
                        }
                        caretMoved.emit(_caret);
                        selectionChanged.emit(_selectBegin, _selectEnd);
                    }
                );
            }
        ));

        _selectionSlots.emplace_back(mouseReleased.subscribe(
            [this] {
                _onMouseMove.disconnect();
            }
        ));

        _selectionSlots.emplace_back(doubleClicked.subscribe(
            [this](const MouseEvent &event) {
                const unsigned clickCount = event.clickCount;
                if (clickCount == 2) {
                    auto word = _textLayout.wordAtIndex(_caret);
                    _selectBegin = word.first;
                    _selectEnd   = word.second;
                } else if (clickCount == 3) {
                    auto sentence = _textLayout.sentenceAtIndex(_caret);
                    _selectBegin = sentence.first;
                    _selectEnd   = sentence.second;
                } else { // if (clickCount == 4) {
                    _selectBegin = 0;
                    _selectEnd   = static_cast<unsigned int>(_text.length());
                }
                selectionChanged.emit(_selectBegin, _selectEnd);
            }
        ));
    }

    void Text::subscribeToEdition() {
        _editionSlots.emplace_back(focused.subscribe([this]() {
            window()->startTextInput();
            updateTextInputArea();
        }));
        _editionSlots.emplace_back(caretMoved.subscribe([this](unsigned int /*caret*/) { updateTextInputArea(); }));
        _editionSlots.emplace_back(blurred.subscribe([this]() { window()->stopTextInput(); }));
        _editionSlots.emplace_back(keyPressed.subscribe([this](const Key key, const Mod mod) { handleKey(key, mod); }));
        _editionSlots.emplace_back(keyRepeated.subscribe([this](const Key key, const Mod mod) { handleKey(key, mod); }));
        _editionSlots.emplace_back(typed.subscribe(
            [this](icu::UnicodeString character) {
                if (_selectBegin != _selectEnd) {
                    _text.replace(_selectBegin, _selectEnd - _selectBegin, character);
                    textEdited(_selectBegin + 1);
                } else {
                    _text.insert(_caret, character);
                    textEdited(++_caret);
                }
            }
        ));
    }

    void Text::removedFromRender() {
        TextBase::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-selection: the mouse up won't reach us
    }

    void Text::textChanged() {
        _textLayout.setText(_text);
        invalidate();
        changed.emit(text());
    }

    void Text::textEdited(unsigned int caret) {
        textChanged(); // Before setCaret so that `onCaret` has access to computed lines
        moveCaret(caret);
    }

    unsigned int Text::indexAt(int x, int y) const {
        // The text starts inside the padding and border
        return _textLayout.indexFromPos(
            static_cast<float>(x) - (_paddedRect.fLeft - _rect.fLeft),
            static_cast<float>(y) - (_paddedRect.fTop - _rect.fTop)
        );
    }

    void Text::handleKey(Key key, Mod mod) {
        switch (key) {
            case Key::A:
                if (mod.ctrl or mod.super) {
                    _selectBegin = 0;
                    _selectEnd   = static_cast<unsigned int>(_text.length());
                    selectionChanged.emit(_selectBegin, _selectEnd);
                }
                break;

            case Key::C:
            case Key::X:
                if ((mod.ctrl or mod.super) && _selectBegin != _selectEnd) {
                    std::string selected;
                    _text.tempSubString(_selectBegin, _selectEnd - _selectBegin).toUTF8String(selected);
                    window()->setClipboardText(selected);
                    if (key == Key::X) {
                        _text.remove(_selectBegin, _selectEnd - _selectBegin);
                        textEdited(_selectBegin);
                    }
                }
                break;

            case Key::V:
                if (mod.ctrl or mod.super) {
                    auto pasted = icu::UnicodeString::fromUTF8(window()->clipboardText());
                    if (!_multiline) {
                        pasted.findAndReplace(u"\r\n", u" ").findAndReplace(u"\n", u" ").findAndReplace(u"\r", u" ");
                    }
                    if (pasted.isEmpty()) {
                        break;
                    }
                    if (_selectBegin != _selectEnd) {
                        _text.replace(_selectBegin, _selectEnd - _selectBegin, pasted);
                        textEdited(_selectBegin + pasted.length());
                    } else {
                        _text.insert(_caret, pasted);
                        textEdited(_caret + pasted.length());
                    }
                }
                break;

            case Key::HOME:
                if (mod.ctrl) {
                    moveCaret(0);
                } else {
                    moveCaret(_textLayout.lineStart(_textLayout.lineFromIndex(_caret)));
                }
                break;

            case Key::END:
                if (mod.ctrl) {
                    moveCaret(static_cast<unsigned int>(_text.length()));
                } else {
                    moveCaret(_textLayout.lineEnd(_textLayout.lineFromIndex(_caret)));
                }
                break;

            case Key::UP: {
                auto pos = _textLayout.posFromIndex(_caret);
                if (pos.first == 0) {
                    // On the first line, so reach beginning of line
                    moveCaret(0);
                } else {
                    // Otherwise go one line up
                    auto index = _textLayout.indexFromPos(_targetXPos, (static_cast<float>(pos.first) - 0.5f) * _lineHeight);
                    moveCaret(index, false);
                }
                break;
            }

            case Key::DOWN: {
                auto pos = _textLayout.posFromIndex(_caret);
                if (pos.first == _textLayout.lineCount() - 1) {
                    // On the last line, so reach end of line
                    moveCaret(static_cast<unsigned int>(_text.length()));
                } else {
                    // Otherwise go one line down
                    auto index = _textLayout.indexFromPos(_targetXPos, (static_cast<float>(pos.first) + 1.5f) * _lineHeight);
                    moveCaret(index, false);
                }
                break;
            }

            case Key::LEFT:
                if (_caret > 0) {
                    if (mod.ctrl) {
                        moveCaret(_textLayout.previousWordBoundary(_caret));
                    } else {
                        moveCaret(--_caret);
                    }
                }
                break;

            case Key::RIGHT:
                if (_caret < static_cast<unsigned int>(_text.length())) {
                    if (mod.ctrl) {
                        moveCaret(_textLayout.nextWordBoundary(_caret));
                    } else {
                        moveCaret(++_caret);
                    }
                }
                break;

            case Key::BACKSPACE:
                if (_caret > 0) {
                    if (_selectBegin != _selectEnd) {
                        // Remove selection
                        _text.remove(_selectBegin, _selectEnd - _selectBegin);
                        textEdited(_selectBegin);
                    } else if (mod.ctrl) {
                        // Remove preceding word
                        auto from = _textLayout.previousWordBoundary(_caret);
                        _text.removeBetween(from, _caret);
                        textEdited(from);
                    } else {
                        // Remove preceding character
                        _text.remove(_caret - 1, 1);
                        textEdited(--_caret);
                    }
                }
                break;

            case Key::DELETE:
                if (_caret < static_cast<unsigned int>(_text.length())) {
                    if (_selectBegin != _selectEnd) {
                        // Delete selection
                        _text.remove(_selectBegin, _selectEnd - _selectBegin);
                        textEdited(_selectBegin);
                    } else if (mod.ctrl) {
                        // Delete following word
                        _text.removeBetween(_caret, _textLayout.nextWordBoundary(_caret));
                        textChanged();
                    } else {
                        // Delete following character
                        _text.remove(_caret, 1);
                        textChanged();
                    }
                }
                break;

            case Key::ENTER:
                if (_multiline) {
                    icu::UnicodeString uni_str(static_cast<UChar32>('\n'));
                    if (_selectBegin != _selectEnd) {
                        _text.replace(_selectBegin, _selectEnd - _selectBegin, uni_str);
                        textEdited(_selectBegin + 1);
                    } else {
                        _text.insert(_caret, uni_str);
                        textEdited(++_caret);
                    }
                }
                break;

            default:
                break;
        }
    }

    void Text::styleUpdated() {
        TextBase::styleUpdated();

        _selectionBackgroundPaint = _textPaint;
        _selectionBackgroundPaint.setColor(_computedStyle.selectionBackgroundColor().argb());
    }

    Size Text::measure(Constraint width, Constraint height) {
        // Pure: the layout caches measures and calls this with various constraints,
        // so the layout used for painting (at the box's width) is left as it is.
        float contentWidth = width.apply(std::ceil(_textLayout.maxContentWidth()));
        auto  lines        = std::max<size_t>(1, _textLayout.lineCountAt(contentWidth));
        return Size{contentWidth, height.apply(linesHeight(lines))};
    }

    void Text::layoutUpdated() {
        TextBase::layoutUpdated();
        _textLayout.layout(_paddedRect.width());
        if (_pendingCaretSignal) {
            caretMoved.emit(_caret);
            _pendingCaretSignal = false;
        }
    }

    void Text::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        if (!_text.isEmpty()) {
            _textLayout.paint(canvas, _paddedRect.fLeft, _paddedRect.fTop);

            if (_selectable && _focused && _selectBegin != _selectEnd) {
                auto begin = _textLayout.posFromIndex(_selectBegin);
                auto end   = _textLayout.posFromIndex(_selectEnd);

                SkPathBuilder builder{};
                for (unsigned int i = begin.first; i <= end.first; ++i) {
                    builder.addRect(
                        SkRect{
                            _paddedRect.fLeft + (i == begin.first ? begin.second : 0.0f),
                            _paddedRect.fTop + (i * _lineHeight),
                            _paddedRect.fLeft + (i == end.first ? end.second : _width),
                            _paddedRect.fTop + ((i + 1.0f) * _lineHeight),
                        }
                    );
                }
                SkPath path = builder.detach();

                canvas->drawPath(path, _selectionBackgroundPaint);
                canvas->save();
                canvas->clipPath(path);
                _textLayout.paintSelected(canvas, _paddedRect.fLeft, _paddedRect.fTop);
                canvas->restore();
            }
        }

        if (_editable && _focused) {
            auto   caret = _textLayout.posFromIndex(_caret);
            SkRect caretRect{
                _paddedRect.fLeft + caret.second - 1.0f,
                _paddedRect.fTop + (caret.first * _lineHeight),
                _paddedRect.fLeft + caret.second + 1.0f,
                _paddedRect.fTop + ((caret.first + 1.0f) * _lineHeight),
            };
            canvas->drawRect(caretRect, _textPaint);
        }
    }

    void Text::updateTextInputArea() {
        Window *window = this->window();
        if (!window || !_focused || !_editable || window->inputMode() != InputMode::Touch) {
            return;
        }
        // The caret's line, across the text, in window coordinates
        const auto caret = _textLayout.posFromIndex(_caret);
        int        x     = 0;
        int        y     = 0;
        localToGlobal(
            x, y,
            static_cast<int>(_paddedRect.fLeft) - _x,
            static_cast<int>(_paddedRect.fTop + static_cast<float>(caret.first) * _lineHeight) - _y
        );
        window->setTextInputArea(
            x, y,
            static_cast<int>(std::ceil(_paddedRect.width())),
            static_cast<int>(std::ceil(_lineHeight)),
            static_cast<int>(caret.second)
        );
    }
}

