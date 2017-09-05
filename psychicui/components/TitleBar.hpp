#pragma once

#include <memory>
#include "Label.hpp"
#include "../Component.hpp"

namespace psychicui {
    class TitleBar: public Component {
    public:
    TitleBar();
    protected:
        std::shared_ptr<Label> _label{};
        void addedToRender() override;
        void onMouseUp(int mouseX, int mouseY, int button, int modifiers) override;
        void onMouseDown(int mouseX, int mouseY, int button, int modifiers) override;
    };
}

