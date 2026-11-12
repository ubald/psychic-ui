#include "ToolBar.hpp"

namespace psychic_ui {

    ToolBar::ToolBar() :
        Div() {
        setTag("ToolBar");
        _defaultStyle.height(48)
            .shrink(0);
    }

}
