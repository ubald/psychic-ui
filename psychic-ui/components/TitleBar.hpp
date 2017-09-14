#pragma once

#include <memory>
#include "Label.hpp"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    class TitleBar: public Div {
    public:
    TitleBar();
    protected:
        std::shared_ptr<Label> _label{};
        void addedToRender() override;
    };
}

