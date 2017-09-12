#include "DefaultScrollBarSkin.hpp"

namespace psychicui {

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
    }

    void DefaultScrollBarSkin::styleUpdated() {
        ScrollBarSkin::styleUpdated();

        if (component()->direction() == Vertical) {
            // TODO: This creates another invalidation cycle, fix
            addClassName("vertical");
            removeClassName("horizontal");
            _thumb
                ->style()
                ->set(left, 0.f)
                ->set(right, 0.f);
        } else {
            // TODO: This creates another invalidation cycle, fix
            addClassName("horizontal");
            removeClassName("vertical");
            _thumb
                ->style()
                ->set(top, 0.f)
                ->set(bottom, 0.f);
        }
    }

    void DefaultScrollBarSkin::updateScrollBar(bool enabled, float scrollPosition, float contentRatio) {
        setEnabled(enabled);
        if (component()->direction() == Vertical) {
            std::cout << scrollPosition * (float) (_height - _thumb->getHeight()) << std::endl;
            _thumb->style()
                  ->set(top, (int)(scrollPosition * (float) (_height - _thumb->getHeight())))
                  ->set(heightPercent, contentRatio);
        } else {
            _thumb->style()
                  ->set(left, (int)(scrollPosition * (float) (_width - _thumb->getWidth())))
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
