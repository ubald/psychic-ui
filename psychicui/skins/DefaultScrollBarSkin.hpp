#pragma once

#include "psychicui/Skin.hpp"
#include "psychicui/components/ScrollBar.hpp"

namespace psychicui {
    class DefaultScrollBarSkin : public ScrollBarSkin {
    public:
        DefaultScrollBarSkin();
    protected:
        std::shared_ptr<Div> _track{nullptr};
        std::shared_ptr<Div> _thumb{nullptr};
        void styleUpdated() override;
        void updateScrollBar(bool enabled, float scrollPosition, float contentRatio) override;
//        void draw(SkCanvas * canvas);
    };
}

