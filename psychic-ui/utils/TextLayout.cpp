#include <algorithm>
#include <cmath>
#include "TextLayout.hpp"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skunicode/include/SkUnicode_icu.h"

namespace psychic_ui {

    namespace tl = skia::textlayout;

    namespace {
        SkFont::Edging edging(FontSmoothing smoothing) {
            switch (smoothing) {
                case FontSmoothing::None:
                    return SkFont::Edging::kAlias;
                case FontSmoothing::Subpixel:
                    return SkFont::Edging::kSubpixelAntiAlias;
                case FontSmoothing::Grayscale:
                default:
                    return SkFont::Edging::kAntiAlias;
            }
        }

        sk_sp<SkUnicode> unicode() {
            static sk_sp<SkUnicode> unicode = SkUnicodes::ICU::Make();
            return unicode;
        }

        tl::TextAlign textAlign(TextAlign align) {
            switch (align) {
                case TextAlign::End:
                    return tl::TextAlign::kEnd;
                case TextAlign::Left:
                    return tl::TextAlign::kLeft;
                case TextAlign::Right:
                    return tl::TextAlign::kRight;
                case TextAlign::Center:
                    return tl::TextAlign::kCenter;
                case TextAlign::Justify:
                    return tl::TextAlign::kJustify;
                case TextAlign::Start:
                default:
                    return tl::TextAlign::kStart;
            }
        }
    }

    TextLayout::TextLayout() {
        UErrorCode status = U_ZERO_ERROR;
        _wordIterator     = std::unique_ptr<icu::BreakIterator>(icu::BreakIterator::createWordInstance(icu::Locale::getDefault(), status));
        _sentenceIterator = std::unique_ptr<icu::BreakIterator>(icu::BreakIterator::createSentenceInstance(icu::Locale::getDefault(), status));
        setText(_text);
    }

    void TextLayout::setText(const icu::UnicodeString &text) {
        if (&text != &_text) {
            _text = text;
        }
        _utf16.assign(reinterpret_cast<const char16_t *>(_text.getBuffer()), static_cast<size_t>(_text.length()));
        // The iterators keep a reference to the text: reset them on every change
        _wordIterator->setText(_text);
        _sentenceIterator->setText(_text);
        changed();
    }

    void TextLayout::setStyle(const TextLayoutStyle &style, SkColor color, SkColor selectionColor) {
        if (style == _style && color == _color && selectionColor == _selectionColor) {
            return;
        }
        _style          = style;
        _color          = color;
        _selectionColor = selectionColor;
        changed();
    }

    void TextLayout::setMultiline(bool multiline) {
        if (multiline != _multiline) {
            _multiline = multiline;
            changed();
        }
    }

    void TextLayout::changed() {
        _paragraph         = nullptr;
        _selectedParagraph = nullptr;
        _paragraphWidth    = -1.0f;
        _linesValid        = false;
    }

