/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <iostream>
#include <cmath>
#include "TextBox.hpp"

namespace psychic_ui {

    TextBox::TextBox() {
        // TODO: Maybe this can be static
        UErrorCode status = U_ZERO_ERROR;
        breakIterator = std::unique_ptr<BreakIterator>(BreakIterator::createLineInstance(Locale::getDefault(), status));
    }

    // region Properties

    void TextBox::setMode(TextBoxMode mode) {
        _mode = mode;
    }

    void TextBox::setAlign(TextBoxAlign align) {
        _align = align;
    }

    void TextBox::setBox(const SkRect &box) {
        _box = box;
    }

    void TextBox::setBox(float left, float top, float right, float bottom) {
        _box.set(left, top, right, bottom);
    }

    void TextBox::setSpacing(float mul, float add) {
        _spacingMult = mul;
        _spacingAdd  = add;
    }

    void TextBox::setPaint(const SkPaint &paint) {
        _paint = &paint;
    }

    // endregion

    void TextBox::setText(const UnicodeString &text) {
        _text = &text;
        recalculate();
    }

    void TextBox::recalculate() {
        breakIterator->setText(*_text);
        _possibleBreakPositions.clear();
        int b = breakIterator->first();
        while (b != BreakIterator::DONE) {
            if (b != 0) {
                _possibleBreakPositions.push_back(static_cast<unsigned int>(b));
            }
            b = breakIterator->next();
        }
        // We always "break" at the end
        _possibleBreakPositions.push_back(static_cast<unsigned int>(_text->length()));
    }

    int TextBox::countLines() const {
        int count = 0;
        if (_box.width() > 0) {
            int pos = 0;
            do {
                count += 1;
                pos = nextLineBreak(pos);
                if (pos == 0) {
                    // Skia's breakText broke down, we're probably narrower than a character
                    // Just assume we're one character wide so that our height is equal to the
                    // length of the text
                    count = _text->length();
                    break;
                }
            } while (pos < _text->length());
        }
        return count;
    }

    unsigned int TextBox::lineCount() const {
        return static_cast<unsigned int>(_lineStarts.size());
    }

    float TextBox::getTextHeight() const {
        float spacing = _paint->getFontSpacing() * _spacingMult + _spacingAdd;
        return countLines() * spacing;
    }

    unsigned int TextBox::nextLineBreak(int start) const {
        UnicodeString remaining = _text->tempSubStringBetween(start, _text->length());
        std::string   str;
        remaining.toUTF8String(str);

        // Start by find where the text would cut at max if we were not to use UnicodeString
        auto advance = static_cast<unsigned int>(_paint->breakText(str.c_str(), str.size(), _box.width()));
        if (advance == 0) {
            return 0;
        }

        // Check if a line return exists before that
        auto lineBreak = _text->indexOf('\n', start, advance);
        if (lineBreak != -1) {
            return static_cast<unsigned int>(lineBreak) + 1;
        }

        unsigned int maxBreak = start + advance;

        // Get the break just before where it would cut
        unsigned int lastBreakPosition = maxBreak;

        for (const unsigned int _breakPosition : _possibleBreakPositions) {
            if (_breakPosition <= start) {
                // Can't consider this one, continue searching
                continue;
            } else if (_breakPosition > start && _breakPosition <= maxBreak) {
                // Consider this one but keep going until another one matches
                lastBreakPosition = _breakPosition;
            } else {
                // We're all out of options
                break;
            }
        }

        return lastBreakPosition;
    }

