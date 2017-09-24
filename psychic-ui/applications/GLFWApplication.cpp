#ifdef WITH_GLFW

#include <SDL_keycode.h>
#include "GLFWApplication.hpp"

namespace psychic_ui {

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

    #if defined(PSYCHIC_UI_GLAD)
    static bool gladInitialized = false;
    #endif

    // APPLICATION

    std::unordered_map<GLFWwindow *, std::unique_ptr<GLFWSystemWindow>> GLFWApplication::glfwWindows{};

    void GLFWApplication::init() {
        #if defined(__APPLE__)
        disable_saved_application_state_osx();
        #endif

        glfwSetErrorCallback(
            [](int error, const char *descr) {
                if (error == GLFW_NOT_INITIALIZED) {
                    return;
                }
                std::cerr << "GLFW error " << error << ": " << descr << std::endl;
            }
        );

        if (!glfwInit()) {
            throw std::runtime_error("Could not initialize GLFW!");
        }

        std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;

        glfwSetTime(0);
    }

    void GLFWApplication::mainloop() {
        if (running) {
            throw std::runtime_error("Main loop is already running!");
        }

        running = true;

        while (running) {
            glfwPollEvents();

            int       numScreens = 0;
            for (auto &kv : glfwWindows) {
                if (kv.second->render()) {
                    numScreens++;
                }
            }

            // Cleanup dirty managers
            for (auto &kv : glfwWindows) {
                kv.second->window()->styleManager()->setValid();
            }

            if (numScreens == 0) {
                running = false;
                break;
            }
        }
    }

    void GLFWApplication::open(std::shared_ptr<Window> window) {
        auto systemWindow = std::make_unique<GLFWSystemWindow>(this, window);
        glfwWindows[systemWindow->glfwWindow()] = std::move(systemWindow);
    }

    void GLFWApplication::close(std::shared_ptr<Window> window) {
        // TODO: Use erase delete
        auto res = std::find_if(glfwWindows.cbegin(), glfwWindows.cend(), [&window](auto &it) { return it.second->_window == window; });
        if (res != glfwWindows.cend()) {
            glfwWindows.erase(res->first);
        }
    }

    void GLFWApplication::shutdown() {
        glfwTerminate();
    }

    // WINDOW

