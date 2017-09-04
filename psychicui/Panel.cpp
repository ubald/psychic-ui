#include <algorithm>
#include "Panel.hpp"

namespace psychicui {

    Panel::Panel(const std::string &title) :
        Component(),
        _title(title),
        _buttonPanel(nullptr),
        _modal(false),
        _drag(false) {
        setTag("Panel");
        YGNodeStyleSetPositionType(_yogaNode, YGPositionTypeAbsolute);
    }

    std::shared_ptr<Panel> Panel::panel() {
        return std::dynamic_pointer_cast<Panel>(shared_from_this());
    }

    void Panel::draw(SkCanvas *canvas) {
        Component::draw(canvas);
    }

    void Panel::dispose() {
//        Component *component = this;
//        while (component->setParent()) {
//            component = component->setParent();
//        }
//        ((Window *) component)->disposePanel(this);
    }

    void Panel::center() {
//        Component *component = this;
//        while (component->setParent()) {
//            component = component->setParent();
//        }
//        ((Window *) component)->centerPanel(this);
    }

    bool Panel::mouseDragEvent(const int mouseX, const int mouseY, const int dragX, const int dragY, int button, int modifiers) {
//        if (_drag && (button & (1 << GLFW_MOUSE_BUTTON_1)) != 0) {
//            _x += dragX;
//            _x = std::min(std::max(_x, 0), parent()->x() - _x);
//            _y += dragY;
//            _y = std::min(std::max(_y, 0), parent()->y() - _y);
//            return true;
//        }
        return false;
    }

    void Panel::onMouseButton(const int mouseX, const int mouseY, int button, bool down, int modifiers) {
//        Component::onMouseButton(mouseX, mouseY, button, down, modifiers);
//        if (button == GLFW_MOUSE_BUTTON_1) {
//            _drag = down && (mouseY - _y) < style()->getValue(titleBarHeight);
//        }
    }

    void Panel::refreshRelativePlacement() {

    }

}