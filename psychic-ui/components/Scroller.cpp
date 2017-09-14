#include "Scroller.hpp"

#include <utility>

namespace psychic_ui {

    Scroller::Scroller() : Scroller(std::make_shared<Div>()) {}

    Scroller::Scroller(std::shared_ptr<Div> viewport) :
        Div(),
        _viewport(std::move(viewport)) {
        _viewport->style()
                 ->set(widthPercent, 1.0f)
                 ->set(heightPercent, 1.0f)
                 ->set(overflow, "scroll");

        auto container = add<Div>();
        container->style()
                 ->set(direction, "row")
                 ->set(widthPercent, 1.0f)
                 ->set(heightPercent, 1.0f);
        container->add(_viewport);

        _verticalScrollBar   = container->add<ScrollBar>(_viewport);
        _horizontalScrollBar = add<ScrollBar>(_viewport, ScrollDirection::Horizontal);
    }
}
