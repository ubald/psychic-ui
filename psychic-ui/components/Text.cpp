#include <algorithm>
#include <cmath>
#include <limits>
#include "psychic-ui/Window.hpp"
#include "psychic-ui/text/WindowClipboard.hpp"
#include "Text.hpp"

namespace psychic_ui {

    Text::Text(const std::string &text) :
        Div::Div() {
        tag("Text");

        _defaultStyle.shrink(0);
        setMeasurable();
        _textLayout.setMultiline(false);

        #ifdef DEBUG_LAYOUT
        dashed = true;
        #endif

        Text::updateText(text);

        if (_selectable) {
            subscribeToSelection();
        }
    }

    std::string Text::text() const {
        return _editing.utf8();
    }

    void Text::updateText(const std::string &text) {
        // The caret goes back to the start, the text is only laid out again when it changed
        if (has(_editing.setText(icu::UnicodeString::fromUTF8(text)), TextEditing::Edit::Text)) {
            _textLayout.setText(_editing.text());
            invalidate();
        }
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
        return _editing.multiline();
    }

    void Text::updateMultiline(const bool multiline) {
        if (_editing.multiline() != multiline) {
            _editing.setMultiline(multiline);
            _textLayout.setMultiline(multiline);
            invalidate();
        }
    }

    bool Text::caretVisible() const {
        return _caretVisible;
    }

    void Text::updateCaretVisible(const bool visible) {
        _caretVisible = visible;
    }

    float Text::offsetX() const {
        return _offsetX;
    }

    void Text::reveal(const unsigned int index) {
        if (_editing.multiline()) {
            _offsetX = 0.0f;
            return;
        }
        const float width  = _paddedRect.width();
        const float x      = _textLayout.caretRect(index).fLeft;
        float       offset = _offsetX;
        if (x - offset > width - 1.0f) {
            offset = x - width + 1.0f; // The caret's bar is 2 px wide, centered on its x
        }
        if (x - offset < 1.0f) {
            offset = x - 1.0f;
        }
        _offsetX = std::clamp(offset, 0.0f, std::max(0.0f, std::ceil(_textLayout.maxContentWidth()) + 1.0f - width));
    }

    std::pair<unsigned int, unsigned int> Text::selection() const {
        return _editing.selection();
    };

    void Text::updateSelection(unsigned int anchor, unsigned int caret) {
        apply(_editing.select(anchor, caret));
    }

    unsigned int Text::caret() const {
        return _editing.caret();
    };

    void Text::moveCaret(unsigned int caret) {
        apply(_editing.select(caret, caret));
    }

    void Text::apply(TextEditing::Edit edit) {
        if (has(edit, TextEditing::Edit::Text)) {
            _textLayout.setText(_editing.text());
            invalidate();
            changed.emit(text());
        }
        if (has(edit, TextEditing::Edit::Caret)) {
            if (isValid()) {
                caretMoved.emit(_editing.caret());
            } else {
                // Wait until the layout is valid to notify,
                // otherwise the caret position won't be correct.
                _pendingCaretSignal = true;
            }
            auto [begin, end] = _editing.selection();
            selectionChanged.emit(begin, end);
        }
    }

