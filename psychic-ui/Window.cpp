#include <iostream>
#include "GrBackendSurface.h"
#include "Window.hpp"

namespace psychic_ui {

    Window::Window(const std::string &title) :
        Div::Div(),
        _title(title) {
        setStyleManager(StyleManager::getInstance()); // NOTE: Each window could get its own style manager
        setTag("Window");
        setWindowSize(1440, 900);

        // Initialize Yoga
        // TODO: This should not be repeated for every window since its global
        YGConfigSetUseWebDefaults(YGConfigGetDefault(), true);
        YGConfigSetExperimentalFeatureEnabled(YGConfigGetDefault(), YGExperimentalFeatureWebFlexBasis, true);
        YGConfigSetPointScaleFactor(YGConfigGetDefault(), 0.0f); // We'll round the values ourselves, rounding is bugged


        _inlineStyle->set(position, "absolute");
        _inlineStyle->set(overflow, "hidden");

        app = add<Div>();
        app->setId("app");
        app->style()
           ->set(position, "absolute")
           ->set(widthPercent, 1.0f)
           ->set(heightPercent, 1.0f)
           ->set(overflow, "hidden");

        modal = add<Modal>();
        modal->setId("modal");
        modal->style()
             ->set(visible, false);

        menu = add<Modal>();
        menu->setId("menu");
        menu->onMouseDown.subscribe(
            [this](const int mouseX, const int mouseY, const int button, const int modifiers) {
                closeMenu();
            }
        );
        menu->style()
            ->set(visible, false);
    }

    Window::~Window() {
        delete _sk_surface;
        delete _sk_context;
    }

    // region Hierarchy

    Window *Window::window() {
        return this;
    }

    // endregion

    // region Lifecycle

    void Window::open(SystemWindow *systemWindow) {
        _systemWindow = systemWindow;

        // Setup Skia
        initSkia();

        // Performance
        lastReport = std::chrono::high_resolution_clock::now();
    }

    void Window::close() {
        // TODO: Find a better application-friendly close method
        _visible = false;
    }

    void Window::initSkia() {
        const GrGLInterface *interface = nullptr;
        _sk_context = GrContext::MakeGL(interface).release();
        getSkiaSurface();
    }

    void Window::getSkiaSurface() {
        if (!_systemWindow) {
            throw std::runtime_error("Skia surface requested without a context");
        }

        GrGLFramebufferInfo framebufferInfo;
        framebufferInfo.fFBOID = 0;  // assume default framebuffer
        GrBackendRenderTarget backendRenderTarget(
            _systemWindow->getWidth(),
            _systemWindow->getHeight(),
            _systemWindow->getSamples(),
            _systemWindow->getStencilBits(),
            kSkia8888_GrPixelConfig,
            framebufferInfo
        );
        _sk_surface = SkSurface::MakeFromBackendRenderTarget(
            _sk_context,
            backendRenderTarget,
            kBottomLeft_GrSurfaceOrigin,
            nullptr,
            nullptr
        ).release();
        if (!_sk_surface) {
            SkDebugf("SkSurface::MakeFromBackendRenderTarget returned null\n");
            return;
        }
        _sk_canvas = _sk_surface->getCanvas();
    }

    // endregion

    // region Window Attributes

    const std::string Window::getTitle() const {
        return _title;
    }

    void Window::setTitle(const std::string &title) {
        if (_title != title) {
            _title = title;
            if (_systemWindow) {
                _systemWindow->setTitle(title);
            }
        }
    }

    bool Window::getResizable() const {
        return _resizable;
    }

    void Window::setResizable(bool resizable) {
        // TODO: Actually do something with the value
        _resizable = resizable;
    }

    bool Window::getDecorated() const {
        return _decorated;
    }

    void Window::setDecorated(bool decorated) {
        // TODO: Actually do something with the value
        _decorated = decorated;
    }

    void Window::toggleFullscreen() {
        setFullscreen(!_fullscreen);
    }

    bool Window::getFullscreen() const {
        return _fullscreen;
    }

    void Window::setFullscreen(bool fullscreen) {
        if (_fullscreen != fullscreen) {
            _fullscreen = fullscreen;
            if (_systemWindow) {
                _systemWindow->setFullscreen(_fullscreen);
            }
        }
    }

    void Window::toggleMinimized() {
        setMinimized(!minimized());
    }

    bool Window::minimized() const {
        return _systemWindow && _systemWindow->getMinimized();
    }

    void Window::setMinimized(bool minimized) {
        if (_systemWindow) {
            _systemWindow->setMinimized(minimized);
        }
    }

    void Window::toggleMaximized() {
        setMaximized(!maximized());
    }

    bool Window::maximized() const {
        return _systemWindow && _systemWindow->getMaximized();
    }

    void Window::setMaximized(bool maximized) {
        if (_systemWindow) {
            _systemWindow->setMaximized(maximized);
        }
    }

    void Window::setVisible(bool value) {
        if (_visible != value) {
            _visible = value;
            if (_systemWindow) {
                _systemWindow->setVisible(_visible);
            }
        }
    }

