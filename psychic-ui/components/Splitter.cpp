#include <algorithm>
#include <yoga/Yoga.h>
#include "Splitter.hpp"
#include "SplitterDivider.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    Splitter::Splitter() :
        Div() {
        tag("Splitter");
        _defaultStyle.direction(Direction::Row);

        _first = &add<Div>();
        _first->addClassName("pane").addClassName("first");
        _second = &add<Div>();
        _second->addClassName("pane").addClassName("second");
        // Last, so it is on top of the panes where it reaches over them
        _divider = &add<SplitterDivider>();

        _divider->onMousePress(
            [this](const MouseEvent &event) {
                if (!_enabled || event.button != MouseButton::LEFT) {
                    return;
                }
                _pressAt       = _orientation == Orientation::Horizontal ? event.windowX : event.windowY;
                _pressPosition = position();
                _onMouseMove   = window()->mouseMoved.subscribe(
                    [this](const MouseEvent &move) {
                        // Toward the end grows the first pane: the kept one when it is the first
                        const float along = _orientation == Orientation::Horizontal ? move.windowX : move.windowY;
                        const float delta = along - _pressAt;
                        moveTo(_pressPosition + (_kept == SplitterPane::First ? delta : -delta));
                    }
                );
            }
        );

        // Wherever the button is released, the drag ends
        _divider->onMouseRelease(
            [this](const MouseEvent &event) {
                if (event.button == MouseButton::LEFT) {
                    _onMouseMove.disconnect();
                }
            }
        );

        _divider->onDoubleClick(
            [this]() {
                if (_enabled) {
                    moveTo(_defaultPosition);
                }
            }
        );

        // The arrows along the splitter move the divider toward them
        _divider->onKeyPress(
            [this](const Key key, const Mod modifiers) {
                if (!_enabled) {
                    return;
                }
                const bool horizontal = _orientation == Orientation::Horizontal;
                float      toward     = 0; // -1 toward the start, 1 toward the end
                if (key == (horizontal ? Key::LEFT : Key::UP)) {
                    toward = -1;
                } else if (key == (horizontal ? Key::RIGHT : Key::DOWN)) {
                    toward = 1;
                } else {
                    return;
                }
                const float step = toward * keyStep * (modifiers.shift ? 10.0f : 1.0f);
                moveTo(position() + (_kept == SplitterPane::First ? step : -step));
            }
        );

        place();
    }

    float Splitter::position() const {
        return fit(_position);
    }

    float Splitter::fit(const float position) const {
        const float kept  = minimum(_kept);
        if (!_laidOut) {
            return std::max(position, kept);
        }
        // The other pane's minimum gives way to the kept one's when there isn't room for both
        const float other = minimum(_kept == SplitterPane::First ? SplitterPane::Second : SplitterPane::First);
        return std::clamp(position, kept, std::max(kept, _end - _start - other));
    }

    void Splitter::moveTo(const float position) {
        const float before = this->position();
        _position = fit(position);
        place();
        if (_position != before) {
            changed.emit(_position);
        }
    }

    void Splitter::updateKept(const SplitterPane pane) {
        if (pane != _kept) {
            // The same layout, now held by the other pane
            const float shown = position();
            _kept = pane;
            if (_laidOut) {
                _position = (_end - _start) - shown;
            }
            place();
        }
    }

    void Splitter::updatePosition(const float position) {
        _position = std::max(position, 0.0f);
        place();
    }

    void Splitter::updateMinimum(const SplitterPane pane, const float minimum) {
        _minimums.at(index(pane)) = std::max(minimum, 0.0f);
        place();
    }

    void Splitter::place() {
        const float shown = position();
        Div        &kept  = pane(_kept);
        Div        &other = pane(_kept == SplitterPane::First ? SplitterPane::Second : SplitterPane::First);
        kept.basis(shown).grow(0).shrink(0);
        other.basis(0).grow(1).shrink(1);

        if (!_laidOut) {
            return;
        }
        // Centered on the boundary, from the padding box's edge as absolute insets are
        const bool  horizontal = _orientation == Orientation::Horizontal;
        const float boundary   = _kept == SplitterPane::First ? _start + shown : _end - shown;
        const float border     = YGNodeLayoutGetBorder(_yogaNode, horizontal ? YGEdgeLeft : YGEdgeTop);
        const auto  reach      = static_cast<float>(horizontal ? _divider->getWidth() : _divider->getHeight());
        const float offset     = boundary - border - (reach / 2.0f);
        if (horizontal) {
            _divider->left(offset).top(0);
        } else {
            _divider->top(offset).left(0);
        }
    }

    void Splitter::styleUpdated() {
        Div::styleUpdated();
        const Orientation orientation = _computedStyle.orientation();
        if (_oriented && orientation == _orientation) {
            return;
        }
        _oriented    = true;
        _orientation = orientation;
        _laidOut     = false; // Along the other axis: wait for the layout
        _defaultStyle.direction(orientation == Orientation::Horizontal ? Direction::Row : Direction::Column);
        _divider->orientation(orientation);
        place();
    }

    void Splitter::layoutUpdated() {
        Div::layoutUpdated();
        // The panes share the content box, whatever their sizes in this layout
        const bool   horizontal = _orientation == Orientation::Horizontal;
        const YGEdge leading    = horizontal ? YGEdgeLeft : YGEdgeTop;
        const YGEdge trailing   = horizontal ? YGEdgeRight : YGEdgeBottom;
        _start   = YGNodeLayoutGetBorder(_yogaNode, leading) + YGNodeLayoutGetPadding(_yogaNode, leading);
        _end     = (horizontal ? YGNodeLayoutGetWidth(_yogaNode) : YGNodeLayoutGetHeight(_yogaNode))
                   - YGNodeLayoutGetBorder(_yogaNode, trailing) - YGNodeLayoutGetPadding(_yogaNode, trailing);
        _laidOut = true;
        // The room changed: the panes and the divider fit it, laid out again in this frame
        place();
    }

    void Splitter::removedFromRender() {
        Div::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-drag: the mouse up won't reach us
    }

}
