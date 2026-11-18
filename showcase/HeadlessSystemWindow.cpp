#include "HeadlessSystemWindow.hpp"
#include <psychic-ui/Window.hpp>

namespace psychic_ui::showcase {
    HeadlessSystemWindow::HeadlessSystemWindow(std::shared_ptr<Window> window, const int width, const int height,
                                               const float pixelRatio) :
        SystemWindow(std::move(window)) {
        _width    = width;
        _height   = height;
        _fbWidth  = static_cast<int>(static_cast<float>(width) * pixelRatio);
        _fbHeight = static_cast<int>(static_cast<float>(height) * pixelRatio);
        _window->open(this);
    }

    HeadlessSystemWindow::~HeadlessSystemWindow() {
        detach();
    }
}
