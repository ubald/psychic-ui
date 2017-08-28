#pragma once

#include <SkCanvas.h>
#include <SkPaint.h>
#include "psychicui.hpp"
#include "Widget.hpp"
#include "Label.hpp"

namespace psychicui {
    class Button : public Widget {
    public:
        Button(const std::string &label = "Untitled");
        std::string label();
        void setLabel(std::string label);
        void draw(SkCanvas *canvas) override;
    protected:
        std::shared_ptr<Label> _label{nullptr};
    };
}

