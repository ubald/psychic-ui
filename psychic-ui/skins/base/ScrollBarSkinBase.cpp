#include "ScrollBarSkinBase.hpp"

namespace psychic_ui {

    ScrollBarSkinBase::ScrollBarSkinBase() :
        ScrollBarSkin() {
        tag("ScrollBarSkinBase");
        _track = &add<Div>();
        _track->addClassName("track")
              .position(Position::Absolute)
              .width(100_pct)
              .height(100_pct);

        _thumb = &add<Div>();
        _thumb->addClassName("thumb")
              .position(Position::Absolute);
    }

    SkRect ScrollBarSkinBase::trackRect() const {
        return SkRect::Make(SkIRect::MakeXYWH(_track->x(), _track->y(), _track->getWidth(), _track->getHeight()));
    }

    SkRect ScrollBarSkinBase::thumbRect() const {
        return SkRect::Make(SkIRect::MakeXYWH(_thumb->x(), _thumb->y(), _thumb->getWidth(), _thumb->getHeight()));
    }

    void ScrollBarSkinBase::added() {
        ScrollBarSkin::added();
        setupDirection();
    }

    void ScrollBarSkinBase::styleUpdated() {
        ScrollBarSkin::styleUpdated();
        if (component() && component()->scrollDirection() != _direction) {
            _direction = component()->scrollDirection();
            setupDirection();
        }
    }

    void ScrollBarSkinBase::setupDirection() {
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

    void ScrollBarSkinBase::updateScrollBar(bool enabled, float scrollPosition, float contentRatio) {
        this->enabled(enabled);
        if (component()->scrollDirection() == ScrollDirection::Vertical) {
            _thumb->top((int) (scrollPosition * (float) (_track->getHeight() - _thumb->getHeight())))
                  .height(Length::percent(contentRatio * 100.0f));
        } else {
            _thumb->left((int) (scrollPosition * (float) (_track->getWidth() - _thumb->getWidth())))
                  .width(Length::percent(contentRatio * 100.0f));
        }
    }
}
