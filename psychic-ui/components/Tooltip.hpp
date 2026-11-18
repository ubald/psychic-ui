#pragma once

#include <string>
#include "psychic-ui/Popup.hpp"
#include "Label.hpp"

namespace psychic_ui {

    /**
     * @class Tooltip
     *
     * A short label in a popup beside its anchor, above it by default (below when there's no room), with an arrow
     * pointing at the anchor (Pivotal's). It takes no input and leaves the focus where it is. The window shows one for
     * a div's `tooltip` text (see `Div::tooltip`, `Window::tooltipDelay`); open one by hand with
     * `overlay.open<Tooltip>("Text").anchor(div)`.
     */
    class Tooltip : public Popup {
    public:
        explicit Tooltip(const std::string &text = "");

        const std::string &text() const { return _label->text(); }

        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<Tooltip &>(self)._label->text(text);
            return self;
        }

        /**
         * How far the arrow reaches out of the box
         */
        static constexpr float arrowSize = 5.0f;

    protected:
        Label *_label{nullptr}; // The `.text` part, a child: our child list owns it

        void draw(SkCanvas *canvas) override;
    };
}
