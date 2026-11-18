#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <unicode/uchar.h>
#include "TextEditing.hpp"

namespace psychic_ui {

    namespace {
        // Past any line's edge, for the line edge moves
        constexpr float far = 1.0e9f;

        // Takes what ICU made, so that it is deleted when it is no use
        std::unique_ptr<icu::BreakIterator> made(icu::BreakIterator *iterator, UErrorCode status) {
            std::unique_ptr<icu::BreakIterator> owned{iterator};
            if (U_FAILURE(status) || !owned) {
                throw std::runtime_error("TextEditing: ICU break iterator unavailable");
            }
            return owned;
        }
    }

    TextEditing::TextEditing()                                   = default;
    TextEditing::~TextEditing()                                  = default;
    TextEditing::TextEditing(TextEditing &&) noexcept            = default;
    TextEditing &TextEditing::operator=(TextEditing &&) noexcept = default;

    icu::BreakIterator &TextEditing::graphemes() const {
        if (!_graphemes) {
            UErrorCode status = U_ZERO_ERROR;
            _graphemes = made(icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status), status);
        }
        // The iterator reads the string itself: point it at it again, as it may have changed or moved
        _graphemes->setText(_text);
        return *_graphemes;
    }

    icu::BreakIterator &TextEditing::words() const {
        if (!_words) {
            UErrorCode status = U_ZERO_ERROR;
            _words = made(icu::BreakIterator::createWordInstance(icu::Locale::getDefault(), status), status);
        }
        _words->setText(_text);
        return *_words;
    }

    icu::BreakIterator &TextEditing::sentences() const {
        if (!_sentences) {
            UErrorCode status = U_ZERO_ERROR;
            _sentences = made(icu::BreakIterator::createSentenceInstance(icu::Locale::getDefault(), status), status);
        }
        _sentences->setText(_text);
        return *_sentences;
    }

    const icu::UnicodeString &TextEditing::text() const {
        return _text;
    }

    std::string TextEditing::utf8() const {
        std::string str{};
        _text.toUTF8String(str);
        return str;
    }

    TextEditing::Edit TextEditing::setText(const icu::UnicodeString &text) {
        const Edit edit = _text == text ? Edit::None : Edit::Text;
        _text     = text;
        _goalX.reset();
        _pressed = {0, 0};
        return edit | place(0, 0);
    }

    unsigned int TextEditing::length() const {
        return static_cast<unsigned int>(_text.length());
    }

    unsigned int TextEditing::anchor() const {
        return _anchor;
    }

    unsigned int TextEditing::caret() const {
        return _caret;
    }

    std::pair<unsigned int, unsigned int> TextEditing::selection() const {
        return std::minmax(_anchor, _caret);
    }

    bool TextEditing::hasSelection() const {
        return _anchor != _caret;
    }

    std::u16string TextEditing::selectedText() const {
        auto [begin, end] = selection();
        return {reinterpret_cast<const char16_t *>(_text.getBuffer()) + begin, end - begin};
    }

    TextEditing::Edit TextEditing::select(unsigned int anchor, unsigned int caret) {
        _goalX.reset();
        return place(std::min(anchor, length()), std::min(caret, length()));
    }

    bool TextEditing::multiline() const {
        return _multiline;
    }

    void TextEditing::setMultiline(bool multiline) {
        _multiline = multiline;
    }

    bool TextEditing::readOnly() const {
        return _readOnly;
    }

    void TextEditing::setReadOnly(bool readOnly) {
        _readOnly = readOnly;
    }

    TextEditing::Edit TextEditing::place(unsigned int anchor, unsigned int caret) {
        if (anchor == _anchor && caret == _caret) {
            return Edit::None;
        }
        _anchor = anchor;
        _caret  = caret;
        return Edit::Caret;
    }

    TextEditing::Edit TextEditing::replace(unsigned int begin, unsigned int end, const icu::UnicodeString &str) {
        if (begin == end && str.isEmpty()) {
            return Edit::None;
        }
        _text.replace(static_cast<int32_t>(begin), static_cast<int32_t>(end - begin), str);
        _goalX.reset();
        const auto caret = begin + static_cast<unsigned int>(str.length());
        place(caret, caret);
        // The caret moves with the text even where its index stays
        return Edit::Text | Edit::Caret;
    }

    TextEditing::Edit TextEditing::insert(const icu::UnicodeString &str) {
        if (_readOnly) {
            return Edit::None;
        }
        icu::UnicodeString inserted = str;
        if (!_multiline) {
            // One line: line returns become spaces
            inserted.findAndReplace(u"\r\n", u" ");
            for (int32_t i = 0; i < inserted.length(); ++i) {
                const char16_t c = inserted.charAt(i);
                if (c == u'\n' || c == u'\r' || c == u' ' || c == u' ' || c == u'\v' || c == u'\f' || c == u'\u0085') {
                    inserted.setCharAt(i, u' ');
                }
            }
        }
        auto [begin, end] = selection();
        return replace(begin, end, inserted);
    }

    TextEditing::Edit TextEditing::erase(Direction direction, Unit unit, const Geometry *geometry) {
        if (_readOnly) {
            return Edit::None;
        }
        auto [begin, end] = selection();
        if (begin == end) {
            const unsigned int to = target(direction, unit, geometry);
            std::tie(begin, end) = std::minmax(_caret, to);
        }
        return replace(begin, end, icu::UnicodeString{});
    }

    TextEditing::Edit TextEditing::move(Direction direction, Unit unit, bool extend, const Geometry *geometry) {
        if (!extend && hasSelection() && unit == Unit::Grapheme) {
            // An arrow collapses the selection to its side
            auto [begin, end] = selection();
            _goalX.reset();
            const unsigned int side = direction == Direction::Backward ? begin : end;
            return place(side, side);
        }
        if (unit != Unit::Line) {
            _goalX.reset(); // Line moves keep it, and set it on the first
        }
        const unsigned int to = target(direction, unit, geometry);
        return place(extend ? _anchor : to, to);
    }

    unsigned int TextEditing::target(Direction direction, Unit unit, const Geometry *geometry) {
        const bool forward = direction == Direction::Forward;
        switch (unit) {
            case Unit::Grapheme:
                return forward ? nextGrapheme(_caret) : previousGrapheme(_caret);

            case Unit::Word:
                return forward ? nextWordEnd(_caret) : previousWordStart(_caret);

            case Unit::All:
                return forward ? length() : 0;

            case Unit::LineEdge: {
                if (!geometry) {
                    return forward ? length() : 0;
                }
                const SkRect       caret = geometry->caretRect(_caret);
                const unsigned int edge  = geometry->indexAt(forward ? far : -far, caret.centerY());
                if (forward && edge > _caret && geometry->caretRect(edge).fTop != caret.fTop) {
                    // A wrapped line's end is the next line's start: stop before the space it wrapped at
                    return previousGrapheme(edge);
                }
                return edge;
            }

            case Unit::Line: {
                if (!geometry) {
                    return forward ? length() : 0;
                }
                const SkRect caret = geometry->caretRect(_caret);
                if (!_goalX) {
                    _goalX = caret.fLeft;
                }
                const float        y     = forward ? caret.fBottom + (caret.height() / 2.0f) : caret.fTop - (caret.height() / 2.0f);
                const unsigned int index = geometry->indexAt(*_goalX, y);
                if (geometry->caretRect(index).fTop == caret.fTop) {
                    // No line there: the first line goes to the start, the last to the end
                    return forward ? length() : 0;
                }
                return index;
            }
        }
        return _caret;
    }

    unsigned int TextEditing::previousGrapheme(unsigned int index) const {
        if (index == 0) {
            return 0;
        }
        const int32_t boundary = graphemes().preceding(static_cast<int32_t>(std::min(index, length())));
        return boundary == icu::BreakIterator::DONE ? 0 : static_cast<unsigned int>(boundary);
    }

    unsigned int TextEditing::nextGrapheme(unsigned int index) const {
        if (index >= length()) {
            return length();
        }
        const int32_t boundary = graphemes().following(static_cast<int32_t>(index));
        return boundary == icu::BreakIterator::DONE ? length() : static_cast<unsigned int>(boundary);
    }

    bool TextEditing::isWhitespace(unsigned int begin, unsigned int end) const {
        for (int32_t i = static_cast<int32_t>(begin); i < static_cast<int32_t>(end);) {
            const UChar32 c = _text.char32At(i);
            if (!u_isUWhiteSpace(c)) {
                return false;
            }
            i += U16_LENGTH(c);
        }
        return true;
    }

    unsigned int TextEditing::previousWordStart(unsigned int index) const {
        icu::BreakIterator &iterator = words();
        unsigned int        end      = std::min(index, length());
        while (end > 0) {
            const int32_t begin = iterator.preceding(static_cast<int32_t>(end));
            if (begin == icu::BreakIterator::DONE) {
                return 0;
            }
            // The segment [begin, its next boundary), which may reach past the caret when it is inside a word
            const int32_t next = iterator.following(begin);
            if (!isWhitespace(static_cast<unsigned int>(begin), static_cast<unsigned int>(next))) {
                return static_cast<unsigned int>(begin);
            }
            end = static_cast<unsigned int>(begin);
        }
        return 0;
    }

    unsigned int TextEditing::nextWordEnd(unsigned int index) const {
        icu::BreakIterator &iterator = words();
        unsigned int        begin    = index;
        while (begin < length()) {
            const int32_t end = iterator.following(static_cast<int32_t>(begin));
            if (end == icu::BreakIterator::DONE) {
                return length();
            }
            const int32_t start = iterator.preceding(end);
            if (!isWhitespace(static_cast<unsigned int>(std::max<int32_t>(start, 0)), static_cast<unsigned int>(end))) {
                return static_cast<unsigned int>(end);
            }
            begin = static_cast<unsigned int>(end);
        }
        return length();
    }

    std::pair<unsigned int, unsigned int> TextEditing::segmentAt(icu::BreakIterator &iterator, unsigned int index) const {
        if (_text.isEmpty()) {
            return {0, 0};
        }
        // The segment the character at the index starts or is in (the last one at the end)
        auto at = static_cast<int32_t>(std::min(index, length() - 1));
        const int32_t begin = iterator.isBoundary(at) ? at : iterator.preceding(at);
        const int32_t end   = iterator.following(begin);
        return {
            begin == icu::BreakIterator::DONE ? 0 : static_cast<unsigned int>(begin),
            end == icu::BreakIterator::DONE ? length() : static_cast<unsigned int>(end)
        };
    }

    std::pair<unsigned int, unsigned int> TextEditing::wordAt(unsigned int index) const {
        return segmentAt(words(), index);
    }

    std::pair<unsigned int, unsigned int> TextEditing::sentenceAt(unsigned int index) const {
        return segmentAt(sentences(), index);
    }

    TextEditing::Edit TextEditing::selectAt(unsigned int index, unsigned int clickCount, bool extend) {
        index = std::min(index, length());
        _goalX.reset();
        if (clickCount <= 1) {
            if (extend) {
                _pressed = selection();
                return place(_anchor, index);
            }
            _pressed = {index, index};
        } else if (clickCount == 2) {
            _pressed = wordAt(index);
        } else if (clickCount == 3) {
            _pressed = sentenceAt(index);
        } else {
            _pressed = {0, length()};
        }
        return place(_pressed.first, _pressed.second);
    }

    TextEditing::Edit TextEditing::dragTo(unsigned int index) {
        index = std::min(index, length());
        _goalX.reset();
        // Before what the press selected, the selection runs back from its end; otherwise on from its start
        if (index < _pressed.first) {
            return place(_pressed.second, index);
        }
        return place(_pressed.first, std::max(index, _pressed.second));
    }

    TextEditing::Edit TextEditing::handleKey(Key key, Mod mod, const Geometry &geometry, Clipboard &clipboard) {
        const bool command = mod.ctrl || mod.super;
        switch (key) {
            case Key::A:
                return command ? select(0, length()) : Edit::None;

            case Key::C:
            case Key::X: {
                if (!command || !hasSelection() || (key == Key::X && _readOnly)) {
                    return Edit::None;
                }
                auto [begin, end] = selection();
                std::string selected{};
                _text.tempSubStringBetween(static_cast<int32_t>(begin), static_cast<int32_t>(end)).toUTF8String(selected);
                clipboard.setClipboardText(selected);
                return key == Key::X ? insert(icu::UnicodeString{}) : Edit::None;
            }

            case Key::V:
                return command ? insert(icu::UnicodeString::fromUTF8(clipboard.clipboardText())) : Edit::None;

            case Key::HOME:
                return move(Direction::Backward, mod.ctrl ? Unit::All : Unit::LineEdge, mod.shift, &geometry);

            case Key::END:
                return move(Direction::Forward, mod.ctrl ? Unit::All : Unit::LineEdge, mod.shift, &geometry);

            case Key::UP:
                return move(Direction::Backward, Unit::Line, mod.shift, &geometry);

            case Key::DOWN:
                return move(Direction::Forward, Unit::Line, mod.shift, &geometry);

            case Key::LEFT:
                return move(Direction::Backward, mod.ctrl ? Unit::Word : Unit::Grapheme, mod.shift, &geometry);

            case Key::RIGHT:
                return move(Direction::Forward, mod.ctrl ? Unit::Word : Unit::Grapheme, mod.shift, &geometry);

            case Key::BACKSPACE:
                return erase(Direction::Backward, mod.ctrl ? Unit::Word : Unit::Grapheme, &geometry);

            case Key::DELETE:
                return erase(Direction::Forward, mod.ctrl ? Unit::Word : Unit::Grapheme, &geometry);

            case Key::ENTER:
                return _multiline ? insert(icu::UnicodeString(u'\n')) : Edit::None;

            default:
                return Edit::None;
        }
    }
}
