#include "Scroller.hpp"

namespace psychic_ui {

    Scroller::Scroller(std::shared_ptr<Div> content) :
        Div(),
        _content(std::move(content)),
        _viewport(std::make_shared<Div>()) {
        _viewport->add(_content);
        _viewport->style()
                 ->set(widthPercent, 1.0f)
                 ->set(heightPercent, 1.0f)
                 ->set(overflow, "scroll");

        auto container = add<Div>();
        container->style()
                 ->set(flexDirection, "row")
                 ->set(widthPercent, 1.0f)
                 ->set(heightPercent, 1.0f);
        container->add(_viewport);

        _verticalScrollBar   = container->add<ScrollBar>(_viewport, ScrollDirection::Vertical);
        _horizontalScrollBar = add<ScrollBar>(_viewport, ScrollDirection::Horizontal);
    }

    Div *Scroller::viewport() {
        return _viewport.get();
    }
}
