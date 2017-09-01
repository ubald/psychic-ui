#include "ToolBar.hpp"
#include <psychicui/Button.hpp>
#include <psychicui/components/Spacer.hpp>

namespace psychicui {

    ToolBar::ToolBar() :
        Component() {
        setTag("ToolBar");
        setHeight(48);
        _style->set(justifyContent, "stretch");

        add(std::make_shared<Button>("New"));
        add(std::make_shared<Button>("Start"));
        add(std::make_shared<Button>("Stop"));

        add(std::make_shared<Spacer>());

        add(std::make_shared<Button>("Help"));
    }

}