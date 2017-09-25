#pragma once

#include <string>
#include "psychic-ui/utils/TextBox.hpp"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    class Text: public Div {
    public:
        explicit Text(const std::string &text = "");
        const std::string &getText() const;
        void setText(const std::string &text);
        void reflow();
    protected:
        const std::string *_text{};
        SkPaint _textPaint{};
        TextBox _textBox{};
        std::unique_ptr<SkTextBlob, std::function<void(SkTextBlob*)>> _blob{nullptr};
    };
}
