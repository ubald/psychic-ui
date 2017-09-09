#include "Shape.hpp"

namespace psychicui {

    Shape::Shape(std::function<void(Shape *, SkCanvas *)> drawFunc) :
        _drawFunc(drawFunc) {}

    void Shape::draw(SkCanvas *canvas) {
        if (_drawFunc) {
            _drawFunc(this, canvas);
        }
    }

}