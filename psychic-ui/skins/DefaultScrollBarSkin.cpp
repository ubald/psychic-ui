#include "DefaultScrollBarSkin.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    DefaultScrollBarSkin::DefaultScrollBarSkin() :
        ScrollBarSkin() {
        setTag("DefaultScrollBarSkin");

        _track = add<Div>();
        _track->addClassName("track")
              ->style()
              ->set(position, "absolute")
              ->set(widthPercent, 1.0f)
              ->set(heightPercent, 1.0f);


        _thumb = add<Div>();
        _thumb->addClassName("thumb")
              ->style()
              ->set(position, "absolute");

        _thumb->onMouseDown(
            [this](const int mouseX, const int mouseY, int button, int modifiers) {
                if (_direction == ScrollDirection::Vertical) {
                    dragOffset = mouseY;
                } else {
                    dragOffset = mouseX;
                }
                _onMouseMove = window()->onMouseMove(
                    [this](const int mouseX, const int mouseY, int button, int modifiers) {
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
            [this](const int mouseX, const int mouseY, int button, int modifiers) {
                if (_onMouseMove) {
                    _onMouseMove->disconnect();
                    _onMouseMove = nullptr;
                }
            }
        );
    }

    void DefaultScrollBarSkin::added() {
        ScrollBarSkin::added();
        setupDirection();
    }

    void DefaultScrollBarSkin::styleUpdated() {
        ScrollBarSkin::styleUpdated();
        if (component() && component()->direction() != _direction) {
            _direction = component()->direction();
            setupDirection();
        }
    }

    void DefaultScrollBarSkin::setupDirection() {
        if (_direction == Vertical) {
            _thumb
                ->style()
                ->set(left, 0.f)
                ->set(right, 0.f)
                ->set(top, 0)
                ->set(bottom, NAN);
        } else {
            _thumb
                ->style()
                ->set(left, 0)
                ->set(right, NAN)
                ->set(top, 0.f)
                ->set(bottom, 0.f);
        }
    }

    void DefaultScrollBarSkin::updateScrollBar(bool enabled, float scrollPosition, float contentRatio) {
        setEnabled(enabled);
        if (component()->direction() == Vertical) {
            _thumb->style()
                  ->set(top, (int) (scrollPosition * (float) (_track->getHeight() - _thumb->getHeight())))
                  ->set(heightPercent, contentRatio);
        } else {
            _thumb->style()
                  ->set(left, (int) (scrollPosition * (float) (_track->getWidth() - _thumb->getWidth())))
                  ->set(widthPercent, contentRatio);
        }

    }

//    void DefaultScrollBarSkin::draw(SkCanvas *canvas) {
//        SkPaint paint;
//        paint.setAntiAlias(_computedStyle->get(antiAlias));
//        if (_computedStyle->has(opacity)) {
//            paint.setAlpha((unsigned int) (_computedStyle->get(opacity) * 255.f));
//        }
//        paint.setStyle(SkPaint::kFill_Style);
//        paint.setColor(_computedStyle->get(backgroundColor));
//        canvas->drawRect(_rect, paint);
//
//        paint.setColor(_computedStyle->get(color));
//
//        int minSize = 24;
//        SkRect rect;
//        if (component()->direction() == Horizontal) {
//            int avail = _width - minSize;
//            int w = ( component()->div()->getHeight() * avail ) / (component()->div()->boundsBottom() - component()->div()->boundsTop());
//            int p = component()->div()->scro
//            rect.set( , , _y, _y + _height);
//        }
//    }
}
