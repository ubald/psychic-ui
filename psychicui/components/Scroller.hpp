#pragma once

#include <memory>
#include <psychicui/components/ScrollBar.hpp>
#include <psychicui/Div.hpp>

namespace psychicui {
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

