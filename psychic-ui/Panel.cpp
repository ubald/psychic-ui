#include <algorithm>
#include "Panel.hpp"

namespace psychic_ui {

    Panel::Panel(const std::string &title) :
        Div(),
        _title(title),
        _buttonPanel(nullptr),
        _modal(false),
        _drag(false) {
        setTag("Panel");
        YGNodeStyleSetPositionType(_yogaNode, YGPositionTypeAbsolute);
    }

    std::shared_ptr<Panel> Panel::panel() {
        return std::dynamic_pointer_cast<Panel>(shared_from_this());
    }

    void Panel::draw(SkCanvas *canvas) {
        Div::draw(canvas);
    }
}
