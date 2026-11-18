#include <memory>
#include <utility>
#include "ScrollBar.hpp"
#include "Scroller.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/style/Length.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    Scroller::Scroller(std::shared_ptr<Div> content) :
        Div(),
        _content(std::move(content)),
        _viewport(std::make_shared<Div>()) {
        tag("Scroller");
        _viewport->add(_content);
        _viewport->width(Full)
                 .height(Full)
                 .overflow(Overflow::Scroll);

        auto &container = add<Div>()
            .direction(Direction::Row)
            .width(Full)
            .height(Full);
        container.add(_viewport);

        _verticalScrollBar   = &container.add<ScrollBar>(*_viewport, ScrollDirection::Vertical);
        _horizontalScrollBar = &add<ScrollBar>(*_viewport, ScrollDirection::Horizontal);
    }

    Div *Scroller::viewport() {
        return _viewport.get();
    }
}
