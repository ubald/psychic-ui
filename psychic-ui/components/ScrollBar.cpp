#include "ScrollBar.hpp"

namespace psychic_ui {
    ScrollBar::ScrollBar(Div &viewport, ScrollDirection direction) :
        Component(),
        _direction(direction),
        _viewport(viewport.shared_from_this()) {
        setTag("ScrollBar");
        if (_direction == ScrollDirection::Vertical) {
            addClassName("vertical");
        } else {
            addClassName("horizontal");
        }
        _inlineStyle.grow(0)
            .shrink(0);

        onResize(
            [this](int /*width*/, int /*height*/) {
                updateSkin();
            }
        );

        onMouseScroll(
            [this](const int /*mouseX*/, const int /*mouseY*/, const double scrollX, const double scrollY) {
                if (_direction == ScrollDirection::Vertical) {
                    _viewport->scroll(0, scrollY);
                } else {
                    _viewport->scroll(scrollX, 0);
                }
            }
        );

        // The viewport may outlive us: these subscriptions end with us
        subscribeTo(
            viewport.resized, [this](int /*width*/, int /*height*/) {
                updateSkin();
            }
        );

        subscribeTo(
            viewport.scrolled, [this](int /*scrollX*/, int /*scrollY*/) {
                updateSkin();
            }
        );
    }

    Div *ScrollBar::viewport() const {
        return _viewport.get();
    }

    ScrollDirection ScrollBar::scrollDirection() const {
        return _direction;
    }

    void ScrollBar::scrollPercentX(float scrollPercentX) {
        _viewport->scrollX(static_cast<int>(-std::max(0.0f, std::fmin(scrollPercentX, 1.0f)) * static_cast<float>(_viewport->contentWidth() - _viewport->getWidth())));
    }

    void ScrollBar::scrollPercentY(float scrollPercentY) {
        _viewport->scrollY(static_cast<int>(-std::max(0.0f, std::fmin(scrollPercentY, 1.0f)) * static_cast<float>(_viewport->contentHeight() - _viewport->getHeight())));
    }

    void ScrollBar::updateSkin() {
        float scrollPosition = 0;
        float contentRatio   = 1.0f;
        bool  enabled        = false;

        if (_direction == ScrollDirection::Vertical) {
             std::cout << "scroll " << _viewport->contentHeight() << " " << _viewport->contentHeight() << " " << _viewport->getHeight() << std::endl;
            if (_viewport && _viewport->contentHeight() > 0 && _viewport->contentHeight() > _viewport->getHeight()) {
                _viewport->scrollY(
                    std::max(_viewport->scrollY(), _viewport->getHeight() - _viewport->contentHeight())
                );
                scrollPosition = -(float) _viewport->scrollY() / (float) (_viewport->contentHeight() - _viewport->getHeight());
                contentRatio   = std::max(0.1f, (float) _viewport->getHeight() / (float) _viewport->contentHeight());
                enabled        = true;
            } else {
                _viewport->scrollY(0);
            }
        } else {
            if (_viewport && _viewport->contentWidth() > 0 && _viewport->contentWidth() > _viewport->getWidth()) {
                _viewport->scrollX(
                    std::max(_viewport->scrollX(), _viewport->getWidth() - _viewport->contentWidth())
                );
                scrollPosition = -(float) _viewport->scrollX() / (float) (_viewport->contentWidth() - _viewport->getWidth());
                contentRatio   = std::max(0.1f, (float) _viewport->getWidth() / (float) _viewport->contentWidth());
                enabled        = true;
            } else {
                _viewport->scrollX(0);
            }
        }

        skin()->updateScrollBar(enabled, scrollPosition, contentRatio);
        this->enabled(enabled);
    }

}
