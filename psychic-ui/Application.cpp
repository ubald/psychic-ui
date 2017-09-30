#include <memory>
#include "Application.hpp"
#include "Window.hpp"

namespace psychic_ui {

    SystemWindow::SystemWindow(Application *application, std::shared_ptr<Window> window) :
        _application(application),
        _window(window) {

        _width  = window->windowWidth();
        _height = window->windowHeight();
    }

    std::shared_ptr<Window> SystemWindow::window() const {
        return _window;
    }

    int SystemWindow::getX() const {
        return _x;
    }

    int SystemWindow::getY() const {
        return _y;
    }

    int SystemWindow::getWidth() const {
        return _width;
    }

    int SystemWindow::getHeight() const {
        return _height;
    }

    int SystemWindow::getSamples() const {
        return _samples;
    }

    int SystemWindow::getStencilBits() const {
        return _stencilBits;
    }

    void SystemWindow::startTextInput() {}

    void SystemWindow::stopTextInput() {}
}
