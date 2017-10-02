#pragma once

#include <memory>
#include <vector>
#include "Div.hpp"

namespace psychic_ui {

    /**
     * @class Modal
     *
     * The Modal class is used to display modal menus and dialogs on top of the application.
     * It overrides most of the mouse event methods in order to allow events to reach some
     * mouse enabled divs under the modal. For example, the menu bar is still usable when a
     * menu is opened, or a tutorial modal could let the user only access certain regions of
     * the application.
     */
    class Modal : public Div {
    public:
        Modal();
        /**
         * Allows a div to receive mouse input even though a modal is present.
         * This is used to only allow interaction with the menu bar when the menu is opened.
         *
         * @param mouseEnabledDiv Div to keep accessible while the modal is opened
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
         MouseEventStatus mouseMoved(int mouseX, int mouseY, int buttons, Mod modifiers, bool handled) override;
         MouseEventStatus mouseButton(int mouseX, int mouseY, MouseButton button, bool down, Mod modifiers) override;
         MouseEventStatus mouseDown(int mouseX, int mouseY, MouseButton button, Mod modifiers) override;
         MouseEventStatus mouseUp(int mouseX, int mouseY, MouseButton button, Mod modifiers) override;
         MouseEventStatus click(int mouseX, int mouseY, MouseButton button, Mod modifiers) override;
         MouseEventStatus doubleClick(int mouseX, int mouseY, unsigned int clickCount, Mod modifiers) override;
         MouseEventStatus mouseScrolled(int mouseX, int mouseY, double scrollX, double scrollY) override;
    };
}

