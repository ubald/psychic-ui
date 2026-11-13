#include "Box.hpp"

namespace psychic_ui {

    Box::Box(Direction direction, Length gap) :
        Div() {
        tag("Box");
        _defaultStyle
            .direction(direction)
            .gap(gap);
    }

    HBox::HBox(Length gap) :
        Box(Direction::Row, gap) {
        tag("HBox");
    }

    VBox::VBox(Length gap) :
        Box(Direction::Column, gap) {
        tag("VBox");
    }
}
