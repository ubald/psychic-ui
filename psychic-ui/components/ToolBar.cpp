#include "ToolBar.hpp"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {

    ToolBar::ToolBar() :
        Div() {
        tag("ToolBar");
        _defaultStyle.height(48)
            .shrink(0);
    }

}
