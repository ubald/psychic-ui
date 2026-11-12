#include "Shape.hpp"

#include <utility>

namespace psychic_ui {

    Shape::Shape(std::function<void(Shape *, SkCanvas *)> drawFunc) :
        _drawFunc(std::move(drawFunc)) {
        setTag("Shape");
    }

    void Shape::draw(SkCanvas *canvas) {
        if (_drawFunc) {
            // Allow drawing from 0,0 in callback
            canvas->save();
            canvas->translate(_x, _y);
            _drawFunc(this, canvas);
            canvas->restore();
        }
    }

}
