#include "Popup.hpp"
#include "Overlay.hpp"

namespace psychic_ui {

    Popup::Popup() :
        Div() {
        tag("Popup");
        startingClass("entering"); // Its rules say where it enters from
        _defaultStyle.position(Position::Absolute);
    }

    void Popup::updateAnchor(Div *anchor, const int x, const int y) {
        _anchored    = true;
        _anchorIsDiv = anchor != nullptr;
        _anchor      = anchor ? anchor->weak_from_this() : std::weak_ptr<Div>{};
        _anchorX     = x;
        _anchorY     = y;
    }

    namespace {
        const char *sideClass(const Side side) {
            switch (side) {
                case Side::Bottom: return "side-bottom";
                case Side::Top: return "side-top";
                case Side::Right: return "side-right";
                case Side::Left: return "side-left";
            }
            return "side-bottom";
        }
    }

    void Popup::placed(const Side side) {
        if (_placedSide != side || !classNames().contains(sideClass(side))) {
            removeClassName(sideClass(_placedSide));
            addClassName(sideClass(side));
        }
        _placedSide = side;
    }

    bool Popup::isOpen() const {
        return dynamic_cast<const Overlay *>(_parent) != nullptr && !exiting();
    }

    void Popup::close() {
        if (auto *overlay = dynamic_cast<Overlay *>(_parent)) {
            overlay->close(*this);
        }
    }
}
