#include <algorithm>
#include "Range.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/psychic-ui.hpp"

namespace psychic_ui::internal {

    RangeBase::RangeBase() :
        Component<RangeSkin>() {
        _focusEnabled = true; // Tab reaches it; the arrows, Home and End move it

        const auto key = [this](const Key key, const Mod modifiers) {
            if (!_enabled) {
                return;
            }
            const float step  = modifiers.shift ? 0.1f : 0.01f;
            const float value = getLinearPercentage();
            switch (key) {
                case Key::RIGHT:
                case Key::UP:
                    setLinearPercentage(std::clamp(value + step, 0.0f, 1.0f));
                    break;
                case Key::LEFT:
                case Key::DOWN:
                    setLinearPercentage(std::clamp(value - step, 0.0f, 1.0f));
                    break;
                case Key::HOME:
                    setLinearPercentage(0.0f);
                    break;
                case Key::END:
                    setLinearPercentage(1.0f);
                    break;
                default:
                    break;
            }
        };
        keyPressed.subscribe(key);
        keyRepeated.subscribe(key);

        onMousePress(
            [this](const MouseEvent &event) {
                if (!_enabled || !_skin || event.button != MouseButton::LEFT || _onMouseMove.connected()) {
                    return;
                }
                _onMouseMove = window()->mouseMoved.subscribe(
                    [this](const MouseEvent &move) {
                        dragTo(static_cast<int>(move.windowX), static_cast<int>(move.windowY));
                    }
                );
                const auto keepAlive = weak_from_this().lock(); // A handler may remove this range from its parent
                dragStarted.emit();
                if (_onMouseMove.connected()) { // Unless a handler ended it
                    dragTo(static_cast<int>(event.windowX), static_cast<int>(event.windowY));
                }
            }
        );

        onMouseRelease(
            [this](const MouseEvent &event) {
                if (event.button == MouseButton::LEFT) {
                    endDrag();
                }
            }
        );
    }

    void RangeBase::endDrag() {
        if (!_onMouseMove.connected()) {
            return;
        }
        _onMouseMove.disconnect();
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this range from its parent
        dragEnded.emit();
    }

    void RangeBase::dragTo(const int windowX, const int windowY) {
        if (!_skin || !_enabled) {
            return;
        }
        int x = 0;
        int y = 0;
        _skin->globalToLocal(x, y, windowX, windowY);
        setLinearPercentage(std::clamp(_skin->valueAt(x, y), 0.0f, 1.0f));
    }

    void RangeBase::removedFromRender() {
        Component<RangeSkin>::removedFromRender();
        endDrag(); // Removed mid-drag: the mouse up won't reach us
    }
}
