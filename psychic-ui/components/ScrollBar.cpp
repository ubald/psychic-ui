#include "ScrollBar.hpp"

namespace psychic_ui {
    ScrollBar::ScrollBar(const std::shared_ptr<Div> &viewport, ScrollDirection direction) :
        Component(),
        _direction(direction),
        _viewport(viewport) {
        setTag("ScrollBar");
        if (_direction == Vertical) {
            addClassName("vertical");
        } else {
            addClassName("horizontal");
        }
        style()
            ->set(grow, 0)
            ->set(shrink, 0);

        onResized(
            [this](int width, int height) {
                updateSkin();
            }
        );

        onMouseScroll(
            [this](const int mouseX, const int mouseY, const double scrollX, const double scrollY) {
                if (_direction == Vertical) {
                    _viewport->scroll(0, scrollY);
                } else {
                    _viewport->scroll(scrollX, 0);
                }
            }
        );

        viewport->onResized(
            [this](int width, int height) {
                updateSkin();
            }
        );

        viewport->onScrolled(
            [this](int scrollX, int scrollY) {
                updateSkin();
            }
        );
    }

    Div *ScrollBar::viewport() const {
        return _viewport.get();
    }

    ScrollDirection ScrollBar::direction() const {
        return _direction;
    }

    void ScrollBar::scrollPercentX(float scrollPercentX) {
        _viewport->setScrollX((int) (
            -std::max(0.0f, std::fmin(scrollPercentX, 1.0f))
            * (float) (_viewport->contentWidth() - _viewport->getWidth())));
    }

    void ScrollBar::scrollPercentY(float scrollPercentY) {
        _viewport->setScrollY((int) (
            -std::max(0.0f, std::fmin(scrollPercentY, 1.0f))
            * (float) (_viewport->contentHeight() - _viewport->getHeight())));
    }

    void ScrollBar::updateSkin() {
        float scrollPosition = 0;
        float contentRatio   = 1.0f;
        bool  enabled        = false;

        if (_direction == Vertical) {
            if (_viewport && _viewport->contentHeight() > 0 && _viewport->contentHeight() > _viewport->getHeight()) {
                _viewport->setScrollY(
                    std::max(_viewport->scrollY(), _viewport->getHeight() - _viewport->contentHeight())
                );
                scrollPosition = -_viewport->scrollY() / (float) (_viewport->contentHeight() - _viewport->getHeight());
                contentRatio   = std::max(0.1f, (float) _viewport->getHeight() / (float) _viewport->contentHeight());
                enabled        = true;
            } else {
                _viewport->setScrollY(0);
            }
        } else {
            if (_viewport && _viewport->contentWidth() > 0 && _viewport->contentWidth() > _viewport->getWidth()) {
                _viewport->setScrollX(
                    std::max(_viewport->scrollX(), _viewport->getWidth() - _viewport->contentWidth())
                );
                scrollPosition = -_viewport->scrollX() / (float) (_viewport->contentWidth() - _viewport->getWidth());
                contentRatio   = std::max(0.1f, (float) _viewport->getWidth() / (float) _viewport->contentWidth());
                enabled        = true;
            } else {
                _viewport->setScrollX(0);
            }
        }

        skin()->updateScrollBar(enabled, scrollPosition, contentRatio);
        setEnabled(enabled);
    }

}