    std::vector<unsigned int> TextBox::visit(TextBoxVisitor visitor) const {
        std::vector<unsigned int> lines{};

        float maxWidth = _box.width();

        if (maxWidth <= 0 || _text->length() == 0) {
            return lines;
        }

        float                x = 0.0f;
        float                y = 0.0f;
        SkPaint::FontMetrics metrics{};

        switch (_paint->getTextAlign()) {
            case SkPaint::kLeft_Align:
                x = 0;
                break;
            case SkPaint::kCenter_Align:
                x = maxWidth * 0.5f;
                break;
            default:
                x = maxWidth;
                break;
        }

        x += _box.fLeft;

        float fontHeight    = _paint->getFontMetrics(&metrics);
        float scaledSpacing = fontHeight * _spacingMult + _spacingAdd;
        float height        = _box.height();

        // Compute Y position for first line
        float textHeight = fontHeight;
        if (_mode == TextBoxMode::LineBreak && _align != TextBoxAlign::Start) {
            textHeight += scaledSpacing * (countLines() - 1);
        }

        switch (_align) {
            case TextBoxAlign::Start:
                y = 0;
                break;
            case TextBoxAlign::Center:
                y = (height - textHeight) * 0.5f;
                break;
            case TextBoxAlign::End:
                y = height - textHeight;
                break;
        }

        y += _box.fTop - metrics.fAscent;

        // Break lines
        unsigned int lastBreak = 0;
        for (;;) {
            unsigned int nextBreak = nextLineBreak(lastBreak);
            if (y + metrics.fDescent + metrics.fLeading > 0) {
                std::string str{};
                _text->tempSubStringBetween(lastBreak, nextBreak).toUTF8String(str);
                visitor(str.c_str(), str.size(), x, y);
            }
            // We actually want the lines to start at 0 and skip the last break
            lines.push_back(lastBreak);
            lastBreak = nextBreak;
            if (lastBreak >= _text->length()) {
                break;
            }
            y += scaledSpacing;
            if (y + metrics.fAscent >= _box.fBottom) {
                break;
            }
        }
        //return y + metrics.fDescent + metrics.fLeading;
        return lines;
    }

    unsigned int TextBox::indexFromPos(int x, int y) const {
        if (_lineStarts.empty()) {
            return 0;
        }

        float lineHeight = _paint->getFontSpacing() * _spacingMult + _spacingAdd;
        auto  line       = static_cast<int>(std::floor((static_cast<float>(y) - _box.fTop) / lineHeight));

        if (line < 0) {
            line = 0;
        } else if (line >= _lineStarts.size()) {
            line = static_cast<int>(_lineStarts.size()) - 1;
        }

        int lineStart = _lineStarts[line];

        std::string str{};
        // TODO: This assumes the text runs all the way to the end and does not account for a possible line break
        //       If not at the last line it should check between lineStart and lineStart of the next line
        _text->tempSubStringBetween(lineStart, _text->length()).toUTF8String(str);

        std::vector<SkScalar> widths(str.size());
        _paint->getTextWidths(str.c_str(), str.size(), &widths.front());
        unsigned int pos    = 0;
        float        xCheck = x + _box.fLeft;
        float        acc    = 0.0f;
        for (pos = 0; pos < widths.size(); ++pos) {
            float width     = widths[pos];
            float halfWidth = width * 0.5f;
            if (xCheck < acc + halfWidth) {
                break;
            }
            acc += width;
        }

        return lineStart + pos;
    }

    std::pair<int, int> TextBox::posFromIndex(int index) const {
        int               line      = 0;
        int               lineStart = 0;
        for (unsigned int i         = 0; i < _lineStarts.size(); ++i) {
            if (index < _lineStarts[i]) {
                break;
            }
            line      = i;
            lineStart = _lineStarts[i];
        }

        std::string str{};
        _text->tempSubStringBetween(lineStart, index).toUTF8String(str);
        auto x = static_cast<int>(std::round(_paint->measureText(str.c_str(), str.size())));

        return std::make_pair(line, x);
    }

    /// CANVAS VISITOR

    void TextBox::draw(SkCanvas *canvas) {
        _lineStarts = visit(
            [this, canvas](const char text[], size_t len, float x, float y) {
                canvas->drawText(text, len, x, y, *_paint);
            }
        );
    }

    /// TEXT BLOB VISITOR

    std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob *)>> TextBox::snapshotTextBlob() {
        SkTextBlobBuilder builder{};
        SkPaint           p(*_paint);
        p.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        _lineStarts = visit(
            [this, &builder, &p](const char text[], size_t len, float x, float y) {
                _paint->textToGlyphs(text, len, builder.allocRun(p, _paint->countText(text, len), x, y).glyphs);
            }
        );
        return std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob *)>>(builder.make().release(), [](SkTextBlob *ptr) { ptr->unref(); });
    }
}
