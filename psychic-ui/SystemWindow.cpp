#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include "SystemWindow.hpp"
#include "Window.hpp"
#include "WindowScale.hpp"

namespace psychic_ui {

    SystemWindow::SystemWindow(std::shared_ptr<Window> window) :
        _window(std::move(window)),
        _width(_window->windowWidth()),
        _height(_window->windowHeight()),
        _fbWidth(_width),
        _fbHeight(_height) {}

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

    void SystemWindow::setSharesContext(bool sharesContext) {
        _sharesContext = sharesContext;
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
        return WindowScale::pixelRatio(_fbWidth, _width);
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

    std::chrono::nanoseconds SystemWindow::gestureTime() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch());
    }

    void SystemWindow::startTextInput() {}

    void SystemWindow::stopTextInput() {}

    void SystemWindow::setTextInputArea(int /*x*/, int /*y*/, int /*width*/, int /*height*/, int /*cursor*/) {}

    std::string SystemWindow::getClipboardText() const {
        return _clipboardText;
    }

    void SystemWindow::setClipboardText(const std::string &text) {
        _clipboardText = text;
    }

    namespace {
        /**
         * A GL procedure as its real type
         */
        template<class Procedure>
        Procedure procedure(const SystemWindow::GLProc address) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): GL procedures come from the loader untyped
            return reinterpret_cast<Procedure>(address);
        }
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

        const auto getIntegerv   = procedure<GetIntegerv>(glProcAddress("glGetIntegerv"));
        const auto getAttachment = procedure<GetFramebufferAttachmentParameteriv>(glProcAddress("glGetFramebufferAttachmentParameteriv"));
        const auto getError      = procedure<GetError>(glProcAddress("glGetError"));
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