    std::unique_ptr<tl::Paragraph> TextLayout::build(SkColor color) const {
        // Line heights are multiples of the font size; every line gets exactly the line height
        float height = _style.fontSize > 0.0f ? _style.lineHeight / _style.fontSize : 1.0f;
        std::vector<SkString> families{SkString(_style.fontFamily.c_str())};

        tl::TextStyle textStyle{};
        textStyle.setColor(color);
        textStyle.setFontFamilies(families);
        textStyle.setFontSize(_style.fontSize);
        textStyle.setLetterSpacing(_style.letterSpacing);
        textStyle.setHeight(height);
        textStyle.setHeightOverride(true);
        textStyle.setHalfLeading(true);
        textStyle.setFontEdging(edging(_style.smoothing));
        // Glyphs are always positioned to the subpixel, which wants their exact advances: slight hinting only snaps
        // vertically and keeps them linear (normal hinting rounds every advance to a whole pixel and crowds small text)
        textStyle.setSubpixel(true);
        textStyle.setFontHinting(SkFontHinting::kSlight);

        tl::StrutStyle strut{};
        strut.setStrutEnabled(true);
        strut.setForceStrutHeight(true); // Fallback fonts don't make their lines taller
        strut.setFontFamilies(families);
        strut.setFontSize(_style.fontSize);
        strut.setHeight(height);
        strut.setHeightOverride(true);
        strut.setHalfLeading(true);

        tl::ParagraphStyle paragraphStyle{};
        paragraphStyle.setTextStyle(textStyle);
        paragraphStyle.setStrutStyle(strut);
        paragraphStyle.setTextAlign(textAlign(_style.align));
        paragraphStyle.setReplaceTabCharacters(true);
        paragraphStyle.setApplyRoundingHack(false); // Flutter's rounding of line widths, which can wrap a line early

        auto fonts   = _style.fonts ? _style.fonts : sk_make_sp<tl::FontCollection>();
        auto builder = tl::ParagraphBuilder::make(paragraphStyle, fonts, unicode());
        if (_multiline) {
            builder->addText(_utf16);
        } else {
            // One line: line returns lay out as spaces (like Label), one for one so indices hold
            std::u16string oneLine = _utf16;
            for (auto &c: oneLine) {
                if (c == u'\n' || c == u'\r' || c == u'\u2028' || c == u'\u2029' || c == u'\v' || c == u'\f' || c == u'\u0085') {
                    c = u' ';
                }
            }
            builder->addText(oneLine);
        }
        return builder->Build();
    }

    tl::Paragraph *TextLayout::paragraphAt(float width) {
        if (!_paragraph) {
            _paragraph      = build(_color);
            _paragraphWidth = -1.0f;
        }
        float w = std::isnan(width) ? 0.0f : std::max(0.0f, width);
        if (!_multiline) {
            // Never wrap: as wide as the text when it overflows. The intrinsic width comes from
            // any layout.
            if (_paragraphWidth < 0.0f) {
                _paragraph->layout(w);
                _paragraphWidth = w;
            }
            w = std::max(w, std::ceil(_paragraph->getMaxIntrinsicWidth()));
        }
        if (w != _paragraphWidth) {
            _paragraph->layout(w);
            _paragraphWidth = w;
        }
        return _paragraph.get();
    }

    tl::Paragraph *TextLayout::laidOut() {
        auto *paragraph = paragraphAt(_width);
        if (!_linesValid) {
            _lines.clear();
            paragraph->getLineMetrics(_lines);
            if (_lines.empty()) {
                // Empty text still has a line for the caret
                _lines.emplace_back();
            }
            // A line starts where the one before it ends: SkParagraph gives the empty line after
            // a final line return the start of the line before
            for (size_t i = 1; i < _lines.size(); ++i) {
                _lines[i].fStartIndex = _lines[i - 1].fEndIncludingNewline;
            }
            _linesValid = true;
        }
        return paragraph;
    }

    float TextLayout::maxContentWidth() {
        return paragraphAt(_paragraphWidth >= 0.0f ? _paragraphWidth : _width)->getMaxIntrinsicWidth();
    }

    size_t TextLayout::lineCountAt(float width) {
        if (_utf16.empty() || !(width > 0.0f)) {
            return 0;
        }
        if (!_multiline) {
            return 1;
        }
        return paragraphAt(width)->lineNumber();
    }

    void TextLayout::layout(float width) {
        if (width != _width) {
            _width      = width;
            _linesValid = false;
        }
    }

    void TextLayout::paint(SkCanvas *canvas, float x, float y) {
        if (_utf16.empty() || !(_width > 0.0f)) {
            return;
        }
        laidOut()->paint(canvas, x, y);
    }

    void TextLayout::paintSelected(SkCanvas *canvas, float x, float y) {
        if (_utf16.empty() || !(_width > 0.0f)) {
            return;
        }
        laidOut();
        if (!_selectedParagraph) {
            _selectedParagraph = build(_selectionColor);
            _selectedWidth     = -1.0f;
        }
        if (_selectedWidth != _paragraphWidth) {
            _selectedParagraph->layout(_paragraphWidth);
            _selectedWidth = _paragraphWidth;
        }
        _selectedParagraph->paint(canvas, x, y);
    }