    void Window::setCursor(int cursor) {
        if (_cursor != cursor) {
            _cursor = cursor;
            if (_systemWindow) {
                _systemWindow->setCursor(cursor);
            }
        }
    }

    void Window::startDrag() {
        if (_systemWindow) {
            _systemWindow->startDrag();
        }
    }

    void Window::stopDrag() {
        if (_systemWindow) {
            _systemWindow->stopDrag();
        }
    }

    const int Window::windowX() const {
        return _systemWindow ? _systemWindow->getX() : 0;
    }

    const int Window::windowY() const {
        return _systemWindow ? _systemWindow->getY() : 0;
    }

    const int Window::windowWidth() const {
        return _systemWindow ? _systemWindow->getWidth() : _width;
    }

    const int Window::windowHeight() const {
        return _systemWindow ? _systemWindow->getHeight() : _height;
    }

    void Window::setWindowSize(const int width, const int height) {
        setSize(width, height);
        if (_systemWindow) {
            _systemWindow->setSize(width, height);
        }
    }

    // endregion

    // region Draw

    void Window::drawAll() {
        drawContents();
        drawComponents();

        // Performance
        ++frames;
        double delta = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now()
            - lastReport
        ).count();
        if (delta >= 500) {
            lastReport = std::chrono::high_resolution_clock::now();
            fps        = frames / (delta / 1000.0f);
            frames     = 0;
        }
    }

    void Window::drawContents() {

    }

    void Window::drawComponents() {
        if (!_visible) {
            // TODO: That should not happen
            return;
        }

        // Check for dirty style manager
        // Before layout since it can have an impact on the layout
        if (!_styleManager->valid()) {
            updateStyleRecursive();
            _styleManager->setValid();
        }

        // Do Layout
        if (YGNodeIsDirty(_yogaNode)) {
            #ifdef DEBUG_LAYOUT
            if (debugLayout) {
                std::cout << "Layout dirty!" << std::endl;
            }
            #endif
            YGNodeCalculateLayout(_yogaNode, _width, _height, YGDirectionLTR);
            layoutUpdated();
            #ifdef DEBUG_LAYOUT
            if (debugLayout) {
                YGNodePrint(
                    _yogaNode,
                    static_cast<YGPrintOptions>(YGPrintOptionsLayout
                                                | YGPrintOptionsStyle
                                                | YGPrintOptionsChildren));
                std::cout << std::endl;
            }
            #endif
        }

        _sk_canvas->clear(0x00000000);
        render(_sk_canvas);
        _sk_canvas->flush();
    }

    // endregion

    // region Modals

    void Window::openMenu(const std::vector<std::shared_ptr<MenuItem>> &items, const int x, const int y) {
        menu->removeAll();
        int lx, ly;
        menu->globalToLocal(lx, ly, x, y);
        auto m = menu->add<Menu>(items);
        m->style()
         ->set(left, lx)
         ->set(top, ly);
        menu->style()
            ->set(visible, true);
        onMenuOpened.emit();
    }

    void Window::closeMenu() {
        menu->removeAll();
        menu->style()
            ->set(visible, false);
        onMenuClosed.emit();
    }

    // endregion

    // region Events???

    bool Window::keyboardEvent(int key, int scancode, int action, int modifiers) {
//        if (_focusPath.size() > 0) {
//            for (auto it = _focusPath.rbegin() + 1; it != _focusPath.rend(); ++it) {
//                if ((*it)->focused() && (*it)->keyboardEvent(key, scancode, action, modifiers)) {
//                    return true;
//                }
//            }
//        }
        return false;
    }

    bool Window::keyboardCharacterEvent(unsigned int codepoint) {
//        if (_focusPath.size() > 0) {
//            for (auto it = _focusPath.rbegin() + 1; it != _focusPath.rend(); ++it) {
//                if ((*it)->focused() && (*it)->keyboardCharacterEvent(codepoint)) {
//                    return true;
//                }
//            }
//        }
        return false;
    }

    // endregion

    // region Callback Delegates

    void Window::windowMoved(const int x, const int y) {
        // std::cout << "Moved" << std::endl;
    }

    void Window::windowResized(const int width, const int height) {
        // std::cout << "Resized" << std::endl;

        // Set setLayout setSize
        YGNodeStyleSetWidth(_yogaNode, width);
        YGNodeStyleSetHeight(_yogaNode, height);

        // Get a new surface
        getSkiaSurface();
    }

    void Window::windowActivated() {
        // std::cout << "Activated" << std::endl;
    }

    void Window::windowDeactivated() {
        // std::cout << "Deactivated" << std::endl;
    }

    void Window::windowMinimized() {
        // std::cout << "Minimized" << std::endl;
    }

    void Window::windowRestored() {
        // std::cout << "Restored" << std::endl;
    }

    bool Window::windowShouldClose() {
        return true;
    }

    // endregion

    // region Event Callbacks



    // endregion

}
