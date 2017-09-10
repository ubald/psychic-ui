#include "Modal.hpp"

namespace psychicui {

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

    MouseEventStatus Modal::mouseMoved(const int mouseX, const int mouseY, const int button, const int modifiers, bool handled) {
        if (!_visible) {
            return Out;
        }

        int gx = 0;
        int gy = 0;
        getLocalToGlobal(gx, gy, mouseX, mouseY);

        MouseEventStatus ret = Out;

        for (auto &weakChild: allowedMouseChildren) {
            if (auto child = weakChild.lock()) {
                int mx = mouseX;
                int my = mouseY;
                if (child->parent()) {
                    child->getGlobalToLocal(mx, my, gx, gy);
                }
                auto res = child->mouseMoved(mx, my, button, modifiers, handled);
                if (res != Out) {
                    handled = true;
                    if (ret != Handled) {
                        ret = res;
                    }
                }
            }
        }

        auto res = Div::mouseMoved(mouseX, mouseY, button, modifiers, handled);
        if ( res != Out && ret != Handled) {
            ret = res;
        }

        return ret;
    }

    MouseEventStatus Modal::mouseButton(const int mouseX, const int mouseY, const int button, const bool down, const int modifiers) {
        if (!_visible) {
            return Out;
        }

        int gx = 0;
        int gy = 0;
        getLocalToGlobal(gx, gy, mouseX, mouseY);

        MouseEventStatus ret = Out;

        for (auto &weakChild: allowedMouseChildren) {
            if (auto child = weakChild.lock()) {
                int mx = mouseX;
                int my = mouseY;
                if (child->parent()) {
                    child->getGlobalToLocal(mx, my, gx, gy);
                }
                auto res = child->mouseButton(mx, my, button, down, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (ret == Out) {
            ret = Div::mouseButton(mouseX, mouseY, button, down, modifiers);
        }

        return ret;
    }

    MouseEventStatus Modal::mouseDown(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_visible) {
            return Out;
        }

        int gx = 0;
        int gy = 0;
        getLocalToGlobal(gx, gy, mouseX, mouseY);

        MouseEventStatus ret = Out;

        for (auto &weakChild: allowedMouseChildren) {
            if (auto child = weakChild.lock()) {
                int mx = mouseX;
                int my = mouseY;
                if (child->parent()) {
                    child->getGlobalToLocal(mx, my, gx, gy);
                }
                auto res = child->mouseDown(mx, my, button, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (ret == Out) {
            ret = Div::mouseDown(mouseX, mouseY, button, modifiers);
        }

        return ret;
    }

    MouseEventStatus Modal::mouseUp(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_visible) {
            return Out;
        }

        int gx = 0;
        int gy = 0;
        getLocalToGlobal(gx, gy, mouseX, mouseY);

        MouseEventStatus ret = Out;

        for (auto &weakChild: allowedMouseChildren) {
            if (auto child = weakChild.lock()) {
                int mx = mouseX;
                int my = mouseY;
                if (child->parent()) {
                    child->getGlobalToLocal(mx, my, gx, gy);
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

    MouseEventStatus Modal::click(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_visible) {
            return Out;
        }

        int gx = 0;
        int gy = 0;
        getLocalToGlobal(gx, gy, mouseX, mouseY);

        MouseEventStatus ret = Out;

        for (auto &weakChild: allowedMouseChildren) {
            if (auto child = weakChild.lock()) {
                int mx = mouseX;
                int my = mouseY;
                if (child->parent()) {
                    child->getGlobalToLocal(mx, my, gx, gy);
                }
                auto res = child->click(mx, my, button, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (ret == Out) {
            ret = Div::click(mouseX, mouseY, button, modifiers);
        }

        return ret;
    }

    MouseEventStatus Modal::mouseScrolled(const int mouseX, const int mouseY, const double scrollX, const double scrollY) {
        if (!_visible) {
            return Out;
        }

        int gx = 0;
        int gy = 0;
        getLocalToGlobal(gx, gy, mouseX, mouseY);

        MouseEventStatus ret = Out;

        for (auto &weakChild: allowedMouseChildren) {
            if (auto child = weakChild.lock()) {
                int mx = mouseX;
                int my = mouseY;
                if (child->parent()) {
                    child->getGlobalToLocal(mx, my, gx, gy);
                }
                auto res = child->mouseScrolled(mx, my, scrollX, scrollX);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (ret == Out) {
            ret = Div::mouseScrolled(mouseX, mouseY, scrollY, scrollY);
        }

        return ret;
    }
}
