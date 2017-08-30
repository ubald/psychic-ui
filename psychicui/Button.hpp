#pragma once

#include <SkCanvas.h>
#include "psychicui.hpp"
#include "Component.hpp"
#include "Label.hpp"

namespace psychicui {
    class Button : public Component {
    public:
        Button(const std::string &label = "Untitled");
        std::string label();
        void setLabel(std::string label);
        void styleUpdated() override;
        void draw(SkCanvas *canvas) override;
    protected:
        std::shared_ptr<Label> _label{nullptr};
    };
}

