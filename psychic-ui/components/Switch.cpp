#include <algorithm>
#include <cstdlib>
#include <functional>
#include <string>
#include <utility>
#include "Switch.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/psychic-ui.hpp"

namespace psychic_ui {

    Switch::Switch(std::string label, std::function<void(bool)> onChangeCallback) :
        Component(),
        _label(std::move(label)) {
        tag("Switch");

        _mouseChildren = false;
        _focusEnabled  = true; // Tab reaches it; Space or Enter flips it

        keyPressed.subscribe(
            [this](const Key key, const Mod /*modifiers*/) {
                if (_enabled && (key == Key::SPACE || key == Key::ENTER)) {
                    updateChecked(!_checked);
                }
            }
        );

        if (onChangeCallback) {
            changed.subscribe(std::move(onChangeCallback));
        }

        onMousePress(
            [this](const MouseEvent &event) {
                if (!_enabled || event.button != MouseButton::LEFT || !_skin) {
                    return;
                }
                _pressed  = true;
                _dragging = false;
                _pressX   = event.windowX;
                _pressY   = event.windowY;
                int x = 0;
                int y = 0;
                _skin->globalToLocal(x, y, static_cast<int>(_pressX), static_cast<int>(_pressY));
                _pressPosition = _skin->positionAt(x, y);
                _onMouseMove   = window()->mouseMoved.subscribe(
                    [this](const MouseEvent &move) {
                        dragTo(move.windowX, move.windowY);
                    }
                );
            }
        );

        // Clicked comes before the release: a click that didn't drag flips it
        clicked.subscribe(
            [this]() {
                if (_enabled && !_dragging) {
                    updateChecked(!_checked);
                }
            }
        );

        // Wherever the button is released, a drag lands on the thumb's nearer side
        onMouseRelease(
            [this](const MouseEvent &event) {
                if (event.button == MouseButton::LEFT) {
                    endDrag();
                }
            }
        );
    }

    const std::string &Switch::label() const {
        return _label;
    }

    void Switch::updateLabel(const std::string &label) {
        _label = label;
        if (_skin) {
            _skin->setLabel(_label);
        }
    }

    bool Switch::checked() const {
        return _checked;
    }

    void Switch::updateChecked(const bool checked) {
        if (_checked != checked) {
            _checked = checked;
            if (_checked) {
                addClassName("checked");
            } else {
                removeClassName("checked");
            }
            invalidateStyle();
            if (_skin) {
                _skin->setThumbPosition(thumbPosition());
            }
            changed.emit(_checked);
        }
    }

    float Switch::thumbPosition() const {
        if (_dragging) {
            return _dragPosition;
        }
        return _checked ? 1.0f : 0.0f;
    }

    bool Switch::active() const {
        return Component::active() || _checked;
    }

    void Switch::dragTo(const float windowX, const float windowY) {
        if (!_pressed || !_skin) {
            return;
        }
        if (!_dragging && std::abs(windowX - _pressX) < dragThreshold && std::abs(windowY - _pressY) < dragThreshold) {
            return;
        }
        if (!_dragging) {
            _dragging = true;
            addClassName("dragging");
        }
        // The thumb follows the mouse from where it was when pressed, whatever part of the switch was pressed
        int x = 0;
        int y = 0;
        _skin->globalToLocal(x, y, static_cast<int>(windowX), static_cast<int>(windowY));
        const float start = _checked ? 1.0f : 0.0f;
        _dragPosition = std::clamp(start + _skin->positionAt(x, y) - _pressPosition, 0.0f, 1.0f);
        _skin->setThumbPosition(_dragPosition);
    }

    void Switch::endDrag() {
        _onMouseMove.disconnect();
        _pressed = false;
        if (_dragging) {
            const bool checked = _dragPosition >= 0.5f;
            _dragging = false;
            removeClassName("dragging");
            if (checked != _checked) {
                updateChecked(checked);
            } else if (_skin) {
                _skin->setThumbPosition(thumbPosition()); // Back where it was
            }
        }
    }

    void Switch::skinChanged() {
        if (_skin) {
            _skin->setLabel(_label);
            _skin->setThumbPosition(thumbPosition());
        }
    }

    void Switch::removedFromRender() {
        Component::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-drag: the mouse up won't reach us
        _pressed  = false;
        _dragging = false;
        removeClassName("dragging");
    }

}
