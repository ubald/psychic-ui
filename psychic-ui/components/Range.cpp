#include <algorithm>
#include "Range.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    namespace internal {

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
                [this](const int mouseX, const int mouseY, MouseButton /*button*/, Mod /*modifiers*/) {
                    int windowX = 0;
                    int windowY = 0;
                    localToGlobal(windowX, windowY, mouseX, mouseY);
                    dragTo(windowX, windowY);
                    _onMouseMove = window()->mouseMoved.subscribe(
                        [this](const int mouseX, const int mouseY, int /*buttons*/, Mod /*modifiers*/) {
                            dragTo(mouseX, mouseY);
                        }
                    );
                }
            );

            onMouseRelease(
                [this](const int /*mouseX*/, const int /*mouseY*/, MouseButton /*button*/, Mod /*modifiers*/) {
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

}
