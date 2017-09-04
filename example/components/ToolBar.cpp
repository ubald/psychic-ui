#include "ToolBar.hpp"
#include <psychicui/components/Button.hpp>
#include <psychicui/components/Spacer.hpp>

namespace psychicui {

    ToolBar::ToolBar() :
        Component() {
        setTag("ToolBar");
        setHeight(48);
        _defaults
            ->set(shrink, 0)
            ->set(justifyContent, "stretch");

        add(std::make_shared<Button>("New"));
        add(std::make_shared<Button>("Start"));
        add(std::make_shared<Button>("Stop"));

        add(std::make_shared<Spacer>());

        #ifdef DEBUG_LAYOUT
        auto d = add<Button>("Debug Layout")
            ->setToggle(true)
            ->onChange([](bool selected) { Component::debugLayout = selected; });
        #endif
        add(std::make_shared<Button>("Help"));
    }

}