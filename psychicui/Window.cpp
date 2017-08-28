#include <iostream>
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

    std::map<GLFWwindow *, Window *> Window::windows;

    Window::Window(const std::string &title) :
        Widget::Widget(),
        _title(title) {
        setSize(1440, 900);
        YGNodeStyleSetOverflow(_yogaNode, YGOverflowHidden);
        YGNodeStyleSetPositionType(_yogaNode, YGPositionTypeAbsolute);
    }

    Window::~Window() {
        windows.erase(_window);
        for (int i = 0; i < (int) Cursor::CursorCount; ++i) {
            if (_cursors[i]) {
                glfwDestroyCursor(_cursors[i]);
            }
        }

        delete _sk_context;
        delete _sk_surface;

//        if (_nvgContext) {
//            nvgDeleteGL3(_nvgContext);
//        }
        if (_window) {
            glfwDestroyWindow(_window);
        }
    }

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
            _window = glfwCreateWindow(mode->width, mode->height, _title.c_str(), monitor, nullptr);
        } else {
            _window = glfwCreateWindow(_width, _height, _title.c_str(), nullptr, nullptr);
        }

        if (!_window) {
            throw std::runtime_error("Could not create an OpenGL context!");
        }

        glfwMakeContextCurrent(_window);

        #if defined(PSYCHICUI_GLAD)
        if (!gladInitialized) {
            gladInitialized = true;
            if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
                throw std::runtime_error("Could not initialize GLAD!");
            glGetError();
        }
        #endif

        glfwGetWindowSize(_window, &_width, &_height);
        _pixelRatio = get_pixel_ratio(_window);
        #if defined(_WIN32) || defined(__linux__)
        if (_pixelRatio != 1 && !setFullscreen)
            glfwSetWindowSize(_window, _size.setX() * _pixelRatio, _size.setY() * _pixelRatio);
        #endif

        glfwGetFramebufferSize(_window, &_fbWidth, &_fbHeight);
        glViewport(0, 0, _fbWidth, _fbHeight);
        glClearColor(
            SkColorGetR(style()->windowBackgroundColor),
            SkColorGetG(style()->windowBackgroundColor),
            SkColorGetB(style()->windowBackgroundColor),
            SkColorGetA(style()->windowBackgroundColor)
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glfwSwapInterval(0);
        glfwSwapBuffers(_window);

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
        _visible         = glfwGetWindowAttrib(_window, GLFW_VISIBLE) != 0;
        _lastInteraction = glfwGetTime();

        for (int i = 0; i < (int) Cursor::CursorCount; ++i) {
            _cursors[i] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR + i);
        }

        windows[_window] = this;

        /// Fixes retina display-related font rendering issue
