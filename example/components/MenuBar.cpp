#include "MenuBar.hpp"
#include <psychicui/Button.hpp>

namespace psychicui {

    MenuBar::MenuBar() :
        Component() {
        setTag("MenuBar");
        //setHeight(24);
        //_style->set(justifyContent, "stretch");

        add(std::make_shared<Button>("File"));
        add(std::make_shared<Button>("Edit"));
        add(std::make_shared<Button>("View"));
        add(std::make_shared<Button>("Window"));
        add(std::make_shared<Button>("Help"));
    }
    
}