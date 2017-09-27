#pragma once

#include <string>
#include <SkTextBlob.h>
#include "psychic-ui/utils/TextBox.hpp"
#include "psychic-ui/TextBase.hpp"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {

    /**
     * @class Text
     *
     * Displays single or multiline text that is optionally selectable.
     */
    class Text : public TextBase {
    public:
        explicit Text(const std::string &text = "");
        void setText(const std::string &text) override;
        const bool selectable() const;
        Text *setSelectable(bool lcdRender);
        void styleUpdated() override;
        YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) override;
        void layoutUpdated() override;
        void draw(SkCanvas *canvas) override;
    protected:
        bool                                                           _selectable{true};
        int                                                            _selectBegin{-1};
        int                                                            _selectEnd{-1};
        TextBox                                                        _textBox{};
        std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob *)>> _blob{nullptr};
        Div::MouseSlot                                                 _onMouseMove{nullptr};
        SkPaint                                                        _selectionPaint{};
        SkPaint                                                        _selectionBackgroundPaint{};
    };
}
