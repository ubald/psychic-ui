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
                dragTo(static_cast<int>(event.windowX), static_cast<int>(event.windowY));
                _onMouseMove = window()->mouseMoved.subscribe(
                    [this](const MouseEvent &move) {
                        dragTo(static_cast<int>(move.windowX), static_cast<int>(move.windowY));
                    }
                );
            }
        );

        onMouseRelease(
            [this] {
                _onMouseMove.disconnect();
            }
        );
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
        _onMouseMove.disconnect(); // Removed mid-drag: the mouse up won't reach us
    }
}
