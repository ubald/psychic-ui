#include <iostream>
#include "GrBackendSurface.h"
#include "Window.hpp"

namespace psychicui {

    #if defined(PSYCHICUI_GLAD)
    static bool gladInitialized = false;
    #endif

    /**
     * Calculate pixel ratio for hi-dpi devices.
     */
    static float get_pixel_ratio(GLFWwindow *window) {
        #if defined(_WIN32)
        HWND hWnd = glfwGetWin32Window(window);
        HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        /* The following function only exists on Windows 8.1+, but we don't want to make that a dependency */
        static HRESULT (WINAPI *GetDpiForMonitor_)(HMONITOR, UINT, UINT*, UINT*) = nullptr;
        static bool GetDpiForMonitor_tried = false;

        if (!GetDpiForMonitor_tried) {
            auto shcore = LoadLibrary(TEXT("shcore"));
            if (shcore)
                GetDpiForMonitor_ = (decltype(GetDpiForMonitor_)) GetProcAddress(shcore, "GetDpiForMonitor");
            GetDpiForMonitor_tried = true;
        }

        if (GetDpiForMonitor_) {
            uint32_t dpiX, dpiY;
            if (GetDpiForMonitor_(monitor, 0 /* effective DPI */, &dpiX, &dpiY) == S_OK)
                return std::round(dpiX / 96.0);
        }
        return 1.f;
        #elif defined(__linux__)
        (void) window;

        /* Try to read the pixel ratio from GTK */
        FILE *fp = popen("gsettings get org.gnome.desktop.interface scaling-factor", "r");
        if (!fp)
            return 1;

        int ratio = 1;
        if (fscanf(fp, "uint32 %i", &ratio) != 1)
            return 1;

        if (pclose(fp) != 0)
            return 1;

        return ratio >= 1 ? ratio : 1;
        #else
        int fbWidth, fbHeight, width, height;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glfwGetWindowSize(window, &width, &height);
        return (float) fbWidth / (float) width;
        #endif
    }

    std::unordered_map<GLFWwindow *, Window *> Window::windows;

    Window::Window(const std::string &title) :
        Div::Div(),
        _title(title) {
        setStyleManager(StyleManager::getInstance());
        setTag("Window");
        setWindowSize(1440, 900);
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
        windows.erase(_glfwWindow);
        for (int i = 0; i < 6; ++i) {
            if (_cursors[i]) {
                glfwDestroyCursor(_cursors[i]);
            }
        }

        delete _sk_context;
        delete _sk_surface;

        if (_glfwWindow) {
            glfwDestroyWindow(_glfwWindow);
        }
    }

    GLFWwindow *Window::glfwWindow() {
        return _glfwWindow;
    }

    // region Hierarchy

    Window *Window::window() {
        return this;
    }

    // endregion

    // region Lifecycle

