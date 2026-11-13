#include "DefaultScrollBarSkin.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    DefaultScrollBarSkin::DefaultScrollBarSkin() :
        ScrollBarSkinBase() {
        tag("DefaultScrollBarSkin");

        _thumb->onMousePress(
            [this](const int mouseX, const int mouseY, MouseButton /*button*/, Mod /*modifiers*/) {
                if (_direction == ScrollDirection::Vertical) {
                    dragOffset = mouseY;
                } else {
                    dragOffset = mouseX;
                }
                _onMouseMove = window()->mouseMoved.subscribe(
                    [this](const int mouseX, const int mouseY, int /*buttons*/, Mod /*modifiers*/) {
                        int lx = 0;
                        int ly = 0;
                        _track->globalToLocal(lx, ly, mouseX, mouseY);
                        if (_direction == ScrollDirection::Vertical) {
                            component()->scrollPercentY((float) (ly - dragOffset) / (float) (_track->getHeight() - _thumb->getHeight()));
                        } else {
                            component()->scrollPercentX((float) (lx - dragOffset) / (float) (_track->getWidth() - _thumb->getWidth()));
                        }
                    }
                );
            }
        );

        _thumb->onMouseRelease(
            [this](const int /*mouseX*/, const int /*mouseY*/, MouseButton /*button*/, Mod /*modifiers*/) {
                _onMouseMove.disconnect();
            }
        );
    }

    void DefaultScrollBarSkin::removedFromRender() {
        ScrollBarSkinBase::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-drag: the mouse up won't reach us
    }
}