    float TextLayout::fittingWidth(float width) {
        auto *paragraph = laidOut();
        if (paragraph->getMaxIntrinsicWidth() <= width) {
            return width;
        }
        tl::Paragraph::GlyphClusterInfo cluster{};
        if (!paragraph->getClosestGlyphClusterAt(width, _style.lineHeight / 2.0f, &cluster)) {
            return width;
        }
        return cluster.fBounds.fRight <= width ? cluster.fBounds.fRight : cluster.fBounds.fLeft;
    }

    unsigned int TextLayout::lineCount() {
        laidOut();
        return static_cast<unsigned int>(_lines.size());
    }

    unsigned int TextLayout::lineStart(unsigned int line) {
        laidOut();
        return static_cast<unsigned int>(_lines[std::min<size_t>(line, _lines.size() - 1)].fStartIndex);
    }

    unsigned int TextLayout::lineEnd(unsigned int line) {
        laidOut();
        if (line + 1 >= _lines.size()) {
            return static_cast<unsigned int>(_utf16.size());
        }
        return static_cast<unsigned int>(_lines[line + 1].fStartIndex) - 1;
    }

    unsigned int TextLayout::lineFromIndex(unsigned int index) {
        laidOut();
        unsigned int line = 0;
        for (unsigned int i = 0; i < _lines.size(); ++i) {
            if (index < _lines[i].fStartIndex) {
                break;
            }
            line = i;
        }
        return line;
    }

    std::pair<unsigned int, float> TextLayout::posFromIndex(unsigned int index) {
        auto         *paragraph = laidOut();
        unsigned int line       = lineFromIndex(index);
        float        x          = static_cast<float>(_lines[line].fLeft);
        if (index > _lines[line].fStartIndex) {
            // After the grapheme before the caret
            tl::Paragraph::GlyphInfo glyph{};
            if (paragraph->getGlyphInfoAtUTF16Offset(index - 1, &glyph)) {
                x = glyph.fDirection == tl::TextDirection::kRtl ? glyph.fGraphemeLayoutBounds.fLeft : glyph.fGraphemeLayoutBounds.fRight;
            }
        }
        return std::make_pair(line, x);
    }

    unsigned int TextLayout::indexFromPos(float x, float y) {
        if (_utf16.empty()) {
            return 0;
        }
        auto position = laidOut()->getGlyphPositionAtCoordinate(x, y).position;
        return static_cast<unsigned int>(std::clamp<int32_t>(position, 0, static_cast<int32_t>(_utf16.size())));
    }

    std::pair<unsigned int, unsigned int> TextLayout::wordAtIndex(unsigned int index) const {
        auto begin = _wordIterator->preceding(static_cast<int32_t>(index));
        auto end   = _wordIterator->following(static_cast<int32_t>(index));
        return std::make_pair(
            begin != icu::BreakIterator::DONE ? static_cast<unsigned int>(begin) : 0,
            end != icu::BreakIterator::DONE ? static_cast<unsigned int>(end) : static_cast<unsigned int>(_text.length())
        );
    }

    std::pair<unsigned int, unsigned int> TextLayout::sentenceAtIndex(unsigned int index) const {
        auto begin = _sentenceIterator->preceding(static_cast<int32_t>(index));
        auto end   = _sentenceIterator->following(static_cast<int32_t>(index));
        return std::make_pair(
            begin != icu::BreakIterator::DONE ? static_cast<unsigned int>(begin) : 0,
            end != icu::BreakIterator::DONE ? static_cast<unsigned int>(end) : static_cast<unsigned int>(_text.length())
        );
    }

    unsigned int TextLayout::previousWordBoundary(unsigned int index) const {
        auto boundary = _wordIterator->preceding(static_cast<int32_t>(index));
        return boundary != icu::BreakIterator::DONE ? static_cast<unsigned int>(boundary) : 0;
    }

    unsigned int TextLayout::nextWordBoundary(unsigned int index) const {
        auto boundary = _wordIterator->following(static_cast<int32_t>(index));
        return boundary != icu::BreakIterator::DONE ? static_cast<unsigned int>(boundary) : static_cast<unsigned int>(_text.length());
    }
}
