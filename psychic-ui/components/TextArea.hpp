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
            tag("TextAreaSkin");
        }

        /**
         * Show the component's scroller (`TextArea::textScroller()`), which it owns, among the skin's children; take
         * it out again when the skin is replaced
         */
        void addedToComponent() override = 0;
        void removedFromComponent() override = 0;
    };

    class TextArea : public Component<TextAreaSkin> {
    public:
        explicit TextArea(std::string text = "");
        const std::string &text() const;

        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<TextArea &>(self).updateText(text);
            return self;
        }

        std::shared_ptr<Text> textDisplay();
        std::shared_ptr<Scroller> textScroller();

        /**
         * The text display: Tab, or a press beside the text, lets one type
         */
        Div *focusDelegate() override;

        /**
         * The text changed, with the new text
         */
        Signal<std::string> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        void updateText(const std::string &text);

        std::string               _text{};
        std::shared_ptr<Text>     _textDisplay{nullptr};
        std::shared_ptr<Scroller> _textScroller{nullptr};
    };
}
