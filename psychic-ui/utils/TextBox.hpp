#pragma once

/*
 * Heavily modified version of Skia's TextBox
 *
 * Original code Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in Skia's LICENSE file.
 */

#include <memory>
#include <vector>
#include <unicode/unistr.h>
#include <unicode/brkiter.h>
#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkTextBlob.h>

namespace psychic_ui {

    /**
     * TextBox mode values
     */
    enum class TextBoxMode {
        OneLine,
        LineBreak,
    };

    /**
     * TextBox vertical alignment values
     */
    enum TextBoxAlign {
        Start,
        Center,
        End,
    };

    /**
     * Shortcut for the TextBox visitor function type
     */
    using TextBoxVisitor = std::function<void(const char text[], size_t len, float x, float y)>;

    /**
     * @class TextBox
     *
     * TextBox is a helper class for drawing 1 or more lines of text
     * within a rectangle. The textbox is positioned and clipped by its Frame.
     * The Margin rectangle controls where the text is drawn relative to
     * the Frame. Line-breaks occur inside the Margin rectangle.
     *
     * Spacing is a linear equation used to compute the distance between lines
     * of text. Spacing consists of two scalars: mul and add, and the spacing
     * between lines is computed as: spacing = paint.getTextSize() * mul + add
     */
    class TextBox {
    public:
        /**
         * Construct a TextBox
         */
        TextBox();

        /**
         * Get the TextBox mode
         * @return
         */
        TextBoxMode getMode() const { return _mode; }

        /**
         * Set the TextBox mode
         *
         * @param mode
         */
        void setMode(TextBoxMode mode);

        /**
         * Get the vertical text alignment of the TextBox
         * @return
         */
        TextBoxAlign getAlign() const { return _align; }

        /**
         * Set the vertical text alignment of the TextBox
         *
         * @param align
         */
        void setAlign(TextBoxAlign align);

        /**
         * Set the box rectangle to use for the TextBox
         *
         * @param rect
         */
        void setBox(const SkRect &rect);

        /**
         * Set the box coordinates to use for the TextBox
         * @param left
         * @param top
         * @param right
         * @param bottom
         */
        void setBox(float left, float top, float right, float bottom);

        /**
         * Set the line spacing.
         * The spacing between lines is computed as:
         * spacing = paint.getTextSize() * mul + add
         *
         * @param mul
         * @param add
         */
        void setSpacing(float mul, float add);

        /**
         * Set the SkPaint used when drawing or computing the TextBlob
         * of this TextBox
         *
         * @param paint
         */
        void setPaint(const SkPaint &paint);

        /**
         * Set the text on which this TextBox makes its calculations
         *
         * @param text
         */
        void setText(const UnicodeString &text);

        /**
         * Resets the iterators in order to recalculate new values.
         * It is automatically called when setting the text, but must be
         * called manually when the text changes.
         */
        void recalculate();

        /**
         * Count the number of lines in the TextBox
         *
         * This will not cache the results since it has to be const
         * for the visit method
         *
         * @return TextBox's line count
         */
        unsigned int countLines() const;

        /**
         * Get the number of lines in the TextBox
         *
         * No calculations are involved but the TextBox must be up to date
         * (by counting lines, drawing or caching the text blob) for this method to work.
         *
         * @return TextBox's line count
         */
        unsigned int lineCount() const;

        /**
         * Get the height of the TextBox
         * @return TextBox's height
         */
        float getTextHeight() const;

        /**
         * Get the next line break from the provided start position
         *
         * @param start Where to start looking for a line break
         * @return Index of the next line break from start
         */
        unsigned int nextLineBreak(int start) const;

        /**
         * Get the caret index from the x, y coordinates.
         *
         * No calculations are involved but the TextBox must be up to date
         * (by counting lines, drawing or caching the text blob) for this method to work.
         *
         * @param x Horizontal coordinate in pixels
         * @param y Vertical coordinates in pixels
         * @return Index position of x,y in the text box
         */
        unsigned int indexFromPos(int x, int y) const;

        /**
         * Get the position from the caret index. The position will be returned
         * as a pair with the first element being the line number and the second
         * element being the x coordinate on that line. This could have returned
         * the x,y coordinates but it was more useful this way in the implementation
         * of text components.
         *
         * No calculations are involved but the TextBox must be up to date
         * (by counting lines, drawing or caching the text blob) for this method to work.
         *
         * @param index Caret index
         * @return Pair of line number and x coordinate
         */
        std::pair<unsigned int, unsigned int> posFromIndex(unsigned int index) const;

        /**
         * Get the character index at the start of the line.
         *
         * No calculations are involved but the TextBox must be up to date
         * (by counting lines, drawing or caching the text blob) for this method to work.
         *
         * @param line Line number for which to retrieve the start index
         * @return Start index of the passed line
         */
        unsigned int lineStart(unsigned int line) const;

        /**
         * Get the character index at the end of the line.
         *
         * No calculations are involved but the TextBox must be up to date
         * (by counting lines, drawing or caching the text blob) for this method to work.
         *
         * @param line Line number for which to retrieve the end index
         * @return End index of the passed line
         */
        unsigned int lineEnd(unsigned int line) const;

        /**
         * Get the line on which the passed index is.
         *
         * No calculations are involved but the TextBox must be up to date
         * (by counting lines, drawing or caching the text blob) for this method to work.
         *
         * @param index Character index for which to get the line
         * @return Line on which this character index appears
         */
        unsigned int lineFromIndex(unsigned int index) const;

        /**
         * Get the word at the index position.
         *
         * @param index
         * @return Pair of start and end word position
         */
        std::pair<unsigned int, unsigned int> wordAtIndex(unsigned int index) const;

        /**
         * Get the previous word boundary from index.
         *
         * @param index
         * @return Previous word boundary position
         */
        unsigned int previousWordBoundary(unsigned int index) const;

        /**
         * Get the next word boundary from index.
         *
         * @param index
         * @return Next word boundary position
         */
        unsigned int nextWordBoundary(unsigned int index) const;

        /**
         * Get the sentence at the index position.
         *
         * @param index
         * @return Pair of start and end sentence position
         */
        std::pair<unsigned int, unsigned int> sentenceAtIndex(unsigned int index) const;

        /**
         * Draw the text on the passed canvas
         *
         * This will cache line starts for the other methods
         *
         * @param canvas
         */
        void draw(SkCanvas *canvas);

        /**
         * Get a TextBlob snapshot for the TextBox
         * This will cache line starts for the other methods
         *
         * @return Unique pointer to a TextBlob
         */
        std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob *)>> snapshotTextBlob();

    private:
        std::unique_ptr<BreakIterator> lineIterator{nullptr};
        std::unique_ptr<BreakIterator> wordIterator{nullptr};
        std::unique_ptr<BreakIterator> sentenceIterator{nullptr};
        SkRect                         _box{};
        float                          _spacingMult{1.0f};
        float                          _spacingAdd{0.0f};
        TextBoxAlign                   _align{TextBoxAlign::Start};
        TextBoxMode                    _mode{TextBoxMode::LineBreak};
        const UnicodeString            *_text{nullptr};
        const SkPaint                  *_paint{nullptr};

        std::vector<unsigned int> visit(TextBoxVisitor visitor) const;

        // Calculated values
        std::vector<unsigned int> _lineStarts{};
    };
}
