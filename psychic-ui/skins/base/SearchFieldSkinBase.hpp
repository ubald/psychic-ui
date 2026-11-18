#pragma once

#include "psychic-ui/components/SearchField.hpp"
#include "psychic-ui/components/Icon.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every search field skin shares, its parts, in a row, all children, which its child list owns: the `.icon`
     * (a search Icon), the `.field` box holding the `.text` part (the view its field drives) and the `.placeholder`
     * part under it, out of the layout, while the field is empty, then the `.clear` button (an `x` Icon) while it
     * has text. A skin set's search field only paints its chrome, behind them.
     */
    class SearchFieldSkinBase : public SearchFieldSkin {
    public:
        SearchFieldSkinBase();

        Text *textView() override;
        void setPlaceholder(const std::string &placeholder, bool shown) override;
        void setClearable(bool clearable) override;
        bool clearAt(float x, float y) const override;

    protected:
        Icon  *_icon{nullptr};        // The `.icon` part
        Div   *_field{nullptr};       // The `.field` part, holding the text and the placeholder
        Label *_placeholder{nullptr}; // The `.placeholder` part
        Text  *_text{nullptr};        // The `.text` part
        Div   *_clear{nullptr};       // The `.clear` part

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the field behind its parts. The default is the Div's own painting.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
