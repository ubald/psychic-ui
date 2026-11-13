#include <algorithm>
#include "Range.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    namespace internal {

        RangeBase::RangeBase() :
            Component<RangeSkin>() {
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
