#include "DefaultScrollBarSkin.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    DefaultScrollBarSkin::DefaultScrollBarSkin() :
        ScrollBarSkin() {
        tag("DefaultScrollBarSkin");

        _track = &add<Div>();
        _track->addClassName("track")
              .position(Position::Absolute)
              .width(100_pct)
              .height(100_pct);


        _thumb = &add<Div>();
        _thumb->addClassName("thumb")
              .position(Position::Absolute);

        _thumb->onMouseDown(
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

        _thumb->onMouseUp(
            [this](const int /*mouseX*/, const int /*mouseY*/, MouseButton /*button*/, Mod /*modifiers*/) {
                _onMouseMove.disconnect();
            }
        );
    }

    void DefaultScrollBarSkin::added() {
        ScrollBarSkin::added();
        setupDirection();
    }

    void DefaultScrollBarSkin::removedFromRender() {
        ScrollBarSkin::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-drag: the mouse up won't reach us
    }

    void DefaultScrollBarSkin::styleUpdated() {
        ScrollBarSkin::styleUpdated();
        if (component() && component()->scrollDirection() != _direction) {
            _direction = component()->scrollDirection();
            setupDirection();
        }
    }

    void DefaultScrollBarSkin::setupDirection() {
        if (_direction == ScrollDirection::Vertical) {
            _thumb->left(0.f)
                .right(0.f)
                .top(0)
                .bottom(Auto);
        } else {
            _thumb->left(0)
                .right(Auto)
                .top(0.f)
                .bottom(0.f);
        }
    }

    void DefaultScrollBarSkin::updateScrollBar(bool enabled, float scrollPosition, float contentRatio) {
        this->enabled(enabled);
        if (component()->scrollDirection() == ScrollDirection::Vertical) {
            _thumb->top((int) (scrollPosition * (float) (_track->getHeight() - _thumb->getHeight())))
                  .height(Length::percent(contentRatio * 100.0f));
        } else {
            _thumb->left((int) (scrollPosition * (float) (_track->getWidth() - _thumb->getWidth())))
                  .width(Length::percent(contentRatio * 100.0f));
        }

    }

//    void DefaultScrollBarSkin::draw(SkCanvas *canvas) {
//        SkPaint paint;
//        paint.setAntiAlias(_computedStyle.antiAlias());
//        if (_computedStyle.has(Property::opacity)) {
//            paint.setAlpha((unsigned int) (_computedStyle.opacity() * 255.f));
//        }
//        paint.setStyle(SkPaint::kFill_Style);
//        paint.setColor(_computedStyle.backgroundColor());
//        canvas->drawRect(_rect, paint);
//
//        paint.setColor(_computedStyle.color());
//
//        int minSize = 24;
//        SkRect rect;
//        if (component()->flexDirection() == Horizontal) {
//            int avail = _width - minSize;
//            int w = ( component()->div()->getHeight() * avail ) / (component()->div()->boundsBottom() - component()->div()->boundsTop());
//            int p = component()->div()->scro
//            rect.set( , , _y, _y + _height);
//        }
//    }
}
