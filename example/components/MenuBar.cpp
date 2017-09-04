#include "MenuBar.hpp"
#include <psychicui/components/Button.hpp>

namespace psychicui {

    MenuBar::MenuBar() :
        Component() {
        setTag("MenuBar");
        _defaults
            ->set(shrink, 0)
            ->set(justifyContent, "stretch");

        add(std::make_shared<Button>("File"));
        add(std::make_shared<Button>("Edit"));
        add(std::make_shared<Button>("View"));
        add(std::make_shared<Button>("Window"));
        add(std::make_shared<Button>("Help"));
    }

}