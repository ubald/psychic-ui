#include "Modal.hpp"

namespace psychic_ui {

    Modal::Modal() : Div() {
        setTag("Modal");
        _inlineStyle.position(Position::Absolute)
            .width(100_pct)
            .height(100_pct)
            .overflow(Overflow::Hidden);
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

    MouseEventStatus Modal::handleMouseMoved(const int mouseX, const int mouseY, const int buttons, const Mod modifiers, bool handled) {
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
                auto res = child->handleMouseMoved(mx, my, buttons, modifiers, handled);
                if (res != Out) {
                    handled = true;
                    if (ret != Handled) {
                        ret = res;
                    }
                }
            }
        }

        auto res = Div::handleMouseMoved(mouseX, mouseY, buttons, modifiers, handled);
        if ( res != Out && ret != Handled) {
            ret = res;
        }

        return ret;
    }

    MouseEventStatus Modal::handleMouseButton(const int mouseX, const int mouseY, const MouseButton button, const bool down, const Mod modifiers) {
        MODAL_MOUSE_EVENT_PROXY(handleMouseButton, button, down, modifiers)
    }

    MouseEventStatus Modal::handleMouseDown(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
        MODAL_MOUSE_EVENT_PROXY(handleMouseDown, button, modifiers)
    }

    MouseEventStatus Modal::handleMouseUp(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
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
                auto res = child->handleMouseUp(mx, my, button, modifiers);
                if (res != Out && ret != Handled) {
                    ret = res;
                }
            }
        }

        if (ret == Out) {
            ret = Div::handleMouseUp(mouseX, mouseY, button, modifiers);
        }

        return ret;
    }

    MouseEventStatus Modal::handleClick(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
        MODAL_MOUSE_EVENT_PROXY(handleClick, button, modifiers)
    }

    MouseEventStatus Modal::handleDoubleClick(const int mouseX, const int mouseY, const unsigned int clickCount, const Mod modifiers) {
        MODAL_MOUSE_EVENT_PROXY(handleDoubleClick, clickCount, modifiers)
    }

    MouseEventStatus Modal::handleMouseScrolled(const int mouseX, const int mouseY, const double scrollX, const double scrollY) {
        MODAL_MOUSE_EVENT_PROXY(handleMouseScrolled, scrollX, scrollY)
    }
}
