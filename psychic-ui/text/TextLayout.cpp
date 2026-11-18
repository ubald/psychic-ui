#include <algorithm>
#include <cmath>
#include "TextLayout.hpp"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphPainter.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "include/core/SkPathBuilder.h"
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

        std::size_t built = 0; // Paragraphs built, by every TextLayout

        /**
         * Paints a paragraph in one color, whatever the color it was built with: SkParagraph keeps the
         * paint of each run in its lines' text blobs once painted, so the color given at paint time
         * replaces it here rather than in the paragraph (`updateForegroundPaint` would need the
         * paragraph laid out again to reach those blobs)
         */
        class ColorPainter final : public tl::ParagraphPainter {
        public:
            ColorPainter(SkCanvas *canvas, SkColor color) : _canvas(canvas), _color(color) {}

            void drawTextBlob(const sk_sp<SkTextBlob> &blob, SkScalar x, SkScalar y, const SkPaintOrID &paint) override {
                SkPaint p = std::get<SkPaint>(paint);
                p.setColor(_color);
                _canvas->drawTextBlob(blob, x, y, p);
            }

            // What follows only paints what this layout never builds (shadows, backgrounds, decorations)
            void drawTextShadow(const sk_sp<SkTextBlob> &blob, SkScalar x, SkScalar y, SkColor color, SkScalar /*blurSigma*/) override {
                SkPaint p;
                p.setColor(color);
                _canvas->drawTextBlob(blob, x, y, p);
            }

            void drawRect(const SkRect &rect, const SkPaintOrID &paint) override {
                _canvas->drawRect(rect, std::get<SkPaint>(paint));
            }

            void drawFilledRect(const SkRect &rect, const DecorationStyle &style) override {
                _canvas->drawRect(rect, style.skPaint());
            }

            void drawPath(const SkPath &path, const DecorationStyle &style) override {
                _canvas->drawPath(path, style.skPaint());
            }

            void drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const DecorationStyle &style) override {
                _canvas->drawLine(x0, y0, x1, y1, style.skPaint());
            }

            void clipRect(const SkRect &rect) override { _canvas->clipRect(rect); }
            void translate(SkScalar dx, SkScalar dy) override { _canvas->translate(dx, dy); }
            void save() override { _canvas->save(); }
            void restore() override { _canvas->restore(); }

        private:
            SkCanvas *_canvas;
            SkColor   _color;
        };

        sk_sp<SkUnicode> unicode() {
            static const sk_sp<SkUnicode> unicode = SkUnicodes::ICU::Make();
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

    TextLayout::TextLayout() = default;

    std::size_t TextLayout::paragraphsBuilt() {
        return built;
    }

    void TextLayout::setText(const icu::UnicodeString &text) {
        _text.assign(text.getBuffer(), static_cast<size_t>(text.length()));
        changed();
    }

    bool TextLayout::setStyle(const TextLayoutStyle &style) {
        if (style == _style) {
            return false;
        }
        _style = style;
        changed();
        return true;
    }

    const TextLayoutStyle &TextLayout::style() const {
        return _style;
    }

    void TextLayout::setMultiline(bool multiline) {
        if (multiline != _multiline) {
            _multiline = multiline;
            changed();
        }
    }

    void TextLayout::changed() {
        _paragraph      = nullptr;
        _paragraphWidth = -1.0f;
        _linesValid     = false;
    }

    std::unique_ptr<tl::Paragraph> TextLayout::build() const {
        ++built;
        // Line heights are multiples of the font size; every line gets exactly the line height
        const float height = _style.fontSize > 0.0f ? _style.lineHeight / _style.fontSize : 1.0f;
        const std::vector<SkString> families{SkString(_style.fontFamily.c_str())};

        tl::TextStyle textStyle{};
        textStyle.setColor(SK_ColorBLACK); // Painted in the color given to `paint`
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
        if (ellipsis()) {
            paragraphStyle.setEllipsis(std::u16string{u"\u2026"});
            paragraphStyle.setMaxLines(1);
        }

        auto fonts   = _style.fonts ? _style.fonts : sk_make_sp<tl::FontCollection>();
        auto builder = tl::ParagraphBuilder::make(paragraphStyle, fonts, unicode());
        if (_multiline) {
            builder->addText(_text);
        } else {
            // One line: line returns lay out as spaces (like Label), one for one so indices hold
            std::u16string oneLine = _text;
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
            _paragraph      = build();
            _paragraphWidth = -1.0f;
        }
        float w = std::isnan(width) ? 0.0f : std::max(0.0f, width);
        if (!_multiline && !ellipsis()) {
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

    bool TextLayout::ellipsis() const {
        return !_multiline && _style.overflow == TextOverflow::Ellipsis;
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
        if (_text.empty() || !(width > 0.0f)) {
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

    void TextLayout::paint(SkCanvas *canvas, float x, float y, SkColor color) {
        if (_text.empty() || !(_width > 0.0f)) {
            return;
        }
        ColorPainter painter{canvas, color};
        laidOut()->paint(&painter, x, y);
    }

    void TextLayout::paintSelection(SkCanvas *canvas, float x, float y, unsigned int begin, unsigned int end, SkColor background, SkColor foreground) {
        if (begin > end) {
            std::swap(begin, end);
        }
        if (begin == end || _text.empty() || !(_width > 0.0f)) {
            return;
        }
        auto *paragraph = laidOut();
        // Every line is the strut's height, so its rectangles are too; a line's line return selects nothing
        auto boxes = paragraph->getRectsForRange(begin, end, tl::RectHeightStyle::kStrut, tl::RectWidthStyle::kTight);
        if (boxes.empty()) {
            return;
        }
        SkPathBuilder builder{};
        for (const auto &box: boxes) {
            builder.addRect(box.rect.makeOffset(x, y));
        }
        const SkPath path = builder.detach();

        SkPaint paint{};
        paint.setAntiAlias(true);
        paint.setColor(background);
        canvas->drawPath(path, paint);
        canvas->save();
        canvas->clipPath(path, true);
        ColorPainter painter{canvas, foreground};
        paragraph->paint(&painter, x, y);
        canvas->restore();
    }

    const tl::LineMetrics &TextLayout::lineAt(unsigned int index) const {
        // The last line starting at or before the index
        auto line = std::upper_bound(_lines.begin(), _lines.end(), index, [](unsigned int i, const tl::LineMetrics &l) {
            return i < l.fStartIndex;
        });
        return line == _lines.begin() ? _lines.front() : *std::prev(line);
    }

    SkRect TextLayout::caretRect(unsigned int index) {
        auto                  *paragraph = laidOut();
        const tl::LineMetrics &line      = lineAt(index);
        auto                   x         = static_cast<float>(line.fLeft);
        if (index > line.fStartIndex) {
            // After the grapheme before the caret
            tl::Paragraph::GlyphInfo glyph{};
            if (paragraph->getGlyphInfoAtUTF16Offset(index - 1, &glyph)) {
                x = glyph.fDirection == tl::TextDirection::kRtl ? glyph.fGraphemeLayoutBounds.fLeft : glyph.fGraphemeLayoutBounds.fRight;
            }
        }
        const float top = static_cast<float>(&line - _lines.data()) * _style.lineHeight;
        return SkRect::MakeLTRB(x, top, x, top + _style.lineHeight);
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

    unsigned int TextLayout::indexAt(float x, float y) {
        if (_text.empty()) {
            return 0;
        }
        auto position = laidOut()->getGlyphPositionAtCoordinate(x, y).position;
        return static_cast<unsigned int>(std::clamp<int32_t>(position, 0, static_cast<int32_t>(_text.size())));
    }
}
