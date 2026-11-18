#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    /**
     * What a TextLayout lays its text out with, from a TextBase's computed style
     */
    struct TextLayoutStyle {
        sk_sp<skia::textlayout::FontCollection> fonts{nullptr};
        std::string                             fontFamily{};
        float                                   fontSize{0.0f};
        float                                   lineHeight{0.0f};
        float                                   letterSpacing{0.0f};
        FontSmoothing                           smoothing{FontSmoothing::Grayscale};
        TextAlign                               align{TextAlign::Start};

        bool operator==(const TextLayoutStyle &) const = default;
    };

    /**
     * Lays out text with SkParagraph: shaping, font fallback, line breaking and alignment, and
     * the queries editing needs (lines, caret positions, hit testing, word and sentence
     * boundaries). Every index is in UTF-16 code units, like icu::UnicodeString's.
     *
     * Every line is `lineHeight` tall, its glyphs centred in it (CSS's half-leading). One-line
     * layouts never wrap: their text overflows the width instead, and line returns lay out as
     * spaces.
     *
     * Colors are given when painting, so a color change (a transition) never builds or lays out the
     * paragraph again.
     */
    class TextLayout {
    public:
        TextLayout();

        /**
         * Set the text, copied (call again after changing it)
         */
        void setText(const icu::UnicodeString &text);

        /**
         * Set what the text is laid out with
         */
        void setStyle(const TextLayoutStyle &style);

        /**
         * Whether the text wraps at the width (or stays on one line, past it)
         */
        void setMultiline(bool multiline);

        /**
         * The width of the widest line when nothing wraps
         */
        float maxContentWidth();

        /**
         * How many lines the text takes at that width, without changing the layout used for
         * painting and the queries below
         */
        size_t lineCountAt(float width);

        /**
         * Lay the text out at the width of its box
         */
        void layout(float width);

        /**
         * Paint the text in a color with its top left at (x, y)
         */
        void paint(SkCanvas *canvas, float x, float y, SkColor color);

        /**
         * Paint the selection from `begin` to `end` over the text painted at (x, y): its rectangles
         * (a line's height each, in visual order, so bidirectional text gets several) in `background`,
         * and the text inside them again in `foreground`
         */
        void paintSelection(SkCanvas *canvas, float x, float y, unsigned int begin, unsigned int end, SkColor background, SkColor foreground);

        /**
         * The caret before the character at the index: a zero-width rectangle across its line
         */
        SkRect caretRect(unsigned int index);

        /**
         * The caret index closest to a point
         */
        unsigned int indexAt(float x, float y);

        /**
         * The x where the visible text ends when it's cut at the width: the end of the last
         * glyph cluster that fits whole
         */
        float fittingWidth(float width);

        unsigned int lineCount();
        unsigned int lineStart(unsigned int line);

        /**
         * Index of the end of the line: before its line return, or before the first
         * character of the next line when it wraps
         */
        unsigned int lineEnd(unsigned int line);
        unsigned int lineFromIndex(unsigned int index);

        /**
         * The line and the x of the caret before the character at the index
         */
        std::pair<unsigned int, float> posFromIndex(unsigned int index);

        std::pair<unsigned int, unsigned int> wordAtIndex(unsigned int index) const;
        std::pair<unsigned int, unsigned int> sentenceAtIndex(unsigned int index) const;
        unsigned int previousWordBoundary(unsigned int index) const;
        unsigned int nextWordBoundary(unsigned int index) const;

        /**
         * How many paragraphs every TextLayout has built so far, for tests of what rebuilds them
         */
        static std::size_t paragraphsBuilt();

    private:
        std::u16string                             _text{}; // The only copy, which the iterators read too
        TextLayoutStyle                            _style{};
        bool                                       _multiline{false};
        float                                      _width{0.0f};
        float                                      _paragraphWidth{-1.0f}; // What _paragraph is laid out at
        bool                                       _linesValid{false};
        std::unique_ptr<skia::textlayout::Paragraph> _paragraph{nullptr};
        std::vector<skia::textlayout::LineMetrics>  _lines{};
        std::unique_ptr<icu::BreakIterator>        _wordIterator{nullptr};
        std::unique_ptr<icu::BreakIterator>        _sentenceIterator{nullptr};

        std::unique_ptr<skia::textlayout::Paragraph> build() const;

        /**
         * The paragraph, built if needed and laid out at `width` (another width than the
         * painting one only to measure)
         */
        skia::textlayout::Paragraph *paragraphAt(float width);

        /**
         * The paragraph laid out at the box width, with its line metrics
         */
        skia::textlayout::Paragraph *laidOut();

        void changed();
    };
}
