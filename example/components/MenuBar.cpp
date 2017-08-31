#include "MenuBar.hpp"
#include <psychicui/Button.hpp>

namespace psychicui {

    MenuBar::MenuBar() :
        Component() {
        setTag("MenuBar");
        setHeight(24);
        _style->set(justifyContent, "stretch");

        addChild(std::make_shared<Button>("File"));
        addChild(std::make_shared<Button>("Edit"));
        addChild(std::make_shared<Button>("View"));
        addChild(std::make_shared<Button>("Window"));
        addChild(std::make_shared<Button>("Help"));
    }

    void MenuBar::styleUpdated()  {
        Component::styleUpdated();
        _parent->computedStyle()->trace();
    }
}