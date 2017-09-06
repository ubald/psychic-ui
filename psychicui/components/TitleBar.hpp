#pragma once

#include <memory>
#include "Label.hpp"
#include "psychicui/Div.hpp"

namespace psychicui {
    class TitleBar: public Div {
    public:
    TitleBar();
    protected:
        std::shared_ptr<Label> _label{};
        void addedToRender() override;
        void onMouseUp(int mouseX, int mouseY, int button, int modifiers) override;
        void onMouseDown(int mouseX, int mouseY, int button, int modifiers) override;
    };
}

