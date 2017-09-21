#include "ToolBar.hpp"

namespace psychic_ui {

    ToolBar::ToolBar() :
        Div() {
        setTag("ToolBar");
        setHeight(48);
        _defaultStyle
            ->set(shrink, 0)
            ->set(justifyContent, "stretch");
    }

}
