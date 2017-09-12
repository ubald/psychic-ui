#include "ScrollBar.hpp"

namespace psychicui {
    ScrollBar::ScrollBar(const std::shared_ptr<Div> &viewport, ScrollDirection direction) :
        Component(),
        _direction(direction),
        _viewport(viewport) {
        setTag("ScrollBar");
        style()
            ->set(grow, 0)
            ->set(shrink, 0);

        onResized(
            [this](int width, int height) {
                updateSkin();
            }
        );

        onMouseScroll([this](const int mouseX, const int mouseY, const double scrollX, const double scrollY) {
            _viewport->scroll(scrollX, scrollY);
        });

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


    const ScrollDirection ScrollBar::direction() const {
        return _direction;
    }

    void ScrollBar::updateSkin() {
        float scrollPosition = 0;
        float contentRatio = 1.0f;
        bool enabled = false;

        if (_direction == Vertical) {
            if (_viewport && _viewport->contentHeight() > 0 && _viewport->contentHeight() > _viewport->getHeight()) {
                _viewport->setScrollY(
                    std::min(_viewport->scrollY(), _viewport->contentHeight() - _viewport->getHeight())
                );
                scrollPosition = -_viewport->scrollY() / (float)(_viewport->contentHeight() - _viewport->getHeight());
                contentRatio = std::max(0.1f, (float)_viewport->getHeight() / (float)_viewport->contentHeight());
                enabled      = true;
            } else {
                _viewport->setScrollY(0);
            }
        } else {
            if (_viewport && _viewport->contentWidth() > 0 && _viewport->contentWidth() > _viewport->getWidth()) {
                _viewport->setScrollX(
                    std::min(_viewport->scrollX(), _viewport->contentWidth() - _viewport->getWidth())
                );
                scrollPosition = -_viewport->scrollX() / (float)(_viewport->contentWidth() - _viewport->getWidth());
                contentRatio = std::max(0.1f, (float)_viewport->getWidth() / (float)_viewport->contentWidth());
                enabled      = true;
            } else {
                _viewport->setScrollX(0);
            }
        }

        skin()->updateScrollBar(enabled, scrollPosition, contentRatio);
        setEnabled(enabled);
    }

}
