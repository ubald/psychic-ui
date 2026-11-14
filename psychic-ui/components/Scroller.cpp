#include "Scroller.hpp"

namespace psychic_ui {

    Scroller::Scroller(std::shared_ptr<Div> content) :
        Div(),
        _content(std::move(content)),
        _viewport(std::make_shared<Div>()) {
        tag("Scroller");
        _viewport->add(_content);
        _viewport->width(100_pct)
                 .height(100_pct)
                 .overflow(Overflow::Scroll);

        auto &container = add<Div>()
            .direction(Direction::Row)
            .width(100_pct)
            .height(100_pct);
        container.add(_viewport);

        _verticalScrollBar   = &container.add<ScrollBar>(*_viewport, ScrollDirection::Vertical);
        _horizontalScrollBar = &add<ScrollBar>(*_viewport, ScrollDirection::Horizontal);
    }

    Div *Scroller::viewport() {
        return _viewport.get();
    }
}
