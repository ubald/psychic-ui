#pragma once

#include "psychic-ui/components/TextInput.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every text input skin shares: the `.text` part, the view its field drives, and the `.placeholder` part
     * under it, out of the layout, while the field is empty; both are children, which its child list owns. A skin
     * set's text input only paints its chrome, behind the text.
     */
    class TextInputSkinBase : public TextInputSkin {
    public:
        TextInputSkinBase();

        Text *textView() override;
        void setPlaceholder(const std::string &placeholder, bool shown) override;

    protected:
        Label *_placeholder{nullptr}; // The `.placeholder` part
        Text  *_text{nullptr};        // The `.text` part

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its text. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
