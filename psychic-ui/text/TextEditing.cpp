#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <unicode/brkiter.h>
#include <unicode/locid.h>
#include <unicode/uchar.h>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include <unicode/utf16.h>
#include <unicode/utypes.h>
#include "include/core/SkRect.h"
#include "TextEditing.hpp"
#include "TextGeometry.hpp"
#include "psychic-ui/psychic-ui.hpp"

namespace psychic_ui {

    namespace {
        // Past any line's edge, for the line edge moves
        constexpr float far = 1.0e9f;

        using Create = icu::BreakIterator *(*)(const icu::Locale &, UErrorCode &);

        /**
         * The iterator, made on first use, pointed at the text: it reads the string itself, which may have changed
         * or moved since
         */
        icu::BreakIterator &pointed(std::unique_ptr<icu::BreakIterator> &iterator, const Create create, const icu::UnicodeString &text) {
            if (!iterator) {
                UErrorCode status = U_ZERO_ERROR;
                iterator.reset(create(icu::Locale::getDefault(), status));
                if (U_FAILURE(status) || !iterator) {
                    throw std::runtime_error("TextEditing: ICU break iterator unavailable");
                }
            }
            iterator->setText(text);
            return *iterator;
        }
    }

    icu::BreakIterator &TextEditing::graphemes() const {
        return pointed(_graphemes, &icu::BreakIterator::createCharacterInstance, _text);
    }

    icu::BreakIterator &TextEditing::words() const {
        return pointed(_words, &icu::BreakIterator::createWordInstance, _text);
    }

    icu::BreakIterator &TextEditing::sentences() const {
        return pointed(_sentences, &icu::BreakIterator::createSentenceInstance, _text);
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

    TextEditing::Edit TextEditing::select(unsigned int anchor, unsigned int caret) {
        _goalX.reset();
        return place(std::min(anchor, length()), std::min(caret, length()));
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
                if (std::u16string_view{u"\n\r\u2028\u2029\v\f\u0085"}.contains(inserted.charAt(i))) {
                    inserted.setCharAt(i, u' ');
                }
            }
        }
        auto [begin, end] = selection();
        return replace(begin, end, inserted);
    }

    TextEditing::Edit TextEditing::erase(Direction direction, Unit unit, const TextGeometry &geometry) {
        if (_readOnly) {
            return Edit::None;
        }
        auto [begin, end] = selection();
        if (begin == end) {
            const unsigned int destination = target(direction, unit, geometry);
            std::tie(begin, end) = std::minmax(_caret, destination);
        }
        return replace(begin, end, icu::UnicodeString{});
    }

    TextEditing::Edit TextEditing::move(Direction direction, Unit unit, bool extend, const TextGeometry &geometry) {
        if (unit != Unit::Line) {
            _goalX.reset(); // Line moves keep it, and set it on the first
        }
        if (!extend && hasSelection() && unit == Unit::Grapheme) {
            // An arrow collapses the selection to its side
            auto [begin, end] = selection();
            const unsigned int side = direction == Direction::Backward ? begin : end;
            return place(side, side);
        }
        const unsigned int destination = target(direction, unit, geometry);
        return place(extend ? _anchor : destination, destination);
    }

    unsigned int TextEditing::target(Direction direction, Unit unit, const TextGeometry &geometry) {
        const bool forward = direction == Direction::Forward;
        switch (unit) {
            case Unit::Grapheme:
                return forward ? nextGrapheme(_caret) : previousGrapheme(_caret);

            case Unit::Word:
                return forward ? nextWordEnd(_caret) : previousWordStart(_caret);

            case Unit::All:
                return forward ? length() : 0;

            case Unit::LineEdge: {
                const SkRect       caret = geometry.caretRect(_caret);
                const unsigned int edge  = geometry.indexAt(forward ? far : -far, caret.centerY());
                if (forward && edge > _caret && geometry.caretRect(edge).fTop != caret.fTop) {
                    // A wrapped line's end is the next line's start: stop before the space it wrapped at
                    return previousGrapheme(edge);
                }
                return edge;
            }

            case Unit::Line: {
                const SkRect caret = geometry.caretRect(_caret);
                if (!_goalX) {
                    _goalX = caret.fLeft;
                }
                const float        y     = forward ? caret.fBottom + (caret.height() / 2.0f) : caret.fTop - (caret.height() / 2.0f);
                const unsigned int index = geometry.indexAt(*_goalX, y);
                if (geometry.caretRect(index).fTop == caret.fTop) {
                    // No line there: the first line goes to the start, the last to the end
                    return forward ? length() : 0;
                }
                return index;
            }
        }
        return _caret;
    }

    unsigned int TextEditing::previousGrapheme(unsigned int index) const {
        const int32_t boundary = graphemes().preceding(static_cast<int32_t>(index));
        return boundary == icu::BreakIterator::DONE ? 0 : static_cast<unsigned int>(boundary);
    }

    unsigned int TextEditing::nextGrapheme(unsigned int index) const {
        const int32_t boundary = graphemes().following(static_cast<int32_t>(index));
        return boundary == icu::BreakIterator::DONE ? length() : static_cast<unsigned int>(boundary);
    }

    bool TextEditing::isWhitespace(unsigned int begin, unsigned int end) const {
        for (auto i = static_cast<int32_t>(begin); std::cmp_less(i, end);) {
            const UChar32 character = _text.char32At(i);
            if (!u_isUWhiteSpace(character)) {
                return false;
            }
            i += U16_LENGTH(character);
        }
        return true;
    }

    unsigned int TextEditing::previousWordStart(unsigned int index) const {
        icu::BreakIterator &iterator = words();
        unsigned int        end      = index;
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
        auto start = static_cast<int32_t>(std::min(index, length() - 1));
        const int32_t begin = iterator.isBoundary(start) ? start : iterator.preceding(start);
        const int32_t end   = iterator.following(begin);
        return {
            begin == icu::BreakIterator::DONE ? 0 : static_cast<unsigned int>(begin),
            end == icu::BreakIterator::DONE ? length() : static_cast<unsigned int>(end)
        };
    }

    std::pair<unsigned int, unsigned int> TextEditing::wordAt(unsigned int index) const {
        return segmentAt(words(), index);
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
            _pressed = segmentAt(sentences(), index);
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

    TextEditing::Edit TextEditing::handleKey(Key key, Mod mod, const TextGeometry &geometry, Clipboard &clipboard) {
        const bool      command   = mod.ctrl || mod.super;
        const Unit      character = mod.ctrl ? Unit::Word : Unit::Grapheme;
        const Direction direction = key == Key::HOME || key == Key::UP || key == Key::LEFT || key == Key::BACKSPACE
                                    ? Direction::Backward
                                    : Direction::Forward;
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
            case Key::END:
                return move(direction, mod.ctrl ? Unit::All : Unit::LineEdge, mod.shift, geometry);

            case Key::UP:
            case Key::DOWN:
                return move(direction, Unit::Line, mod.shift, geometry);

            case Key::LEFT:
            case Key::RIGHT:
                return move(direction, character, mod.shift, geometry);

            case Key::BACKSPACE:
            case Key::DELETE:
                return erase(direction, character, geometry);

            case Key::ENTER:
                return _multiline ? insert(icu::UnicodeString(u'\n')) : Edit::None;

            default:
                return Edit::None;
        }
    }
}
