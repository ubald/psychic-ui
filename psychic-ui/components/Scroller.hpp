#pragma once

#include <memory>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/Div.hpp>

namespace psychic_ui {
    class Scroller : public Div {
    public:
        explicit Scroller(std::shared_ptr<Div> content);
        Div *viewport();
    protected:
        std::shared_ptr<Div>       _content{nullptr};
        std::shared_ptr<Div>       _viewport{nullptr};
        ScrollBar            *_horizontalScrollBar{nullptr}; // Children, which we own
        ScrollBar            *_verticalScrollBar{nullptr};
    };
}