    void Text::subscribeToSelection() {
        _selectionSlots.emplace_back(mousePressed.subscribe(
            [this](const MouseEvent &event) {
                // 1 click: the caret, 2: the word, 3: the sentence, 4: everything
                apply(_editing.selectAt(indexAt(event.x, event.y), std::max(1u, event.clickCount), event.modifiers.shift));
                _onMouseMove = window()->mouseMoved.subscribe(
                    [this](const MouseEvent &move) {
                        int lx = 0;
                        int ly = 0;
                        globalToLocal(lx, ly, static_cast<int>(move.windowX), static_cast<int>(move.windowY));
                        apply(_editing.dragTo(indexAt(static_cast<float>(lx), static_cast<float>(ly))));
                    }
                );
            }
        ));

        _selectionSlots.emplace_back(mouseReleased.subscribe(
            [this] {
                _onMouseMove.disconnect();
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
            [this](const icu::UnicodeString &str) {
                apply(_editing.insert(str));
            }
        ));
    }

    void Text::removedFromRender() {
        Div::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-selection: the mouse up won't reach us
    }

    SkRect Text::caretRect(unsigned int index) const {
        // The text starts inside the padding and border, scrolled
        return _textLayout.caretRect(index).makeOffset(_paddedRect.fLeft - _rect.fLeft - _offsetX, _paddedRect.fTop - _rect.fTop);
    }

    unsigned int Text::indexAt(float x, float y) const {
        return _textLayout.indexAt(x - (_paddedRect.fLeft - _rect.fLeft - _offsetX), y - (_paddedRect.fTop - _rect.fTop));
    }

    void Text::handleKey(Key key, Mod mod) {
        WindowClipboard clipboard{window()};
        apply(_editing.handleKey(key, mod, *this, clipboard));
    }

    void Text::styleUpdated() {
        Div::styleUpdated();

        const float fontSize = _computedStyle.fontSize();
        const bool  changed  = _textLayout.setStyle(TextLayoutStyle{
            styleManager()->fontCollection(),
            _computedStyle.fontFamily().str(),
            fontSize,
            // Auto is 1.5 times the font size, a percent is of the font size
            _computedStyle.lineHeight().resolve(fontSize, fontSize * 1.5f),
            _computedStyle.letterSpacing(),
            _computedStyle.fontSmoothing(),
            _computedStyle.textAlign(),
            _computedStyle.textOverflow()
        });
        // Yoga caches measures: re-measure when anything changing the text metrics changes. Colors aren't part of
        // the layout: they are given when painting, so a color transition lays nothing out again
        if (changed) {
            invalidate();
        }
    }

    Size Text::measure(Constraint width, Constraint height) {
        // Pure: the layout caches measures and calls this with various constraints,
        // so the layout used for painting (at the box's width) is left as it is.
        float contentWidth = width.apply(std::ceil(_textLayout.maxContentWidth()));
        auto  lines        = std::max<size_t>(1, _textLayout.lineCountAt(contentWidth));
        return Size{contentWidth, height.apply(static_cast<float>(lines) * _textLayout.style().lineHeight)};
    }

    void Text::layoutUpdated() {
        Div::layoutUpdated();
        _textLayout.layout(_paddedRect.width());
        // One line of static text that doesn't fit is cut after the last character that fits whole (textOverflow's
        // Clip; Ellipsis is the layout's own)
        const bool cut = !_editing.multiline() && !_editable && _computedStyle.textOverflow() == TextOverflow::Clip;
        _visibleWidth  = cut ? _textLayout.fittingWidth(_paddedRect.width()) : std::numeric_limits<float>::infinity();
        if (_pendingCaretSignal) {
            caretMoved.emit(_editing.caret());
            _pendingCaretSignal = false;
        }
    }

    void Text::draw(SkCanvas *canvas) {
        Div::draw(canvas);
        const float left = _paddedRect.fLeft - _offsetX;
        // Scrolled, the line is clipped to the box (the caret's bar may reach a pixel past it); else one line of
        // static text that doesn't fit is cut after its last whole character
        const bool  scrolled = _offsetX != 0.0f;
        const bool  cut      = !scrolled && _visibleWidth < _textLayout.maxContentWidth();
        if (scrolled || cut) {
            canvas->save();
            // Only horizontally: glyphs can reach above and below the line
            const float lineHeight = _textLayout.style().lineHeight;
            const float right      = scrolled ? _paddedRect.fRight + 1.0f : _paddedRect.fLeft + _visibleWidth;
            canvas->clipRect(SkRect::MakeLTRB(_paddedRect.fLeft - (scrolled ? 1.0f : 0.0f), _paddedRect.fTop - lineHeight, right, _paddedRect.fBottom + lineHeight));
        }
        if (!_editing.text().isEmpty()) {
            _textLayout.paint(canvas, left, _paddedRect.fTop, _computedStyle.color().argb());
            const Color background = _computedStyle.selectionBackgroundColor();
            if (_editing.hasSelection() && background.alpha() > 0) {
                auto [begin, end] = _editing.selection();
                _textLayout.paintSelection(
                    canvas, left, _paddedRect.fTop, begin, end, background.argb(), _computedStyle.selectionColor().argb()
                );
            }
        }

        if (_caretVisible || (_editable && _focused)) {
            // A 2 px bar, in the text's color unless caretColor says otherwise
            const SkRect caret = _textLayout.caretRect(_editing.caret()).makeOffset(left, _paddedRect.fTop);
            SkPaint      paint{};
            paint.setAntiAlias(true);
            paint.setColor((_computedStyle.has(Property::caretColor) ? _computedStyle.caretColor() : _computedStyle.color()).argb());
            canvas->drawRect(caret.makeOutset(1.0f, 0.0f), paint);
        }
        if (scrolled || cut) {
            canvas->restore();
        }
    }

    void Text::updateTextInputArea() {
        Window *window = this->window();
        if (!window || !_focused || !_editable) {
            return;
        }
        // The caret's line, across the text, in window coordinates
        const SkRect caret = _textLayout.caretRect(_editing.caret());
        int          x     = 0;
        int          y     = 0;
        localToGlobal(
            x, y,
            static_cast<int>(_paddedRect.fLeft) - _x,
            static_cast<int>(_paddedRect.fTop + caret.fTop) - _y
        );
        window->setTextInputArea(
            x, y,
            static_cast<int>(std::ceil(_paddedRect.width())),
            static_cast<int>(std::ceil(caret.height())),
            static_cast<int>(caret.fLeft)
        );
    }
}