    GLFWSystemWindow::GLFWSystemWindow(GLFWApplication *application, std::shared_ptr<Window> window) :
        SystemWindow(application, window), _glfwApplication(application) {

        // OPENGL INIT

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 8);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 0);
        glfwWindowHint(GLFW_STENCIL_BITS, _stencilBits);
        glfwWindowHint(GLFW_SAMPLES, _samples);

        // WINDOW FLAGS

        glfwWindowHint(GLFW_VISIBLE, window->getVisible() ? GL_TRUE : GL_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, window->getResizable() ? GL_TRUE : GL_FALSE);
        glfwWindowHint(GLFW_DECORATED, window->getDecorated() ? GL_TRUE : GL_FALSE);

        // CREATE WINDOW

        if (window->getFullscreen()) {
            GLFWmonitor       *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode    = glfwGetVideoMode(monitor);
            _glfwWindow = glfwCreateWindow(mode->width, mode->height, window->getTitle().c_str(), monitor, nullptr);
        } else {
            _glfwWindow = glfwCreateWindow(_width, _height, window->getTitle().c_str(), nullptr, nullptr);
        }

        if (!_glfwWindow) {
            throw std::runtime_error("Could not create an OpenGL context!");
        }

        // GATHER INFORMATION

        glfwGetWindowPos(_glfwWindow, &_x, &_y);
        glfwGetWindowSize(_glfwWindow, &_width, &_height);
        _pixelRatio = get_pixel_ratio(_glfwWindow);
        #if defined(_WIN32) || defined(__linux__)
        if (_pixelRatio != 1 && !_fullscreen)
            glfwSetWindowSize(_glfwWindow, _width * _pixelRatio, _height * _pixelRatio);
        #endif

        // GL CONTEXT

        glfwMakeContextCurrent(_glfwWindow);

        #if defined(PSYCHIC_UI_GLAD)
        if (!gladInitialized) {
            gladInitialized = true;
            if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
                throw std::runtime_error("Could not initialize GLAD!");
            glGetError();
        }
        #endif

        //glfwGetFramebufferSize(_glfwWindow, &_fbWidth, &_fbHeight);
        //glViewport(0, 0, _fbWidth, _fbHeight);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glfwSwapInterval(0);
        glfwSwapBuffers(_glfwWindow);

        #if defined(__APPLE__)
        // Poll for events once before starting. This is needed to be classified as "interactive"
        glfwPollEvents();
        #endif

        // Get Some info back about the framebuffer (in case its different from what we set?)
        glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER,
            GL_STENCIL,
            GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
            &_stencilBits
        );
        glGetIntegerv(GL_SAMPLES, &_samples);

        // Setup Callbacks
        attachCallbacks();

        // Be ready to start
        window->setVisible(glfwGetWindowAttrib(_glfwWindow, GLFW_VISIBLE) != 0);
        _lastInteraction = glfwGetTime();

        // Create system cursors
        for (int i = 0; i < 6; ++i) {
            _cursors[i] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR + i);
        }

        window->open(this);
    }

    GLFWSystemWindow::~GLFWSystemWindow() {
        for (auto &_cursor : _cursors) {
            if (_cursor) {
                glfwDestroyCursor(_cursor);
            }
        }

        glfwDestroyWindow(_glfwWindow);
    };

    GLFWwindow *GLFWSystemWindow::glfwWindow() const {
        return _glfwWindow;
    }

    bool GLFWSystemWindow::render() {
        if (!_window->getVisible()) {
            return false;
        } else if (glfwWindowShouldClose(_glfwWindow)) {
            _window->setVisible(false);
            return false;
        }

        //glfwMakeContextCurrent(_glfwWindow);
        //glClearColor(0, 0, 0, 0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        //glfwGetFramebufferSize(_glfwWindow, &_fbWidth, &_fbHeight);
        //glfwGetWindowSize(_glfwWindow, &_width, &_height);
        //glViewport(0, 0, _fbWidth, _fbHeight);
        //glBindSampler(0, 0);

        //#if defined(_WIN32) || defined(__linux__)
        //_width = (int)(_width / _pixelRatio);
        //_height =(int)(_height / _pixelRatio);
        //_fbWidth = (int)(_fbWidth * _pixelRatio);
        //_fbHeight = (int)(_fbHeight * _pixelRatio);
        //#else
        //if (_width) {
        //    _pixelRatio = (float) _fbWidth / (float) _width;
        //}
        //#endif

        _window->drawAll();

        glfwSwapBuffers(_glfwWindow);

        return true;
    }

    void GLFWSystemWindow::setSize(int width, int height) {
        _width  = width;
        _height = height;
        glfwSetWindowSize(_glfwWindow, _width, _height);
    }

    void GLFWSystemWindow::setPosition(int x, int y) {
        _x = x;
        _y = y;
        glfwSetWindowPos(_glfwWindow, _x, _y);
    }

    void GLFWSystemWindow::setTitle(const std::string &title) {
        glfwSetWindowTitle(_glfwWindow, title.c_str());
    }

    void GLFWSystemWindow::setFullscreen(bool fullscreen) {
        if (fullscreen) {
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

    bool GLFWSystemWindow::getMinimized() const {
        return _minimized;
    }

    void GLFWSystemWindow::setMinimized(bool minimized) {
        if (_minimized != minimized) {
            _minimized = minimized;
            if (_minimized) {
                glfwIconifyWindow(_glfwWindow);
            } else {
                glfwRestoreWindow(_glfwWindow);
            }
        }
    }

    bool GLFWSystemWindow::getMaximized() const {
        return _maximized;
    }

    void GLFWSystemWindow::setMaximized(bool maximized) {
        if (_maximized != maximized) {
            _maximized = maximized;
            if (_maximized) {
                glfwMaximizeWindow(_glfwWindow);
            } else {
                glfwRestoreWindow(_glfwWindow);
            }
        }
    }

    void GLFWSystemWindow::setVisible(bool visible) {
        if (visible) {
            glfwHideWindow(_glfwWindow);
        } else {
            glfwShowWindow(_glfwWindow);
        }
    }

    void GLFWSystemWindow::setCursor(int cursor) {
        glfwSetCursor(_glfwWindow, _cursors[cursor]);
    }

    void GLFWSystemWindow::startDrag() {
        _windowDragMouseX = _mouseX;
        _windowDragMouseY = _mouseY;
        _dragging         = true;
    }

    void GLFWSystemWindow::stopDrag() {
        _dragging         = false;
        _windowDragMouseX = 0;
        _windowDragMouseY = 0;
    }

    void GLFWSystemWindow::attachCallbacks() {
        glfwSetCursorPosCallback(
            _glfwWindow, [](GLFWwindow *w, double x, double y) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->cursorPosEventCallback(x, y);
            }
        );

        glfwSetMouseButtonCallback(
            _glfwWindow, [](GLFWwindow *w, int button, int action, int modifiers) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->mouseButtonEventCallback(button, action, modifiers);
            }
        );

        glfwSetScrollCallback(
            _glfwWindow, [](GLFWwindow *w, double x, double y) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->scrollEventCallback(x, y);
            }
        );

        glfwSetKeyCallback(
            _glfwWindow, [](GLFWwindow *w, int key, int scancode, int action, int mods) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->keyEventCallback(key, scancode, action, mods);
            }
        );

        glfwSetCharCallback(
            _glfwWindow, [](GLFWwindow *w, unsigned int codepoint) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->charEventCallback(codepoint);
            }
        );

        glfwSetDropCallback(
            _glfwWindow, [](GLFWwindow *w, int count, const char **filenames) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->dropEventCallback(count, filenames);
            }
        );

        glfwSetFramebufferSizeCallback(
            _glfwWindow, [](GLFWwindow *w, int width, int height) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->resizeEventCallback(width, height);
            }
        );

        glfwSetWindowPosCallback(
            _glfwWindow, [](GLFWwindow *w, int x, int y) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->positionEventCallback(x, y);
            }
        );

        glfwSetWindowFocusCallback(
            _glfwWindow, [](GLFWwindow *w, int focused) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->focusEventCallback(focused);
            }
        );

        glfwSetWindowIconifyCallback(
            _glfwWindow, [](GLFWwindow *w, int iconified) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->iconifyEventCallback(iconified);
            }
        );

        glfwSetWindowCloseCallback(
            _glfwWindow, [](GLFWwindow *w) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->closeEventCallback();
            }
        );

        // Continue rendering while event polling is frozen by a resize
        glfwSetWindowRefreshCallback(
            _glfwWindow, [](GLFWwindow *w) {
                auto it = GLFWApplication::glfwWindows.find(w);
                if (it == GLFWApplication::glfwWindows.cend()) { return; }
                it->second->render();
            }
        );
    }

    void GLFWSystemWindow::cursorPosEventCallback(double x, double y) {
        #if defined(_WIN32) || defined(__linux__)
        x /= _pixelRatio;
        y /= _pixelRatio;
        #endif

        _lastInteraction = glfwGetTime();

        // Weird but the cursor seem better aligned like this
        // At least on a mac...
        _mouseX = static_cast<int>(x) - 1;
        _mouseY = static_cast<int>(y) - 2;

        // Handle window dragging
        if (_dragging) {
            _windowDragOffsetX = _mouseX - _windowDragMouseX;
            _windowDragOffsetY = _mouseY - _windowDragMouseY;
            glfwSetWindowPos(_glfwWindow, _x + _windowDragOffsetX, _y + _windowDragOffsetY);
        }

        _window->mouseMoved(_mouseX, _mouseY, _mouseState, _modifiers, false);
    }

    void GLFWSystemWindow::mouseButtonEventCallback(int button, int action, int modifiers) {
        _modifiers       = modifiers;
        _lastInteraction = glfwGetTime();

        int btn = 0;
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            btn = 1;
        } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            btn = 2;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            btn = 3;
        } else {
            return;
        }

        if (action == GLFW_PRESS) {
            _mouseState |= 1 << btn;
        } else {
            _mouseState &= ~(1 << btn);
        }

        _window->mouseButton(_mouseX, _mouseY, btn, action == GLFW_PRESS, _modifiers);
        if (action == GLFW_PRESS) {
            _window->mouseDown(_mouseX, _mouseY, btn, _modifiers);
        } else {
            _window->click(_mouseX, _mouseY, btn, _modifiers);
            _window->mouseUp(_mouseX, _mouseY, btn, _modifiers);
        }
    }

    void GLFWSystemWindow::scrollEventCallback(double x, double y) {
        _lastInteraction = glfwGetTime();
        _window->mouseScrolled(_mouseX, _mouseY, x, y);
    }

    /**
     *
     * @param key GLFW_KEY_*
     * @param scancode Unique per platform, safe to save but different across platforms
     * @param action GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
     * @param mods
     */
    void GLFWSystemWindow::keyEventCallback(int key, int scancode, int action, int mods) {
        _lastInteraction = glfwGetTime();
        Key k = mapKey(key);
        // TODO: Key mods map
        switch (action) {
            case GLFW_PRESS:
                _window->keyDown(k);
                break;

            case GLFW_REPEAT:
                _window->keyRepeat(k);
                break;

            case GLFW_RELEASE:
                _window->keyUp(k);
                break;

            default:break;
        }
    }

    /**
     *
     * @param codepoint Unicode code points
     */
    void GLFWSystemWindow::charEventCallback(unsigned int codepoint) {
        _lastInteraction = glfwGetTime();
        _window->keyboardCharacterEvent(codepoint);
    }

    void GLFWSystemWindow::dropEventCallback(int count, const char **filenames) {
        std::vector<std::string> arg(count);

        for (int i = 0; i < count; ++i) {
            arg[i] = filenames[i];
        }
        _window->dropEvent(arg);
    }

    void GLFWSystemWindow::resizeEventCallback(int /*setWidth*/, int /*setHeight*/) {
        //glfwGetFramebufferSize(_glfwWindow, &_fbWidth, &_fbHeight);
        glfwGetWindowSize(_glfwWindow, &_width, &_height);

        #if defined(_WIN32) || defined(__linux__)
        _width /= _pixelRatio;
        _height /= _pixelRatio;
        #endif

        if (/*(_fbWidth == 0 && _fbHeight == 0) ||*/ (_width == 0 && _height == 0)) {
            return;
        }

        _lastInteraction = glfwGetTime();

        _window->windowResized(_width, _height);
    }

    void GLFWSystemWindow::positionEventCallback(int x, int y) {
        _x               = x;
        _y               = y;
        _lastInteraction = glfwGetTime();
        _window->windowMoved(_x, _y);
    }

    void GLFWSystemWindow::focusEventCallback(int focused) {
        _lastInteraction = glfwGetTime();
        _focused         = focused == 1;
        if (_focused) {
            _window->windowActivated();
        } else {
            _window->windowDeactivated();
        }
    }

    void GLFWSystemWindow::iconifyEventCallback(int iconified) {
        _lastInteraction = glfwGetTime();
        _minimized       = iconified == 1;
        if (_minimized) {
            _window->windowMinimized();
        } else {
            _window->windowRestored();
        }
    }

    void GLFWSystemWindow::closeEventCallback() {
        _lastInteraction = glfwGetTime();
        glfwSetWindowShouldClose(_glfwWindow, _window->windowShouldClose() ? GLFW_TRUE : GLFW_FALSE);
    }

    Key GLFWSystemWindow::mapKey(int keycode) {
        switch (keycode) {
            case GLFW_KEY_SPACE: return Key::SPACE;
            case GLFW_KEY_APOSTROPHE: return Key::APOSTROPHE;
            case GLFW_KEY_COMMA: return Key::COMMA;
            case GLFW_KEY_MINUS: return Key::MINUS;
            case GLFW_KEY_PERIOD: return Key::PERIOD;
            case GLFW_KEY_SLASH: return Key::SLASH;
            case GLFW_KEY_0: return Key::ZERO;
            case GLFW_KEY_1: return Key::ONE;
            case GLFW_KEY_2: return Key::TWO;
            case GLFW_KEY_3: return Key::THREE;
            case GLFW_KEY_4: return Key::FOUR;
            case GLFW_KEY_5: return Key::FIVE;
            case GLFW_KEY_6: return Key::SIX;
            case GLFW_KEY_7: return Key::SEVEN;
            case GLFW_KEY_8: return Key::EIGHT;
            case GLFW_KEY_9: return Key::NINE;
            case GLFW_KEY_SEMICOLON: return Key::SEMICOLON;
            case GLFW_KEY_EQUAL: return Key::EQUAL;
            case GLFW_KEY_A: return Key::A;
            case GLFW_KEY_B: return Key::B;
            case GLFW_KEY_C: return Key::C;
            case GLFW_KEY_D: return Key::D;
            case GLFW_KEY_E: return Key::E;
            case GLFW_KEY_F: return Key::F;
            case GLFW_KEY_G: return Key::G;
            case GLFW_KEY_H: return Key::H;
            case GLFW_KEY_I: return Key::I;
            case GLFW_KEY_J: return Key::J;
            case GLFW_KEY_K: return Key::K;
            case GLFW_KEY_L: return Key::L;
            case GLFW_KEY_M: return Key::M;
            case GLFW_KEY_N: return Key::N;
            case GLFW_KEY_O: return Key::O;
            case GLFW_KEY_P: return Key::P;
            case GLFW_KEY_Q: return Key::Q;
            case GLFW_KEY_R: return Key::R;
            case GLFW_KEY_S: return Key::S;
            case GLFW_KEY_T: return Key::T;
            case GLFW_KEY_U: return Key::U;
            case GLFW_KEY_V: return Key::V;
            case GLFW_KEY_W: return Key::W;
            case GLFW_KEY_X: return Key::X;
            case GLFW_KEY_Y: return Key::Y;
            case GLFW_KEY_Z: return Key::Z;
            case GLFW_KEY_LEFT_BRACKET: return Key::LEFT_BRACKET;
            case GLFW_KEY_BACKSLASH: return Key::BACKSLASH;
            case GLFW_KEY_RIGHT_BRACKET: return Key::RIGHT_BRACKET;
            case GLFW_KEY_GRAVE_ACCENT: return Key::GRAVE_ACCENT;
            case GLFW_KEY_WORLD_1: return Key::WORLD_1;
            case GLFW_KEY_WORLD_2: return Key::WORLD_2;
            case GLFW_KEY_ESCAPE: return Key::ESCAPE;
            case GLFW_KEY_ENTER: return Key::ENTER;
            case GLFW_KEY_TAB: return Key::TAB;
            case GLFW_KEY_BACKSPACE: return Key::BACKSPACE;
            case GLFW_KEY_INSERT: return Key::INSERT;
            case GLFW_KEY_DELETE: return Key::DELETE;
            case GLFW_KEY_RIGHT: return Key::RIGHT;
            case GLFW_KEY_LEFT: return Key::LEFT;
            case GLFW_KEY_DOWN: return Key::DOWN;
            case GLFW_KEY_UP: return Key::UP;
            case GLFW_KEY_PAGE_UP: return Key::PAGE_UP;
            case GLFW_KEY_PAGE_DOWN: return Key::PAGE_DOWN;
            case GLFW_KEY_HOME: return Key::HOME;
            case GLFW_KEY_END: return Key::END;
            case GLFW_KEY_CAPS_LOCK: return Key::CAPS_LOCK;
            case GLFW_KEY_SCROLL_LOCK: return Key::SCROLL_LOCK;
            case GLFW_KEY_NUM_LOCK: return Key::NUM_LOCK;
            case GLFW_KEY_PRINT_SCREEN: return Key::PRINT_SCREEN;
            case GLFW_KEY_PAUSE: return Key::PAUSE;
            case GLFW_KEY_F1: return Key::F1;
            case GLFW_KEY_F2: return Key::F2;
            case GLFW_KEY_F3: return Key::F3;
            case GLFW_KEY_F4: return Key::F4;
            case GLFW_KEY_F5: return Key::F5;
            case GLFW_KEY_F6: return Key::F6;
            case GLFW_KEY_F7: return Key::F7;
            case GLFW_KEY_F8: return Key::F8;
            case GLFW_KEY_F9: return Key::F9;
            case GLFW_KEY_F10: return Key::F10;
            case GLFW_KEY_F11: return Key::F11;
            case GLFW_KEY_F12: return Key::F12;
            case GLFW_KEY_F13: return Key::F13;
            case GLFW_KEY_F14: return Key::F14;
            case GLFW_KEY_F15: return Key::F15;
            case GLFW_KEY_F16: return Key::F16;
            case GLFW_KEY_F17: return Key::F17;
            case GLFW_KEY_F18: return Key::F18;
            case GLFW_KEY_F19: return Key::F19;
            case GLFW_KEY_F20: return Key::F20;
            case GLFW_KEY_F21: return Key::F21;
            case GLFW_KEY_F22: return Key::F22;
            case GLFW_KEY_F23: return Key::F23;
            case GLFW_KEY_F24: return Key::F24;
            case GLFW_KEY_F25: return Key::F25;
            case GLFW_KEY_KP_0: return Key::KP_0;
            case GLFW_KEY_KP_1: return Key::KP_1;
            case GLFW_KEY_KP_2: return Key::KP_2;
            case GLFW_KEY_KP_3: return Key::KP_3;
            case GLFW_KEY_KP_4: return Key::KP_4;
            case GLFW_KEY_KP_5: return Key::KP_5;
            case GLFW_KEY_KP_6: return Key::KP_6;
            case GLFW_KEY_KP_7: return Key::KP_7;
            case GLFW_KEY_KP_8: return Key::KP_8;
            case GLFW_KEY_KP_9: return Key::KP_9;
            case GLFW_KEY_KP_DECIMAL: return Key::KP_DECIMAL;
            case GLFW_KEY_KP_DIVIDE: return Key::KP_DIVIDE;
            case GLFW_KEY_KP_MULTIPLY: return Key::KP_MULTIPLY;
            case GLFW_KEY_KP_SUBTRACT: return Key::KP_SUBTRACT;
            case GLFW_KEY_KP_ADD: return Key::KP_ADD;
            case GLFW_KEY_KP_ENTER: return Key::KP_ENTER;
            case GLFW_KEY_KP_EQUAL: return Key::KP_EQUAL;
            case GLFW_KEY_LEFT_SHIFT: return Key::LEFT_SHIFT;
            case GLFW_KEY_LEFT_CONTROL: return Key::LEFT_CONTROL;
            case GLFW_KEY_LEFT_ALT: return Key::LEFT_ALT;
            case GLFW_KEY_LEFT_SUPER: return Key::LEFT_SUPER;
            case GLFW_KEY_RIGHT_SHIFT: return Key::RIGHT_SHIFT;
            case GLFW_KEY_RIGHT_CONTROL: return Key::RIGHT_CONTROL;
            case GLFW_KEY_RIGHT_ALT: return Key::RIGHT_ALT;
            case GLFW_KEY_RIGHT_SUPER: return Key::RIGHT_SUPER;
            case GLFW_KEY_MENU: return Key::MENU;
            case GLFW_KEY_UNKNOWN:
            default:return Key::UNKNOWN;
        }
    }
}

#endif
