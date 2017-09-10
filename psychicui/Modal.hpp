#pragma once

#include <memory>
#include <vector>
#include "Div.hpp"

namespace psychicui {
    class Modal : public Div {
    public:
        Modal();
        /**
         * Allows a div to receive mouse input even though a modal is present.
         * This is used to only allow interaction with the menu bar when the menu is opened.
         */
        void addMouseEnabledDiv(std::shared_ptr<Div> mouseEnabledDiv);

        /**
         * Removes a div from receiving mouse input while the modal is active.
         */
        void removeMouseEnabledDiv(std::shared_ptr<Div> mouseEnabledDiv);
    protected:
        /**
         * List of divs that will still receive mouse input when the modal is active
         */
        std::vector<std::weak_ptr<Div>>allowedMouseChildren{};

        // Overridden Mouse Handlers
         MouseEventStatus mouseMoved(int mouseX, int mouseY, int button, int modifiers, bool handled) override;
         MouseEventStatus mouseButton(int mouseX, int mouseY, int button, bool down, int modifiers) override;
         MouseEventStatus mouseDown(int mouseX, int mouseY, int button, int modifiers) override;
         MouseEventStatus mouseUp(int mouseX, int mouseY, int button, int modifiers) override;
         MouseEventStatus click(int mouseX, int mouseY, int button, int modifiers) override;
         MouseEventStatus mouseScrolled(int mouseX, int mouseY, double scrollX, double scrollY) override;
    };
}