    void Window::open() {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwWindowHint(GLFW_SAMPLES, 0);
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 8);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_VISIBLE, _visible ? GL_TRUE : GL_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, _resizable ? GL_TRUE : GL_FALSE);
        glfwWindowHint(GLFW_DECORATED, _decorated ? GL_TRUE : GL_FALSE);

        if (_fullscreen) {
            GLFWmonitor       *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode    = glfwGetVideoMode(monitor);
            _glfwWindow = glfwCreateWindow(mode->width, mode->height, _title.c_str(), monitor, nullptr);
        } else {
            _glfwWindow = glfwCreateWindow(_windowWidth, _windowHeight, _title.c_str(), nullptr, nullptr);
        }

        if (!_glfwWindow) {
            throw std::runtime_error("Could not create an OpenGL context!");
        }

        glfwMakeContextCurrent(_glfwWindow);

        #if defined(PSYCHICUI_GLAD)
        if (!gladInitialized) {
            gladInitialized = true;
            if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
                throw std::runtime_error("Could not initialize GLAD!");
            glGetError();
        }
        #endif

        glfwGetWindowPos(_glfwWindow, &_windowX, &_windowY);
        glfwGetWindowSize(_glfwWindow, &_windowWidth, &_windowHeight);
        _pixelRatio = get_pixel_ratio(_glfwWindow);
        #if defined(_WIN32) || defined(__linux__)
        if (_pixelRatio != 1 && !_fullscreen)
            glfwSetWindowSize(_glfwWindow, _windowWidth * _pixelRatio, _windowHeight * _pixelRatio);
        #endif

        glfwGetFramebufferSize(_glfwWindow, &_fbWidth, &_fbHeight);
        glViewport(0, 0, _fbWidth, _fbHeight);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glfwSwapInterval(0);
        glfwSwapBuffers(_glfwWindow);

        #if defined(__APPLE__)
        // Poll for events once before starting. This is needed to be classified as "interactive"
        glfwPollEvents();
        #endif

        // Get Some info about the framebuffer
        glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER,
            GL_STENCIL,
            GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
            &_stencilBits
        );
        glGetIntegerv(GL_SAMPLES, &_samples);

        // Setup Skia
        initSkia();

        // Setup Callbacks
        attachCallbacks();

        // Be ready to start
        _visible         = glfwGetWindowAttrib(_glfwWindow, GLFW_VISIBLE) != 0;
        _lastInteraction = glfwGetTime();

        for (int i = 0; i < 6; ++i) {
            _cursors[i] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR + i);
        }

        windows[_glfwWindow] = this;

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
        GrGLFramebufferInfo framebufferInfo;
        framebufferInfo.fFBOID = 0;  // assume default framebuffer
        GrBackendRenderTarget backendRenderTarget(
            _windowWidth,
            _windowHeight,
            _samples,
            _stencilBits,
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

    void Window::attachCallbacks() {
        glfwSetCursorPosCallback(
            _glfwWindow,
            [](GLFWwindow *w, double x, double y) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->cursorPosEventCallback(x, y);
            }
        );

        glfwSetMouseButtonCallback(
            _glfwWindow,
            [](GLFWwindow *w, int button, int action, int modifiers) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->mouseButtonEventCallback(button, action, modifiers);
            }
        );

        glfwSetScrollCallback(
            _glfwWindow,
            [](GLFWwindow *w, double x, double y) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->scrollEventCallback(x, y);
            }
        );

        glfwSetKeyCallback(
            _glfwWindow,
            [](GLFWwindow *w, int key, int scancode, int action, int mods) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->keyEventCallback(key, scancode, action, mods);
            }
        );

        glfwSetCharCallback(
            _glfwWindow,
            [](GLFWwindow *w, unsigned int codepoint) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->charEventCallback(codepoint);
            }
        );

        glfwSetDropCallback(
            _glfwWindow,
            [](GLFWwindow *w, int count, const char **filenames) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->dropEventCallback(count, filenames);
            }
        );

        glfwSetFramebufferSizeCallback(
            _glfwWindow,
            [](GLFWwindow *w, int width, int height) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->resizeEventCallback(width, height);
            }
        );

        glfwSetWindowPosCallback(
            _glfwWindow,
            [](GLFWwindow *w, int x, int y) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->positionEventCallback(x, y);
            }
        );

        glfwSetWindowFocusCallback(
            _glfwWindow,
            [](GLFWwindow *w, int focused) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->focusEventCallback(focused);
            }
        );

        glfwSetWindowIconifyCallback(
            _glfwWindow,
            [](GLFWwindow *w, int iconified) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->iconifyEventCallback(iconified);
            }
        );

        glfwSetWindowCloseCallback(
            _glfwWindow,
            [](GLFWwindow *w) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->closeEventCallback();
            }
        );

        // Continue rendering while event polling is frozen by a resize
        glfwSetWindowRefreshCallback(
            _glfwWindow,
            [](GLFWwindow *w) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->drawAll();
            }
        );
    }

    // endregion

    // region Title

    const std::string Window::title() const {
        return _title;
    }

    void Window::setTitle(const std::string &title) {
        if (_title != title) {
            _title = title;
            if (_glfwWindow) {
                glfwSetWindowTitle(_glfwWindow, _title.c_str());
            }
        }
    }

    // endregion

    // region Fullscreen

    void Window::toggleFullscreen() {
        setFullscreen(!_fullscreen);
    }

    bool Window::fullscreen() const {
        return _fullscreen;
    }

    void Window::setFullscreen(bool fullscreen) {
        if (_fullscreen != fullscreen) {
            _fullscreen = fullscreen;
            if (_glfwWindow) {
                if (_fullscreen) {
                    GLFWmonitor       *monitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode *mode    = glfwGetVideoMode(monitor);
                    glfwGetWindowPos(_glfwWindow, &_previousWindowX, &_previousWindowY);
                    glfwGetWindowSize(_glfwWindow, &_previousWindowWidth, &_previousWindowHeight);
                    glfwSetWindowMonitor(
                        _glfwWindow,
                        monitor,
                        0,
                        0,
                        mode->width,
                        mode->height,
                        GLFW_DONT_CARE
                    );
                } else {
                    glfwSetWindowMonitor(
                        _glfwWindow,
                        nullptr,
                        _previousWindowX,
                        _previousWindowY,
                        _previousWindowWidth,
                        _previousWindowHeight,
                        GLFW_DONT_CARE
                    );
                }
            }
        }
    }

    // endregion

    // region Minimized

    void Window::toggleMinimized() {
        setMinimized(!_minimized);
    }

    bool Window::minimized() const {
        return _minimized;
    }

    void Window::setMinimized(bool minimized) {
        if (_minimized != minimized) {
            _minimized = minimized;
            if (_glfwWindow) {
                if (_minimized) {
                    glfwIconifyWindow(_glfwWindow);
                } else {
                    glfwRestoreWindow(_glfwWindow);
                }
            }
        }
    }

    // endregion

    // region Maximized

    void Window::toggleMaximized() {
        setMaximized(!_maximized);
    }

    bool Window::maximized() const {
        return _maximized;
    }

    void Window::setMaximized(bool maximized) {
        if (_maximized != maximized) {
            _maximized = maximized;
            if (_glfwWindow) {
                if (_maximized) {
                    glfwMaximizeWindow(_glfwWindow);
                } else {
                    glfwRestoreWindow(_glfwWindow);
                }
            }
        }
    }

    // endregion

    // region Visible

    void Window::setVisible(bool value) {
        if (_visible != value) {
            _visible = value;
            if (_visible) {
                glfwShowWindow(_glfwWindow);
            } else {
                glfwHideWindow(_glfwWindow);
            }
        }
    }

    // endregion

    // region Mouse

    void Window::setCursor(int cursor) {
        if (_cursor != cursor) {
            _cursor = cursor;
            glfwSetCursor(_glfwWindow, _cursors[_cursor]);
        }
    }

    // endregion

    // region Position

    void Window::startDrag() {
        _windowDragMouseX = _mouseX;
        _windowDragMouseY = _mouseY;
        _dragging         = true;
    }

    void Window::stopDrag() {
        _dragging         = false;
        _windowDragMouseX = 0;
        _windowDragMouseY = 0;
    }

    const int Window::windowX() const {
        return _windowX;
    }

    const int Window::windowY() const {
        return _windowY;
    }

    void Window::setWindowPosition(const int x, const int y) {
        _windowX = x;
        _windowY = y;
        if (_glfwWindow) {
            glfwSetWindowPos(_glfwWindow, _windowX, _windowY);
        }
    }

    // endregion

    // region Size

    const int Window::windowWidth() const {
        return _windowWidth;
    }

    const int Window::windowHeight() const {
        return _windowHeight;
    }

    void Window::setWindowSize(const int width, const int height) {
        _windowWidth  = width;
        _windowHeight = height;
        setSize(_windowWidth, _windowHeight);
        if (_glfwWindow) {
            glfwSetWindowSize(_glfwWindow, _windowWidth, _windowHeight);
        }
    }

    // endregion

    // region Draw

    void Window::drawAll() {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawContents();
        drawComponents();
        glfwSwapBuffers(_glfwWindow);

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
            return;
        }

        glfwMakeContextCurrent(_glfwWindow);
        glfwGetFramebufferSize(_glfwWindow, &_fbWidth, &_fbHeight);
        glfwGetWindowSize(_glfwWindow, &_windowWidth, &_windowHeight);

        #if defined(_WIN32) || defined(__linux__)
        _windowWidth = (int)(_windowWidth / _pixelRatio);
        _windowHeight =(int)(_windowHeight / _pixelRatio);
        _fbWidth = (int)(_fbWidth * _pixelRatio);
        _fbHeight = (int)(_fbHeight * _pixelRatio);
        #else
        if (_windowWidth) {
            _pixelRatio = (float) _fbWidth / (float) _windowWidth;
        }
        #endif

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
            YGNodeCalculateLayout(_yogaNode, _windowWidth, _windowHeight, YGDirectionLTR);
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

        glViewport(0, 0, _fbWidth, _fbHeight);
        glBindSampler(0, 0);

        _sk_canvas->clear(0x00000000);
        render(_sk_canvas);
        _sk_canvas->flush();
    }

    // endregion

    // region Modals

    void Window::openMenu(const std::vector<std::shared_ptr<MenuItem>> &items, const int x, const int y) {
        menu->removeAll();
        int lx, ly;
        menu->getGlobalToLocal(lx, ly, x, y);
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

    void Window::cursorPosEventCallback(double x, double y) {
        #if defined(_WIN32) || defined(__linux__)
        _x /= _pixelRatio;
        _y /= _pixelRatio;
        #endif

        _lastInteraction = glfwGetTime();

        // Weird but the cursor seem better aligned like this
        // At least on a mac...
        _mouseX = (int) x - 1;
        _mouseY = (int) y - 2;

        // Handle window dragging
        if (_dragging) {
            _windowDragOffsetX = (int) x - _windowDragMouseX;
            _windowDragOffsetY = (int) y - _windowDragMouseY;
            setWindowPosition(_windowX + _windowDragOffsetX, _windowY + _windowDragOffsetY);
        }

        try {
            mouseMoved(_mouseX, _mouseY, _mouseState, _modifiers, false);

        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::mouseButtonEventCallback(int button, int action, int modifiers) {
        _modifiers       = modifiers;
        _lastInteraction = glfwGetTime();

        try {
            if (action == GLFW_PRESS) {
                _mouseState |= 1 << button;
            } else {
                _mouseState &= ~(1 << button);
            }

            mouseButton(_mouseX, _mouseY, button, action == GLFW_PRESS, _modifiers);
            if (action == GLFW_PRESS) {
                mouseDown(_mouseX, _mouseY, button, _modifiers);
            } else {
                click(_mouseX, _mouseY, button, _modifiers);
                mouseUp(_mouseX, _mouseY, button, _modifiers);
            }

        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::scrollEventCallback(double x, double y) {
        _lastInteraction = glfwGetTime();
        try {
//            if (_focusPath.setSize() > 1) {
//                const std::shared_ptr<Panel> panel = std::dynamic_pointer_cast<Panel>(
//                    _focusPath[_focusPath.setSize()
//                               - 2]
//                );
//                if (panel && panel->modal()) {
//                    if (!panel->boundsContains(_mouseX, _mouseY)) {
//                        return;
//                    }
//                }
//            }
            mouseScrolled(_mouseX, _mouseY, x, y);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::keyEventCallback(int key, int scancode, int action, int mods) {
        _lastInteraction = glfwGetTime();
        try {
            keyboardEvent(key, scancode, action, mods);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::charEventCallback(unsigned int codepoint) {
        _lastInteraction = glfwGetTime();
        try {
            keyboardCharacterEvent(codepoint);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::dropEventCallback(int count, const char **filenames) {
        std::vector<std::string> arg(count);
        for (int                 i = 0; i < count; ++i) {
            arg[i] = filenames[i];
        }
        try {
            dropEvent(arg);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::resizeEventCallback(int /*setWidth*/, int /*setHeight*/) {
        int fbWidth, fbHeight, width, height;
        glfwGetFramebufferSize(_glfwWindow, &fbWidth, &fbHeight);
        glfwGetWindowSize(_glfwWindow, &width, &height);

        #if defined(_WIN32) || defined(__linux__)
        _windowWidth /= _pixelRatio;
        _windowHeight /= _pixelRatio;
        #endif

        if ((fbWidth == 0 && fbHeight == 0) || (width == 0 && height == 0)) {
            return;
        }

        _fbWidth      = fbWidth;
        _fbHeight     = fbHeight;
        _windowWidth  = width;
        _windowHeight = height;

        // Set setLayout setSize
        YGNodeStyleSetWidth(_yogaNode, _windowWidth);
        YGNodeStyleSetHeight(_yogaNode, _windowHeight);

        // Get a new surface
        getSkiaSurface();

        _lastInteraction = glfwGetTime();

        try {
            windowResized(_windowWidth, _windowHeight);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::positionEventCallback(int x, int y) {
        _windowX = x;
        _windowY = y;

        try {
            windowMoved(_windowX, _windowY);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::focusEventCallback(int focused) {
        _lastInteraction = glfwGetTime();
        try {
            _windowFocused = focused == 1;
            if (_windowFocused) {
                windowActivated();
            } else {
                windowDeactivated();
            }
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::iconifyEventCallback(int iconified) {
        _lastInteraction = glfwGetTime();
        try {
            _minimized = iconified == 1;
            if (_minimized) {
                windowMinimized();
            } else {
                windowRestored();
            }
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::closeEventCallback() {
        _lastInteraction = glfwGetTime();
        try {
            glfwSetWindowShouldClose(_glfwWindow, windowShouldClose() ? GLFW_TRUE : GLFW_FALSE);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    // endregion

}
