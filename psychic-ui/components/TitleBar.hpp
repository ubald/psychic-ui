#pragma once

#include <memory>
#include "Label.hpp"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    class TitleBar: public Div {
    public:
    TitleBar();
    protected:
        Label *_label{nullptr}; // A child, which we own
        void addedToRender() override;
    };
}

