#pragma once

#include "psychic-ui/components/TextInput.hpp"
#include "psychic-ui/components/TextArea.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every text input skin shares: it shows the component's text display (a `Text`, which the component
     * owns, so that it keeps its text, selection and caret when the skin changes), and the `.placeholder` part
     * under it, out of the layout, while the text is empty. A skin set's text input only paints its chrome, behind
     * the text.
     */
    class TextInputSkinBase : public TextInputSkin {
    public:
        TextInputSkinBase();

        void setText(const std::string &text) override;
        void setPlaceholder(const std::string &placeholder) override;

    protected:
        Label *_placeholder{nullptr}; // The `.placeholder` part, a child, which we own


        void addedToComponent() override;
        void removedFromComponent() override;
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its text. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };

    /**
     * What every text area skin shares: it shows the component's scroller, around its text display, both owned by
     * the component. A skin set's text area only paints its chrome, behind them.
     */
    class TextAreaSkinBase : public TextAreaSkin {
    public:
        TextAreaSkinBase();

    protected:
        void addedToComponent() override;
        void removedFromComponent() override;
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its text. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
