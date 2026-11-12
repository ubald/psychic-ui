#include "ToolBar.hpp"

namespace psychic_ui {

    ToolBar::ToolBar() :
        Div() {
        setTag("ToolBar");
        _defaultStyle
            ->set(height, 48)
            ->set(shrink, 0)
            ->set(justifyContent, "stretch");
    }

}