//        nvgBeginFrame(_nvgContext, _size.setX(), _size.setY(), _pixelRatio);
//        nvgEndFrame(_nvgContext);
    }

    void Window::initSkia() {
        const GrGLInterface *interface = nullptr;
        _sk_context = GrContext::MakeGL(interface).release();
        getSkiaSurface();
    }

    void Window::getSkiaSurface() {
        GrBackendRenderTargetDesc desc;
        desc.fWidth              = _width;
        desc.fHeight             = _height;
        desc.fConfig             = kSkia8888_GrPixelConfig;
        desc.fOrigin             = kBottomLeft_GrSurfaceOrigin;
        desc.fSampleCnt          = _samples;
        desc.fStencilBits        = _stencilBits;
        desc.fRenderTargetHandle = 0;  // assume default framebuffer
        _sk_surface = SkSurface::MakeFromBackendRenderTarget(_sk_context, desc, nullptr, nullptr).release();
        if (!_sk_surface) {
            SkDebugf("SkSurface::MakeFromBackendRenderTarget returned null\n");
            return;
        }
        _sk_canvas = _sk_surface->getCanvas();
    }

    void Window::attachCallbacks() {
        glfwSetCursorPosCallback(
            _window,
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
            _window,
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
            _window,
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
            _window,
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
            _window,
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
            _window,
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
            _window,
            [](GLFWwindow *w, int width, int height) {
                auto it = windows.find(w);
                if (it == windows.end()) {
                    return;
                }
                Window *s = it->second;
                s->resizeEventCallback(width, height);
            }
        );

        glfwSetWindowFocusCallback(
            _window,
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
            _window,
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
            _window,
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
            _window,
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

    GLFWwindow *Window::window() {
        return _window;
    }

    // region Title

    const std::string Window::title() const {
        return _title;
    }

    void Window::setTitle(const std::string &title) {
        if (_title != title) {
            _title = title;
            if (_window) {
                glfwSetWindowTitle(_window, _title.c_str());
            }
        }
    }

    // endregion

    // region Fullscreen

    bool Window::fullscreen() const {
        return _fullscreen;
    }

    void Window::setFullscreen(bool fullscreen) {
        if (_fullscreen != fullscreen) {
            _fullscreen = fullscreen;
            if (_window) {
                if (_fullscreen) {
                    GLFWmonitor       *monitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode *mode    = glfwGetVideoMode(monitor);
                    glfwGetWindowPos(_window, &_previousWindowX, &_previousWindowY);
                    glfwGetWindowSize(_window, &_previousWindowWidth, &_previousWindowHeight);
                    glfwSetWindowMonitor(
                        _window,
                        monitor,
                        0,
                        0,
                        mode->width,
                        mode->height,
                        GLFW_DONT_CARE
                    );
                } else {
                    glfwSetWindowMonitor(
                        _window,
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

    bool Window::minimized() const {
        return _minimized;
    }

    void Window::setMinimized(bool minimized) {
        if (_minimized != minimized) {
            _minimized = minimized;
            if (_window) {
                if (_minimized) {
                    glfwIconifyWindow(_window);
                } else {
                    glfwRestoreWindow(_window);
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
                glfwShowWindow(_window);
            } else {
                glfwHideWindow(_window);
            }
        }
    }

    // endregion

    // region Position

//    const Vector2i &Window::setWindowPosition() const {
//        return _windowPosition;
//    }

    void Window::setWindowPosition(const int &x, const int &y) {
        _windowX = x;
        _windowY = y;
        if (_window) {
            glfwSetWindowPos(_window, _windowX, _windowY);
        }
    }

    // endregion

    // region Size

//    const Vector2i &Window::setWindowSize() const {
//        return setSize();
//    }

    void Window::setWindowSize(const int &width, const int &height) {
        setSize(width, height);
        if (_window) {
            glfwSetWindowSize(_window, _width, _height);
        }
    }

    // endregion

    // region Draw

    void Window::drawAll() {
        // TODO: Cache setStyle changes
        Style *s = style().get();
        glClearColor(
            SkColorGetR(s->windowBackgroundColor),
            SkColorGetG(s->windowBackgroundColor),
            SkColorGetB(s->windowBackgroundColor),
            SkColorGetA(s->windowBackgroundColor)
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawContents();
        drawWidgets();
        glfwSwapBuffers(_window);
    }

    void Window::drawContents() {

    }

    void Window::drawWidgets() {
        if (!_visible) {
            return;
        }

        glfwMakeContextCurrent(_window);
        glfwGetFramebufferSize(_window, &_fbWidth, &_fbHeight);
        glfwGetWindowSize(_window, &_width, &_height);

        #if defined(_WIN32) || defined(__linux__)
        _width = (_width / _pixelRatio).cast<int>();
        _height = (_height / _pixelRatio).cast<int>();
        _fbWidth = (_fbWidth * _pixelRatio).cast<int>();
        _fbHeight = (_fbHeight * _pixelRatio).cast<int>();
        #else
        if (_width) {
            _pixelRatio = (float) _fbWidth / (float) _width;
        }
        #endif

        // Do setLayout
        if (YGNodeIsDirty(_yogaNode)) {
            YGNodeCalculateLayout(_yogaNode, _width, _width, YGDirectionLTR);
        }

        glViewport(0, 0, _fbWidth, _fbHeight);
        glBindSampler(0, 0);

//        nvgBeginFrame(_nvgContext, _size[0], _size[1], _pixelRatio);
//        draw(_nvgContext);
//        nvgEndFrame(_nvgContext);
        _sk_canvas->clear(style()->windowBackgroundColor);
//        _sk_canvas->clear(SK_ColorBLACK);
        render(_sk_canvas);
        _sk_canvas->flush();
    }

    // endregion

    // region Events???

    bool Window::keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (_focusPath.size() > 0) {
            for (auto it = _focusPath.rbegin() + 1; it != _focusPath.rend(); ++it) {
                if ((*it)->focused() && (*it)->keyboardEvent(key, scancode, action, modifiers)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Window::keyboardCharacterEvent(unsigned int codepoint) {
        if (_focusPath.size() > 0) {
            for (auto it = _focusPath.rbegin() + 1; it != _focusPath.rend(); ++it) {
                if ((*it)->focused() && (*it)->keyboardCharacterEvent(codepoint)) {
                    return true;
                }
            }
        }
        return false;
    }

    // endregion

    // region Callback Delegates

    void Window::windowResized(const int &width, const int &height) {
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
        setX /= _pixelRatio;
        setY /= _pixelRatio;
        #endif

        _lastInteraction = glfwGetTime();
        try {
            bool ret = false;
            x -= 1;
            y -= 2;

            if (!_dragActive) {
                auto widget = findWidget(x, y);
                if (widget != nullptr && widget->cursor() != _cursor) {
                    _cursor = widget->cursor();
                    glfwSetCursor(_window, _cursors[(int) _cursor]);
                }
            } else {
//                ret = _dragWidget->mouseDragEvent(
//                    p - _dragWidget->setParent()->absolutePosition(),
//                    p - _mousePosition,
//                    _mouseState,
//                    _modifiers
//                );
            }

            if (!ret) {
                mouseMovedPropagation(x, y, _mouseState, _modifiers);
            }

            _mouseX = x;
            _mouseY = y;

        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Window::mouseButtonEventCallback(int button, int action, int modifiers) {
        _modifiers       = modifiers;
        _lastInteraction = glfwGetTime();
        try {
//            if (_focusPath.setSize() > 1) {
//                const std::shared_ptr<Panel> panel = std::dynamic_pointer_cast<Panel>(
//                    _focusPath[_focusPath.setSize()
//                               - 2]
//                );
//                if (panel && panel->modal()) {
//                    if (!panel->contains(_mouseX, _mouseY)) {
//                        return;
//                    }
//                }
//            }

            if (action == GLFW_PRESS) {
                _mouseState |= 1 << button;
            } else {
                _mouseState &= ~(1 << button);
            }

            auto dropWidget = findWidget(_mouseX, _mouseY);
            if (_dragActive && action == GLFW_RELEASE && dropWidget != _dragWidget) {
                // TODO: Actually call a drop event, don't send the mouse event
//                _dragWidget->mouseButtonPropagation(
//                    _mousePosition - _dragWidget->setParent()->absolutePosition(),
//                    button,
//                    false,
//                    _modifiers
//                );
            }

            if (dropWidget != nullptr && dropWidget->cursor() != _cursor) {
                _cursor = dropWidget->cursor();
                glfwSetCursor(_window, _cursors[(int) _cursor]);
            }

            if (action == GLFW_PRESS && (button == GLFW_MOUSE_BUTTON_1 || button == GLFW_MOUSE_BUTTON_2)) {
                _dragWidget = findWidget(_mouseX, _mouseY);
                if (_dragWidget.get() == this) {
                    _dragWidget = nullptr;
                }
                _dragActive = _dragWidget != nullptr;
                if (!_dragActive) {
                    requestFocus(nullptr);
                }
            } else {
                _dragActive = false;
                _dragWidget = nullptr;
            }

            mouseButtonPropagation(_mouseX, _mouseY, button, action == GLFW_PRESS, _modifiers);

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
//                    if (!panel->contains(_mouseX, _mouseY)) {
//                        return;
//                    }
//                }
//            }
            mouseScrolledPropagation(_mouseX, _mouseY, x, y);
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
        glfwGetFramebufferSize(_window, &fbWidth, &fbHeight);
        glfwGetWindowSize(_window, &width, &height);

        #if defined(_WIN32) || defined(__linux__)
        setWidth /= mPixelRatio;
        setHeight /= mPixelRatio;
        #endif

        if ((fbWidth == 0 && fbHeight == 0) || (width == 0 && height == 0)) {
            return;
        }

        _fbWidth  = fbWidth;
        _fbHeight = fbHeight;
        _width    = width;
        _height   = height;

        // Set setLayout setSize
        YGNodeStyleSetWidth(_yogaNode, _width);
        YGNodeStyleSetHeight(_yogaNode, _height);

        // Get a new surface
        getSkiaSurface();

        _lastInteraction = glfwGetTime();

        try {
            windowResized(_width, _height);
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
            glfwSetWindowShouldClose(_window, windowShouldClose() ? GLFW_TRUE : GLFW_FALSE);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    // endregion

    // PANELS

    void Window::requestFocus(Widget *widget) {
        for (auto w: _focusPath) {
            if (!w->focused()) {
                continue;
            }
            w->focusEvent(false);
        }
        _focusPath.clear();
        if (widget) {
            _focusPath  = widget->path();
            for (auto w: _focusPath) {
                w->focusEvent(true);
            }
            if (auto  p = widget->panel()) {
                movePanelToFront(p);
            }
        }
    }

    void Window::disposePanel(std::shared_ptr<Panel> panel) {
//        if (std::find(_focusPath.begin(), _focusPath.end(), panel) != _focusPath.end()) {
//            _focusPath.clear();
//        }
//        if (_dragWidget == panel) {
//            _dragWidget = nullptr;
//        }
//        removeChild(panel);
    }

    void Window::centerPanel(std::shared_ptr<Panel> panel) {
//        if (panel->setSize() == Vector2i::Zero()) {
//            panel->setSize(panel->preferredSize(_nvgContext));
//            panel->performLayout(_nvgContext);
//        }
//        panel->setPosition((_size - panel->setSize()) / 2);
    }

    void Window::movePanelToFront(std::shared_ptr<Panel> panel) {
//        _children.erase(std::remove(_children.begin(), _children.end(), panel), _children.end());
//        _children.push_back(panel);
//        /* Brute force topological sort (no problem for a few panels..) */
//        bool changed = false;
//        do {
//            size_t      baseIndex = 0;
//            for (size_t index     = 0; index < _children.setSize(); ++index) {
//                if (_children[index] == panel) {
//                    baseIndex = index;
//                }
//            }
//            changed               = false;
//            for (size_t index = 0; index < _children.setSize(); ++index) {
//                auto pw = std::dynamic_pointer_cast<Popup>(_children[index]);
//                if (pw && pw->parentPanel() == panel && index < baseIndex) {
//                    movePanelToFront(pw);
//                    changed = true;
//                    break;
//                }
//            }
//        } while (changed);
    }

}