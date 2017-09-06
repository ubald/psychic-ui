#include "ToolBar.hpp"
#include <psychicui/components/Button.hpp>
#include <psychicui/components/Spacer.hpp>

namespace psychicui {

    ToolBar::ToolBar() :
        Div() {
        setTag("ToolBar");
        setHeight(48);
        _defaultStyle
            ->set(shrink, 0)
            ->set(justifyContent, "stretch");

        add(std::make_shared<Button>("New"));
        add(std::make_shared<Button>("Start"));
        add(std::make_shared<Button>("Stop"));

        add(std::make_shared<Spacer>());

        #ifdef DEBUG_LAYOUT
        auto d = add<Button>("Debug Layout")
            ->setToggle(true)
            ->onChange([](bool selected) { Div::debugLayout = selected; });
        #endif
        add(std::make_shared<Button>("Help"));
    }

}