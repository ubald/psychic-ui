#ifdef WITH_SDL2

#include <iostream>
#include "SDL2Application.hpp"

namespace psychic_ui {

    /**
    * Log an SDL error with some error message to the output stream of our choice
    * @param os The output stream to write the message to
    * @param msg The error message to write, format will be msg error: SDL_GetError()
    */
    void logSDLError(const std::string &msg) {
        std::cerr << msg << " error: " << SDL_GetError() << std::endl;
    }

    static int resizingEventWatcher(void *data, SDL_Event *event) {
        if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
            auto res = SDL2Application::sdl2Windows.find(event->window.windowID);
            if (res != SDL2Application::sdl2Windows.cend()) {
                res->second->liveResize(event->window.data1, event->window.data2);
            }
        }
        return 0;
    }

    // APPLICATION

    std::unordered_map<unsigned int, std::unique_ptr<SDL2SystemWindow>> SDL2Application::sdl2Windows{};

    void SDL2Application::init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            logSDLError("SDL_Init");
            throw std::runtime_error("Could not initialize SDL2!");
        }

        // Event watch, cheat for live resize
        SDL_AddEventWatch(resizingEventWatcher, nullptr);
    }

    void SDL2Application::mainloop() {
        if (running) {
            throw std::runtime_error("Main loop is already running!");
        }

        running = true;

        while (running) {
            sdl2PollEvents();

            int       numScreens = 0;
            for (auto &kv : sdl2Windows) {
                if (kv.second->render()) {
                    numScreens++;
                }
            }

            // Cleanup dirty managers
            for (auto &kv : sdl2Windows) {
                kv.second->window()->styleManager()->setValid();
            }

            if (numScreens == 0) {
                running = false;
                break;
            }
        }
    }

    void SDL2Application::open(std::shared_ptr<Window> window) {
        auto systemWindow = std::make_unique<SDL2SystemWindow>(this, window);
        sdl2Windows[SDL_GetWindowID(systemWindow->_sdl2Window)] = std::move(systemWindow);
    }

    void SDL2Application::close(std::shared_ptr<Window> window) {
    }

    void SDL2Application::shutdown() {
        SDL_Quit();
    }

    void SDL2Application::sdl2PollEvents() {
        SDL_Event e{};
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: {
                    running = false;
                    break;
                }
                case SDL_WINDOWEVENT:
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                case SDL_TEXTINPUT:
                case SDL_TEXTEDITING:
                case SDL_MOUSEMOTION:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEWHEEL: {
                    auto res = sdl2Windows.find(e.window.windowID);
                    if (res == sdl2Windows.cend()) {
                        std::cerr << "Received an event for an unregistered window" << std::endl;
                        break;
                    }
                    res->second->handleEvent(e);
                    break;
                }
                default:
                    break;

            }
        }
    }

    // WINDOW

    SDL2SystemWindow::SDL2SystemWindow(SDL2Application *application, std::shared_ptr<Window> window) :
        SystemWindow(application, window), _sdl2Application(application) {

        // OPENGL INIT

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, GL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, GL_TRUE);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, GL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, _stencilBits);
        if (_samples > 0) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, _samples);
        }

        // WINDOW FLAGS

        uint32_t windowFlags = 0;
        #if defined(ANDROID)
        // For Android we need to set up for OpenGL ES and we make the window hi res & full screen
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                      SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP |
                      SDL_WINDOW_ALLOW_HIGHDPI;
        #else
        // For all other clients we use the core profile and operate in a window
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        windowFlags = SDL_WINDOW_OPENGL;
        if (!window->getVisible()) {
            windowFlags |= SDL_WINDOW_HIDDEN;
        }
        if (window->getResizable()) {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }
        if (!window->getDecorated()) {
            windowFlags |= SDL_WINDOW_BORDERLESS;
        }
        #endif

        // CREATE WINDOW

        _sdl2Window = SDL_CreateWindow(
            window->getTitle().c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            _width,
            _height,
            windowFlags
        );

        if (!_sdl2Window) {
            logSDLError("SDL_CreateWindow");
            SDL_Quit();
            throw std::runtime_error("Could not create SDL2 window!");
        }

        if (window->getFullscreen()) {
            SDL_SetWindowFullscreen(_sdl2Window, SDL_WINDOW_FULLSCREEN);
        }

        // GATHER INFORMATION
        SDL_GetWindowPosition(_sdl2Window, &_x, &_y);
        SDL_GetWindowSize(_sdl2Window, &_width, &_height);
        // TODO: Pixel ratio?

        // GL CONTEXT

        _sdl2GlContext = SDL_GL_CreateContext(_sdl2Window);
        if (!_sdl2GlContext) {
            logSDLError("SDL_GL_CreateContext");
            SDL_Quit();
            throw std::runtime_error("Could not create SDL2 GL context!");
        }

        int success = SDL_GL_MakeCurrent(_sdl2Window, _sdl2GlContext);
        if (success != 0) {
            logSDLError("SDL_GL_CreateContext");
            SDL_Quit();
            throw std::runtime_error("Could not make SDL2 GL context current!");
        }

        // Get Some info back about the framebuffer (in case its different from what we set?)
        glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER,
            GL_STENCIL,
            GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
            &_stencilBits
        );
        glGetIntegerv(GL_SAMPLES, &_samples);

        glViewport(0, 0, _width, _height);
        glClearColor(0, 0, 0, 0);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // CUSTOM DATA

        SDL_SetWindowData(_sdl2Window, "psychic", window.get());

        // OPEN

        _lastInteraction = static_cast<double>(SDL_GetTicks()) / 1000.0f;

        // Create system cursors (We match only what is also available in GLFW)
        _cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        _cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
        _cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
        _cursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
        _cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
        _cursors[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);


        window->open(this);
    }

    SDL2SystemWindow::~SDL2SystemWindow() {
        for (auto &_cursor : _cursors) {
            if (_cursor) {
                SDL_FreeCursor(_cursor);
            }
        }

        if (_sdl2GlContext) {
            SDL_GL_DeleteContext(_sdl2GlContext);
        }
        if (_sdl2Window) {
            SDL_DestroyWindow(_sdl2Window);
        }
    }

    bool SDL2SystemWindow::render() {
        if (!_window->getVisible()) {
            return false;
        }

        _window->drawAll();

        SDL_GL_SwapWindow(_sdl2Window);

        return true;
    }

    SDL_Window *SDL2SystemWindow::sdl2Window() const {
        return _sdl2Window;
    }

    void SDL2SystemWindow::setSize(int width, int height) {
        _width  = width;
        _height = height;
        SDL_SetWindowSize(_sdl2Window, _width, _height);
    }

    void SDL2SystemWindow::setTitle(const std::string &title) {
        SDL_SetWindowTitle(_sdl2Window, title.c_str());
    }

    void SDL2SystemWindow::setFullscreen(bool fullscreen) {
        SDL_SetWindowFullscreen(_sdl2Window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }

    bool SDL2SystemWindow::getMinimized() const {
        return _minimized;
    }

    void SDL2SystemWindow::setMinimized(bool minimized) {
        if (_minimized != minimized) {
            _minimized = minimized;
            if (_minimized) {
                SDL_MinimizeWindow(_sdl2Window);
            } else {
                SDL_RestoreWindow(_sdl2Window);
            }
        }
    }

    bool SDL2SystemWindow::getMaximized() const {
        return _maximized;
    }

    void SDL2SystemWindow::setMaximized(bool maximized) {
        if (_maximized != maximized) {
            _maximized = maximized;
            if (_maximized) {
                SDL_MaximizeWindow(_sdl2Window);
            } else {
                SDL_RestoreWindow(_sdl2Window);
            }
        }
    }

    void SDL2SystemWindow::setVisible(bool visible) {
        if (visible) {
            SDL_ShowWindow(_sdl2Window);
        } else {
            SDL_HideWindow(_sdl2Window);
        }
    }

    void SDL2SystemWindow::setCursor(int cursor) {
        SDL_SetCursor(_cursors[cursor]);
    }

    void SDL2SystemWindow::startDrag() {
        _windowDragMouseX = _mouseX;
        _windowDragMouseY = _mouseY;
        _dragging         = true;
    }

    void SDL2SystemWindow::stopDrag() {
        _dragging         = false;
        _windowDragMouseX = 0;
        _windowDragMouseY = 0;
    }

    void SDL2SystemWindow::handleEvent(const SDL_Event &e) {
        _lastInteraction = static_cast<double>(SDL_GetTicks()) / 1000.0f;

        switch (e.type) {
            case SDL_WINDOWEVENT:
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SHOWN:
                        break;
                    case SDL_WINDOWEVENT_HIDDEN:
                        break;
                    case SDL_WINDOWEVENT_EXPOSED:
                        break;
                    case SDL_WINDOWEVENT_MOVED:
                        _x = e.window.data1;
                        _y = e.window.data2;
                        _window->windowMoved(_x, _y);
                        break;

                    case SDL_WINDOWEVENT_RESIZED:
                        _width  = e.window.data1;
                        _height = e.window.data2;
                        _window->windowResized(_width, _height);
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        break;

                    case SDL_WINDOWEVENT_MINIMIZED:
                        _minimized = true;
                        _window->windowMinimized();
                        break;

                    case SDL_WINDOWEVENT_RESTORED:
                        _minimized = false;
                        _window->windowRestored();
                        break;

                    case SDL_WINDOWEVENT_MAXIMIZED:
                        _maximized = true;
                        break;

                    case SDL_WINDOWEVENT_ENTER:
                        break;

                    case SDL_WINDOWEVENT_LEAVE:
                        break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        _focused = true;
                        _window->windowActivated();
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        _focused = false;
                        _window->windowDeactivated();
                        break;

                    case SDL_WINDOWEVENT_CLOSE:
                        std::cerr << "No way to close?" << std::endl;
                        break;

                    case SDL_WINDOWEVENT_TAKE_FOCUS:
                        break;

                    case SDL_WINDOWEVENT_HIT_TEST:
                        break;

                    default:
                        break;
                }
                break;

            case SDL_KEYDOWN:
                // TODO: Key mods map
                _window->keyboardEvent(e.key.keysym.sym, e.key.keysym.scancode, e.key.repeat ? 2 : 1, 0);
                break;

            case SDL_KEYUP:
                // TODO: Key mods map
                _window->keyboardEvent(e.key.keysym.sym, e.key.keysym.scancode, 0, 0);
                break;

            case SDL_TEXTINPUT:
            case SDL_TEXTEDITING:
                // TODO: Setup text input
                break;

            case SDL_MOUSEMOTION:
                // Weird but the cursor seem better aligned like this
                // At least on a mac...
                _mouseX = static_cast<int>(e.motion.x) - 1;
                _mouseY = static_cast<int>(e.motion.y) - 2;

                // Handle window dragging
                if (_dragging) {
                    _windowDragOffsetX = _mouseX - _windowDragMouseX;
                    _windowDragOffsetY = _mouseY - _windowDragMouseY;
                    SDL_SetWindowPosition(_sdl2Window, _x + _windowDragOffsetX, _y + _windowDragOffsetY);
                }

                _window->mouseMoved(_mouseX, _mouseY, _mouseState, _modifiers, false);
                break;

            case SDL_MOUSEBUTTONDOWN: {
                int btn = 0;
                if (e.button.button == SDL_BUTTON_LEFT) {
                    btn = 1;
                } else if (e.button.button == SDL_BUTTON_MIDDLE) {
                    btn = 2;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    btn = 3;
                } else {
                    break;
                }
                _mouseState |= 1 << btn;
                _window->mouseButton(_mouseX, _mouseY, btn, true, _modifiers);
                _window->mouseDown(_mouseX, _mouseY, btn, _modifiers);
                break;
            }

            case SDL_MOUSEBUTTONUP: {
                int btn = 0;
                if (e.button.button == SDL_BUTTON_LEFT) {
                    btn = 1;
                } else if (e.button.button == SDL_BUTTON_MIDDLE) {
                    btn = 2;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    btn = 3;
                } else {
                    break;
                }
                _mouseState &= ~(1 << btn);
                _window->mouseButton(_mouseX, _mouseY, btn, false, _modifiers);
                _window->click(_mouseX, _mouseY, btn, _modifiers);
                _window->mouseUp(_mouseX, _mouseY, btn, _modifiers);
                break;
            }

            case SDL_MOUSEWHEEL:
                _window->mouseScrolled(_mouseX, _mouseY, e.wheel.x, e.wheel.y);
                break;

            default:
                break;
        }
    }

    void SDL2SystemWindow::liveResize(int width, int height) {
        _width  = width;
        _height = height;
        _window->windowResized(_width, _height);
        render();
    }

}

#endif
