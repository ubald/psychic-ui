#include <algorithm>
#include "SegmentedControl.hpp"

namespace psychic_ui {

    SegmentedControl::SegmentedControl(std::vector<Segment> segments) :
        Component() {
        tag("SegmentedControl");

        _focusEnabled = true; // Tab reaches it; the arrows, Home and End move the selection

        updateSegments(std::move(segments));

        const auto key = [this](const Key key, const Mod /*modifiers*/) {
            if (!_enabled || _segments.empty()) {
                return;
            }
            switch (key) {
                case Key::LEFT:
                case Key::UP:
                    updateSelected(_selected - 1);
                    break;
                case Key::RIGHT:
                case Key::DOWN:
                    updateSelected(_selected + 1);
                    break;
                case Key::HOME:
                    updateSelected(0);
                    break;
                case Key::END:
                    updateSelected(static_cast<int>(_segments.size()) - 1);
                    break;
                default:
                    break;
            }
        };
        keyPressed.subscribe(key);
        keyRepeated.subscribe(key);

        // The segment pressed is the one a click picks
        onMousePress(
            [this](const MouseEvent &event) {
                if (event.button != MouseButton::LEFT || !_skin) {
                    return;
                }
                int x = 0;
                int y = 0;
                _skin->globalToLocal(x, y, static_cast<int>(event.windowX), static_cast<int>(event.windowY));
                _pressed = _skin->segmentAt(x, y);
            }
        );

        clicked.subscribe(
            [this]() {
                if (_enabled && _pressed >= 0) {
                    updateSelected(_pressed);
                }
                _pressed = -1;
            }
        );
    }

    const std::vector<Segment> &SegmentedControl::segments() const {
        return _segments;
    }

    void SegmentedControl::updateSegments(std::vector<Segment> segments) {
        _segments = std::move(segments);
        const int count = static_cast<int>(_segments.size());
        const int selected = count == 0 ? -1 : (_selected >= 0 && _selected < count ? _selected : 0);
        if (_skin) {
            _skin->setSegments(_segments);
        }
        if (selected != _selected) {
            _selected = selected;
            if (_selected >= 0) {
                changed.emit(_selected);
            }
        }
        if (_skin) {
            _skin->setSelected(_selected);
        }
    }

    int SegmentedControl::selected() const {
        return _selected;
    }

    void SegmentedControl::updateSelected(const int index) {
        if (_segments.empty()) {
            return;
        }
        const int selected = std::clamp(index, 0, static_cast<int>(_segments.size()) - 1);
        if (selected != _selected) {
            _selected = selected;
            if (_skin) {
                _skin->setSelected(_selected);
            }
            changed.emit(_selected);
        }
    }

    void SegmentedControl::skinChanged() {
        if (_skin) {
            _skin->setSegments(_segments);
            _skin->setSelected(_selected);
        }
    }

}
