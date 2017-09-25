/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <unicode/unistr.h>
#include <iostream>
#include <SkTextBlob.h>
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

    // endregion

    void TextBox::setPaint(const SkPaint &paint) {
        _paint = &paint;
    }

    void TextBox::setText(const std::string &text) {
        _text = UnicodeString::fromUTF8(text);
        breakIterator->setText(_text);
        _breakPositions.clear();
        int b = breakIterator->first();
        while (b != BreakIterator::DONE) {
            if (b != 0) {
                _breakPositions.push_back(b);
            }
            b = breakIterator->next();
        }
        // We always "break" at the end
        _breakPositions.push_back(_text.length());
    }

    int TextBox::countLines() const {
        int pos   = 0;
        int count = 0;
        if (_box.width() > 0) {
            do {
                count += 1;
                pos = nextLineBreak(pos);
            } while (pos < _text.length());
        }
        return count;
    }

    float TextBox::getTextHeight() const {
        float spacing = _paint->getTextSize() * _spacingMult + _spacingAdd;
        return countLines() * spacing;
    }

    int TextBox::nextLineBreak(int start) const {
        UnicodeString remaining = _text.tempSubStringBetween(start, _text.length());
        std::string   str;
        remaining.toUTF8String(str);

        // Start by find where the text would cut
        std::size_t maxBreak = start + _paint->breakText(str.c_str(), str.size(), _box.width());

        // Get the break just before where it would cut
        int lastBreakPosition = 0;

        for (const int _breakPosition : _breakPositions) {
            if (_breakPosition <= maxBreak) {
                lastBreakPosition = _breakPosition;
            } else {
                break;
            }
        }

        return lastBreakPosition;
    }

    float TextBox::visit(TextBoxVisitor visitor) const {
        float maxWidth = _box.width();

        if (maxWidth <= 0 || _text.length() == 0) {
            return _box.top();
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
        int lastBreak = 0;
        for (;;) {
            int nextBreak = nextLineBreak(lastBreak);
            if (y + metrics.fDescent + metrics.fLeading > 0) {
                std::string str{};
                _text.tempSubStringBetween(lastBreak, nextBreak).toUTF8String(str);
                visitor(str.c_str(), str.size(), x, y);
            }
            lastBreak = nextBreak;
            if (lastBreak >= _text.length()) {
                break;
            }
            y += scaledSpacing;
            if (y + metrics.fAscent >= _box.fBottom) {
                break;
            }
        }
        return y + metrics.fDescent + metrics.fLeading;
    }

    /// CANVAS VISITOR

    void TextBox::draw(SkCanvas *canvas) {
        visit(
            [this, canvas](const char text[], size_t len, float x, float y) {
                canvas->drawText(text, len, x, y, *_paint);
            }
        );
    }

    /// TEXT BLOB VISITOR

    std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob*)>> TextBox::snapshotTextBlob() const {
        SkTextBlobBuilder builder{};
        SkPaint           p(*_paint);
        p.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        visit(
            [this, &builder, &p](const char text[], size_t len, float x, float y) {
                _paint->textToGlyphs(text, len, builder.allocRun(p, _paint->countText(text, len), x, y).glyphs);
            }
        );
        return std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob*)>>(builder.make().release(), [](SkTextBlob* ptr)
        {
            //std::cout << "destroying from a custom deleter...\n";
            ptr->unref();
        });
    }
}
