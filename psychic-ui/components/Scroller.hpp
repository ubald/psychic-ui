#pragma once

#include <memory>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/Div.hpp>

namespace psychic_ui {
    class Scroller: public Div {
    public:
        Scroller();
        Scroller(std::shared_ptr<Div> viewport);
    protected:
        std::shared_ptr<Div> _viewport{};
        std::shared_ptr<ScrollBar> _horizontalScrollBar{};
        std::shared_ptr<ScrollBar> _verticalScrollBar{};
    };
}

