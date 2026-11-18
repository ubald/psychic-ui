#pragma once

#include "psychic-ui/Shape.hpp"
#include "psychic-ui/components/CheckBox.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every check box skin shares: the `.box` part, then the `.label` part.
     * A skin set's check box only paints the box, whose style gives its colors (`CheckBox.checked .box`).
     */
    class CheckBoxSkinBase : public CheckBoxSkin {
    public:
        CheckBoxSkinBase();

        void setLabel(const std::string &label) override;

    protected:
        Shape *_box{nullptr};   // The `.box` part, a child: our child list owns it
        Label *_label{nullptr}; // The `.label` part, a child: our child list owns it

        /**
         * Nothing: the box, sized by its own style, carries the chrome
         */
        void draw(SkCanvas *canvas) final;

        /**
         * Paint the `.box` part, in its coordinates (`box` is its border box; its colors are in
         * `_box->computedStyle()`). The default fills it with its background color, strokes its border, and draws a
         * check mark in its `color` when checked.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
