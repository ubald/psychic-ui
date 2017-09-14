#pragma once

#include <SkCanvas.h>
#include "Div.hpp"

namespace psychic_ui {
    class Shape : public Div {
    public:
        Shape() = delete;
        Shape(std::function<void(Shape *, SkCanvas *)> drawFunc);
    protected:
        /**
         * Override this method to draw a custom shape
         * Otherwise, if the class is used directly, a drawFunc
         * must be provided as a draw callback.
         */
        void draw(SkCanvas *canvas) override;
        std::function<void(Shape *, SkCanvas *)> _drawFunc{};
    };
}

