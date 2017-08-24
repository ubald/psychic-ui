#include <map>
#include <iostream>
#include <vector>
#include "opengl.hpp"
#include "Screen.hpp"
#include "Window.hpp"
#include "Popup.hpp"

#define NANOVG_GL3_IMPLEMENTATION

#include <nanovg_gl.h>

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

    std::map<GLFWwindow *, Screen *> Screen::screens;

    Screen::Screen(const std::string &title) :
        Widget::Widget(nullptr),
        _title(title) {
            _style = &Style::defaultStyle;
            _size = Vector2i(1440, 900);
    }

    Screen::~Screen() {
        screens.erase(_window);
        for (int i = 0; i < (int) Cursor::CursorCount; ++i) {
            if (_cursors[i]) {
                glfwDestroyCursor(_cursors[i]);
            }
        }
        if (_nvgContext) {
            nvgDeleteGL3(_nvgContext);
        }
        if (_window) {
            glfwDestroyWindow(_window);
        }
    }

    void Screen::open() {
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
            _window = glfwCreateWindow(_size.x(), _size.y(), _title.c_str(), nullptr, nullptr);
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

        glfwGetWindowSize(_window, &_size[0], &_size[1]);
        _pixelRatio = get_pixel_ratio(_window);
        #if defined(_WIN32) || defined(__linux__)
        if (_pixelRatio != 1 && !fullscreen)
            glfwSetWindowSize(_window, _size.x() * _pixelRatio, _size.y() * _pixelRatio);
        #endif

        glfwGetFramebufferSize(_window, &_fbSize[0], &_fbSize[1]);
        glViewport(0, 0, _fbSize[0], _fbSize[1]);
        glClearColor(
            _style->windowBackgroundColor.r(),
            _style->windowBackgroundColor.g(),
            _style->windowBackgroundColor.b(),
            _style->windowBackgroundColor.a()
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glfwSwapInterval(0);
        glfwSwapBuffers(_window);

        #if defined(__APPLE__)
        // Poll for events once before starting. This is needed to be classified as "interactive"
        glfwPollEvents();
        #endif

        // Detect framebuffer properties and set up compatible NanoVG context
        GLint nStencilBits = 0, nSamples = 0;
        glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER,
            GL_STENCIL,
            GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
            &nStencilBits
        );
        glGetIntegerv(GL_SAMPLES, &nSamples);

        int flags = 0;
        if (nStencilBits >= 8) {
            flags |= NVG_STENCIL_STROKES;
        }
        if (nSamples <= 1) {
            flags |= NVG_ANTIALIAS;
        }
        #if !defined(NDEBUG)
        flags |= NVG_DEBUG;
        #endif

        _nvgContext = nvgCreateGL3(flags);
        if (_nvgContext == nullptr) {
            throw std::runtime_error("Could not initialize NanoVG!");
        }

        // Setup Callbacks

        glfwSetCursorPosCallback(
            _window,
            [](GLFWwindow *w, double x, double y) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->cursorPosEventCallback(x, y);
            }
        );

        glfwSetMouseButtonCallback(
            _window,
            [](GLFWwindow *w, int button, int action, int modifiers) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->mouseButtonEventCallback(button, action, modifiers);
            }
        );

        glfwSetScrollCallback(
            _window,
            [](GLFWwindow *w, double x, double y) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->scrollEventCallback(x, y);
            }
        );

        glfwSetKeyCallback(
            _window,
            [](GLFWwindow *w, int key, int scancode, int action, int mods) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->keyEventCallback(key, scancode, action, mods);
            }
        );

        glfwSetCharCallback(
            _window,
            [](GLFWwindow *w, unsigned int codepoint) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->charEventCallback(codepoint);
            }
        );

        glfwSetDropCallback(
            _window,
            [](GLFWwindow *w, int count, const char **filenames) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->dropEventCallback(count, filenames);
            }
        );

        glfwSetFramebufferSizeCallback(
            _window,
            [](GLFWwindow *w, int width, int height) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->resizeEventCallback(width, height);
            }
        );

        glfwSetWindowFocusCallback(
            _window,
            [](GLFWwindow *w, int focused) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->focusEventCallback(focused);
            }
        );

        glfwSetWindowIconifyCallback(
            _window,
            [](GLFWwindow *w, int iconified) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->iconifyEventCallback(iconified);
            }
        );

        glfwSetWindowCloseCallback(
            _window,
            [](GLFWwindow *w) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->closeEventCallback();
            }
        );

        // Continue rendering while event polling is frozen by a resize
        glfwSetWindowRefreshCallback(
            _window,
            [](GLFWwindow *w) {
                auto it = screens.find(w);
                if (it == screens.end()) {
                    return;
                }
                Screen *s = it->second;
                s->drawAll();
            }
        );

        // Be ready to start
        _visible = glfwGetWindowAttrib(_window, GLFW_VISIBLE) != 0;
        _lastInteraction = glfwGetTime();

        for (int i = 0; i < (int) Cursor::CursorCount; ++i) {
            _cursors[i] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR + i);
        }

        screens[_window] = this;

        /// Fixes retina display-related font rendering issue
        nvgBeginFrame(_nvgContext, _size.x(), _size.y(), _pixelRatio);
        nvgEndFrame(_nvgContext);
    }

    GLFWwindow *Screen::window() {
        return _window;
    }

    // TITLE

    const std::string Screen::title() const {
        return _title;
    }

    void Screen::setTitle(const std::string &title) {
        if (_title != title) {
            _title = title;
            if (_window) {
                glfwSetWindowTitle(_window, _title.c_str());
            }
        }
    }

    // FULLSCREEN

    bool Screen::fullscreen() const {
        return _fullscreen;
    }

    void Screen::setFullscreen(bool fullscreen) {
        if (_fullscreen != fullscreen) {
            _fullscreen = fullscreen;
            if (_window) {
                if (_fullscreen) {
                    GLFWmonitor       *monitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode *mode    = glfwGetVideoMode(monitor);
                    glfwGetWindowPos(_window, &_previousScreenPosition[0], &_previousScreenPosition[1]);
                    glfwGetWindowSize(_window, &_previousScreenSize[0], &_previousScreenSize[1]);
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
                        _previousScreenPosition[0],
                        _previousScreenPosition[1],
                        _previousScreenSize[0],
                        _previousScreenSize[1],
                        GLFW_DONT_CARE
                    );
                }
            }
        }
    }

    // MINIMIZED

    bool Screen::minimized() const {
        return _minimized;
    }

    void Screen::setMinimized(bool minimized) {
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
    
    // VISIBLE

    void Screen::setVisible(bool visible) {
        if (_visible != visible) {
            _visible = visible;
            if (_visible) {
                glfwShowWindow(_window);
            } else {
                glfwHideWindow(_window);
            }
        }
    }

    // POSITION

    const Vector2i &Screen::windowPosition() const {
        return _windowPosition;
    }

    void Screen::setWindowPosition(const Vector2i &windowPosition) {
        _windowPosition = windowPosition;
        if (_window) {
            glfwSetWindowPos(_window, _windowPosition[0], _windowPosition[1]);
        }
    }

    // SIZE

    const Vector2i &Screen::windowSize() const {
        return size();
    }

    void Screen::setWindowSize(const Vector2i &size) {
        setSize(size);
        if (_window) {
            glfwSetWindowSize(_window, _size[0], _size[1]);
        }
    }

    // DRAW

    void Screen::drawAll() {
        glClearColor(
            style()->windowBackgroundColor.r(),
            style()->windowBackgroundColor.g(),
            style()->windowBackgroundColor.b(),
            style()->windowBackgroundColor.a()
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawContents();
        drawWidgets();
        glfwSwapBuffers(_window);
    }

    void Screen::drawContents() {

    }

    void Screen::drawWidgets() {
        if (!_visible) {
            return;
        }

        glfwMakeContextCurrent(_window);
        glfwGetFramebufferSize(_window, &_fbSize[0], &_fbSize[1]);
        glfwGetWindowSize(_window, &_size[0], &_size[1]);

        #if defined(_WIN32) || defined(__linux__)
        _size = (_size / _pixelRatio).cast<int>();
        _fbSize = (_fbSize * _pixelRatio).cast<int>();
        #else
        if (_size[0]) {
            _pixelRatio = (float) _fbSize[0] / (float) _size[0];
        }
        #endif

        glViewport(0, 0, _fbSize[0], _fbSize[1]);
        glBindSampler(0, 0);
        nvgBeginFrame(_nvgContext, _size[0], _size[1], _pixelRatio);
        draw(_nvgContext);
        nvgEndFrame(_nvgContext);
    }

    bool Screen::keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (_focusPath.size() > 0) {
            for (auto it = _focusPath.rbegin() + 1; it != _focusPath.rend(); ++it) {
                if ((*it)->focused() && (*it)->keyboardEvent(key, scancode, action, modifiers)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Screen::keyboardCharacterEvent(unsigned int codepoint) {
        if (_focusPath.size() > 0) {
            for (auto it = _focusPath.rbegin() + 1; it != _focusPath.rend(); ++it) {
                if ((*it)->focused() && (*it)->keyboardCharacterEvent(codepoint)) {
                    return true;
                }
            }
        }
        return false;
    }

    // WINDOW DELEGATE

    void Screen::windowResized(const Vector2i &size) {
        // std::cout << "Resized" << std::endl;
    }

    void Screen::windowActivated() {
        // std::cout << "Activated" << std::endl;
    }

    void Screen::windowDeactivated() {
        // std::cout << "Deactivated" << std::endl;
    }

    void Screen::windowMinimized() {
        // std::cout << "Minimized" << std::endl;
    }

    void Screen::windowRestored() {
        // std::cout << "Restored" << std::endl;
    }

    bool Screen::windowShouldClose() {
        return true;
    }

    // CALLBACKS

    void Screen::cursorPosEventCallback(double x, double y) {
        Vector2i p((int) x, (int) y);

        #if defined(_WIN32) || defined(__linux__)
        p /= _pixelRatio;
        #endif

        _lastInteraction = glfwGetTime();
        try {
            bool ret = false;
            p -= Vector2i(1, 2);

            if (!_dragActive) {
                Widget *widget = findWidget(p);
                if (widget != nullptr && widget->cursor() != _cursor) {
                    _cursor = widget->cursor();
                    glfwSetCursor(_window, _cursors[(int) _cursor]);
                }
            } else {
                ret = _dragWidget->mouseDragEvent(
                    p - _dragWidget->parent()->absolutePosition(),
                    p - _mousePosition,
                    _mouseState,
                    _modifiers
                );
            }

            if (!ret) {
                mouseMovedPropagation(p, _mouseState, _modifiers);
            }

            _mousePosition = p;

        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Screen::mouseButtonEventCallback(int button, int action, int modifiers) {
        _modifiers       = modifiers;
        _lastInteraction = glfwGetTime();
        try {
            if (_focusPath.size() > 1) {
                const Window *window = dynamic_cast<Window *>(_focusPath[_focusPath.size() - 2]);
                if (window && window->modal()) {
                    if (!window->contains(_mousePosition)) {
                        return;
                    }
                }
            }

            if (action == GLFW_PRESS) {
                _mouseState |= 1 << button;
            } else {
                _mouseState &= ~(1 << button);
            }

            auto dropWidget = findWidget(_mousePosition);
            if (_dragActive && action == GLFW_RELEASE && dropWidget != _dragWidget) {
                // TODO: Actually call a drop event, don't send the mouse event
//                _dragWidget->mouseButtonPropagation(
//                    _mousePosition - _dragWidget->parent()->absolutePosition(),
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
                _dragWidget = findWidget(_mousePosition);
                if (_dragWidget == this) {
                    _dragWidget = nullptr;
                }
                _dragActive = _dragWidget != nullptr;
                if (!_dragActive) {
                    updateFocus(nullptr);
                }
            } else {
                _dragActive = false;
                _dragWidget = nullptr;
            }

            mouseButtonPropagation(_mousePosition, button, action == GLFW_PRESS, _modifiers);

        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Screen::scrollEventCallback(double x, double y) {
        _lastInteraction = glfwGetTime();
        try {
            if (_focusPath.size() > 1) {
                const Window *window = dynamic_cast<Window *>(_focusPath[_focusPath.size() - 2]);
                if (window && window->modal()) {
                    if (!window->contains(_mousePosition)) {
                        return;
                    }
                }
            }
            mouseScrolledPropagation(_mousePosition, Vector2f(x, y));
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Screen::keyEventCallback(int key, int scancode, int action, int mods) {
        _lastInteraction = glfwGetTime();
        try {
            keyboardEvent(key, scancode, action, mods);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Screen::charEventCallback(unsigned int codepoint) {
        _lastInteraction = glfwGetTime();
        try {
            keyboardCharacterEvent(codepoint);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Screen::dropEventCallback(int count, const char **filenames) {
        std::vector<std::string> arg(count);
        for (int i = 0; i < count; ++i) {
            arg[i] = filenames[i];
        }
        try {
            dropEvent(arg);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Screen::resizeEventCallback(int /*width*/, int /*height*/) {
        Vector2i fbSize, size;
        glfwGetFramebufferSize(_window, &fbSize[0], &fbSize[1]);
        glfwGetWindowSize(_window, &size[0], &size[1]);

        #if defined(_WIN32) || defined(__linux__)
        size /= mPixelRatio;
        #endif

        if (_fbSize == Vector2i(0, 0) || _size == Vector2i(0, 0)) {
            return;
        }

        _fbSize = fbSize;
        _size   = size;

        _lastInteraction = glfwGetTime();

        try {
            windowResized(_size);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    void Screen::focusEventCallback(int focused) {
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

    void Screen::iconifyEventCallback(int iconified) {
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

    void Screen::closeEventCallback() {
        _lastInteraction = glfwGetTime();
        try {
            glfwSetWindowShouldClose(_window, windowShouldClose() ? GLFW_TRUE : GLFW_FALSE);
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        }
    }

    // INTERNAL WINDOWS

    void Screen::updateFocus(Widget *widget) {
        for (auto w: _focusPath) {
            if (!w->focused()) {
                continue;
            }
            w->focusEvent(false);
        }
        _focusPath.clear();
        Widget *window = nullptr;
        while (widget) {
            _focusPath.push_back(widget);
            if (dynamic_cast<Window *>(widget)) {
                window = widget;
            }
            widget = widget->parent();
        }
        for (auto it = _focusPath.rbegin(); it != _focusPath.rend(); ++it) {
            (*it)->focusEvent(true);
        }

        if (window) {
            moveWindowToFront((Window *) window);
        }
    }

    void Screen::disposeWindow(Window *window) {
        if (std::find(_focusPath.begin(), _focusPath.end(), window) != _focusPath.end()) {
            _focusPath.clear();
        }
        if (_dragWidget == window) {
            _dragWidget = nullptr;
        }
        removeChild(window);
    }

    void Screen::centerWindow(Window *window) {
        if (window->size() == Vector2i::Zero()) {
            window->setSize(window->preferredSize(_nvgContext));
            window->performLayout(_nvgContext);
        }
        window->setPosition((_size - window->size()) / 2);
    }

    void Screen::moveWindowToFront(Window *window) {
        _children.erase(std::remove(_children.begin(), _children.end(), window), _children.end());
        _children.push_back(window);
        /* Brute force topological sort (no problem for a few windows..) */
        bool changed = false;
        do {
            size_t      baseIndex = 0;
            for (size_t index     = 0; index < _children.size(); ++index) {
                if (_children[index] == window) {
                    baseIndex = index;
                }
            }
            changed               = false;
            for (size_t index = 0; index < _children.size(); ++index) {
                Popup *pw = dynamic_cast<Popup *>(_children[index]);
                if (pw && pw->parentWindow() == window && index < baseIndex) {
                    moveWindowToFront(pw);
                    changed = true;
                    break;
                }
            }
        } while (changed);
    }

}