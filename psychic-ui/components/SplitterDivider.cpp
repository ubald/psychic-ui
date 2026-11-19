#include "SplitterDivider.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/style/Properties.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    SplitterDivider::SplitterDivider() :
        Component() {
        tag("SplitterDivider");
        _mouseChildren = false;
        _focusEnabled  = true; // Tab reaches it; the arrows move it
        _defaultStyle.position(Position::Absolute)
                     .justify(Justify::Center);
    }

    void SplitterDivider::styleUpdated() {
        Component::styleUpdated();
        const Orientation orientation = _computedStyle.orientation();
        if (_oriented && orientation == _orientation) {
            return;
        }
        _oriented    = true;
        _orientation = orientation;

        // Across the whole boundary, `defaultReach` wide over it: a horizontal splitter's panes are side by side, so
        // its divider is a vertical strip moving along x. Its content (the skin's line) is centered across it.
        using P = Property;
        const bool horizontal = orientation == Orientation::Horizontal;
        _defaultStyle.unset(horizontal ? P::height : P::width)
                     .unset(horizontal ? P::left : P::top)
                     .unset(horizontal ? P::right : P::bottom);
        if (horizontal) {
            _defaultStyle.width(defaultReach).top(0).bottom(0).direction(Direction::Row).cursor(Cursor::HResize);
        } else {
            _defaultStyle.height(defaultReach).left(0).right(0).direction(Direction::Column).cursor(Cursor::VResize);
        }
        removeClassName(horizontal ? "vertical" : "horizontal");
        addClassName(horizontal ? "horizontal" : "vertical");
    }

}
