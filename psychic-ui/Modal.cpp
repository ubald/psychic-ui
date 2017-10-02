#include "Modal.hpp"

namespace psychic_ui {

    Modal::Modal() : Div() {
        setTag("Modal");
        style()
            ->set(position, "absolute")
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f)
            ->set(overflow, "hidden");
    }

    void Modal::addMouseEnabledDiv(std::shared_ptr<Div> mouseEnabledDiv) {
        allowedMouseChildren.push_back(mouseEnabledDiv);
    }

    void Modal::removeMouseEnabledDiv(std::shared_ptr<Div> mouseEnabledDiv) {
        allowedMouseChildren.erase(
            std::remove_if(
                allowedMouseChildren.begin(),
                allowedMouseChildren.end(),
                [mouseEnabledDiv](const auto &weakAllowedDiv) {
                    if (auto allowedDiv = weakAllowedDiv.lock()) {
                        return allowedDiv.get() == mouseEnabledDiv.get();
                    } else {
                        // Cleanup while we're at it
                        return true;
                    }
                }
            ),
            allowedMouseChildren.end()
        );
    }

    #define MODAL_MOUSE_EVENT_PROXY(event, args...)\
        if (!_visible) {\
            return Out;\
        }\
        \
        int gx = 0;\
        int gy = 0;\
        localToGlobal(gx, gy, mouseX, mouseY);\
        \
        MouseEventStatus ret = Out;\
        \
        for (auto &weakChild: allowedMouseChildren) {\
            if (auto child = weakChild.lock()) {\
                int mx = mouseX;\
                int my = mouseY;\
                if (child->parent()) {\
                    child->parent()->globalToLocal(mx, my, gx, gy);\
                }\
                auto res = child->event(mx, my, args);\
                if (res != Out) {\
                    ret = res;\
                    break;\
                }\
            }\
        }\
        \
        if (ret == Out) {\
            ret = Div::event(mouseX, mouseY, args);\
        }\
        \
        return ret;\

    MouseEventStatus Modal::mouseMoved(const int mouseX, const int mouseY, const int buttons, const Mod modifiers, bool handled) {
        if (!_visible) {
            return Out;
        }

        int gx = 0;
        int gy = 0;
        localToGlobal(gx, gy, mouseX, mouseY);

        MouseEventStatus ret = Out;

        for (auto &weakChild: allowedMouseChildren) {
            if (auto child = weakChild.lock()) {
                int mx = mouseX;
                int my = mouseY;
                if (child->parent()) {
                    child->parent()->globalToLocal(mx, my, gx, gy);
                }
                auto res = child->mouseMoved(mx, my, buttons, modifiers, handled);
                if (res != Out) {
                    handled = true;
                    if (ret != Handled) {
                        ret = res;
                    }
                }
            }
        }

        auto res = Div::mouseMoved(mouseX, mouseY, buttons, modifiers, handled);
        if ( res != Out && ret != Handled) {
            ret = res;
        }

        return ret;
    }

    MouseEventStatus Modal::mouseButton(const int mouseX, const int mouseY, const MouseButton button, const bool down, const Mod modifiers) {
        MODAL_MOUSE_EVENT_PROXY(mouseButton, button, down, modifiers)
    }

    MouseEventStatus Modal::mouseDown(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
        MODAL_MOUSE_EVENT_PROXY(mouseDown, button, modifiers)
    }

    MouseEventStatus Modal::mouseUp(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
        if (!_visible) {
            return Out;
        }

        int gx = 0;
        int gy = 0;
        localToGlobal(gx, gy, mouseX, mouseY);

        MouseEventStatus ret = Out;

        for (auto &weakChild: allowedMouseChildren) {
            if (auto child = weakChild.lock()) {
                int mx = mouseX;
                int my = mouseY;
                if (child->parent()) {
                    child->parent()->globalToLocal(mx, my, gx, gy);
                }
                auto res = child->mouseUp(mx, my, button, modifiers);
                if (res != Out && ret != Handled) {
                    ret = res;
                }
            }
        }

        if (ret == Out) {
            ret = Div::mouseUp(mouseX, mouseY, button, modifiers);
        }

        return ret;
    }

    MouseEventStatus Modal::click(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
        MODAL_MOUSE_EVENT_PROXY(click, button, modifiers)
    }

    MouseEventStatus Modal::doubleClick(const int mouseX, const int mouseY, const unsigned int clickCount, const Mod modifiers) {
        MODAL_MOUSE_EVENT_PROXY(doubleClick, clickCount, modifiers)
    }

    MouseEventStatus Modal::mouseScrolled(const int mouseX, const int mouseY, const double scrollX, const double scrollY) {
        MODAL_MOUSE_EVENT_PROXY(mouseScrolled, scrollX, scrollY)
    }
}
