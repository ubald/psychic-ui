#include "Shape.hpp"

#include <utility>

namespace psychic_ui {

    Shape::Shape(std::function<void(Shape *, SkCanvas *)> drawFunc) :
        _drawFunc(std::move(drawFunc)) {
        tag("Shape");
    }

    void Shape::draw(SkCanvas *canvas) {
        if (_drawFunc) {
            // Allow drawing from 0,0 in callback
            canvas->save();
            canvas->translate(static_cast<float>(_x), static_cast<float>(_y));
            _drawFunc(this, canvas);
            canvas->restore();
        }
    }

}
