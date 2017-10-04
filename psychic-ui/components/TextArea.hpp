#pragma once

#include <memory>
#include "psychic-ui/Component.hpp"
#include "psychic-ui/components/Scroller.hpp"
#include "psychic-ui/components/Text.hpp"

namespace psychic_ui {
    class TextArea;

    class TextAreaSkin : public Skin<TextArea> {
    public:
        TextAreaSkin() : Skin<TextArea>() {
            setTag("TextAreaSkin");
        }

        virtual void setText(const std::string &/*text*/) {};
    };

    class TextArea : public Component<TextAreaSkin> {
    public:
        explicit TextArea(const std::string &text = "");
        const std::string &getText() const;
        TextArea *setText(const std::string &text);
        std::shared_ptr<Text> textDisplay();
        std::shared_ptr<Scroller> textScroller();
        Signal<std::string> onChange{};

    protected:
        std::string               _text{};
        std::shared_ptr<Text>     _textDisplay{nullptr};
        std::shared_ptr<Scroller> _textScroller{nullptr};
    };
}
