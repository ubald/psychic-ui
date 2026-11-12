#include <memory>
#include "SystemWindow.hpp"
#include "Window.hpp"

namespace psychic_ui {

    SystemWindow::SystemWindow(std::shared_ptr<Window> window) :
        _window(std::move(window)) {

        _width    = _window->windowWidth();
        _height   = _window->windowHeight();
        _fbWidth  = _width;
        _fbHeight = _height;
    }

    SystemWindow::~SystemWindow() {
        detach();
    }

    void SystemWindow::detach() {
        if (_window && _window->systemWindow() == this) {
            _window->open(nullptr);
        }
    }

    std::shared_ptr<Window> SystemWindow::window() const {
        return _window;
    }

    bool SystemWindow::sharesContext() const {
        return _sharesContext;
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

    int SystemWindow::getFramebufferWidth() const {
        return _fbWidth;
    }

    int SystemWindow::getFramebufferHeight() const {
        return _fbHeight;
    }

    float SystemWindow::getPixelRatio() const {
        return _width > 0 ? static_cast<float>(_fbWidth) / static_cast<float>(_width) : 1.0f;
    }

    int SystemWindow::getSamples() const {
        return _samples;
    }

    int SystemWindow::getStencilBits() const {
        return _stencilBits;
    }

    bool SystemWindow::supportsDrag() const {
        return true;
    }

    void SystemWindow::startTextInput() {}

    void SystemWindow::stopTextInput() {}

    std::string SystemWindow::getClipboardText() const {
        return _clipboardText;
    }

    void SystemWindow::setClipboardText(const std::string &text) {
        _clipboardText = text;
    }

    void SystemWindow::readFramebufferFormat() {
        // Through the window's own GL loader, so that no GL headers or link-time GL symbols are needed
        constexpr unsigned GL_SAMPLES_                           = 0x80A9;
        constexpr unsigned GL_DRAW_FRAMEBUFFER_                  = 0x8CA9;
        constexpr unsigned GL_STENCIL_                           = 0x1802;
        constexpr unsigned GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE_ = 0x8217;
        using GetIntegerv                          = void (*)(unsigned, int *);
        using GetFramebufferAttachmentParameteriv  = void (*)(unsigned, unsigned, unsigned, int *);
        using GetError                             = unsigned (*)();

        auto getIntegerv = reinterpret_cast<GetIntegerv>(glProcAddress("glGetIntegerv"));
        auto getAttachment = reinterpret_cast<GetFramebufferAttachmentParameteriv>(glProcAddress("glGetFramebufferAttachmentParameteriv"));
        auto getError = reinterpret_cast<GetError>(glProcAddress("glGetError"));
        if (!getIntegerv || !getAttachment || !getError) {
            return;
        }

        int samples = 0;
        getIntegerv(GL_SAMPLES_, &samples);
        if (getError() == 0) {
            _samples = samples;
        }

        int stencil = 0;
        getAttachment(GL_DRAW_FRAMEBUFFER_, GL_STENCIL_, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE_, &stencil);
        if (getError() == 0) {
            _stencilBits = stencil;
        }
    }
}
