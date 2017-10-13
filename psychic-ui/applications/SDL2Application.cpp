#ifdef WITH_SDL2

#include <iostream>
#include <unicode/unistr.h>
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
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

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

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, SDL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, _stencilBits);
        if (_samples > 0) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
        }

        // WINDOW FLAGS

        uint32_t windowFlags = 0;
        // TODO: IOS
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
        //glGetFramebufferAttachmentParameteriv(
        //    GL_DRAW_FRAMEBUFFER,
        //    GL_STENCIL,
        //    GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
        //    &_stencilBits
        //);
        //glGetIntegerv(GL_SAMPLES, &_samples);

        glViewport(0, 0, _width, _height);
        glClearColor(1, 1, 1, 1);
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

    void SDL2SystemWindow::setPosition(int x, int y) {
        _x = x;
        _y = y;
        SDL_SetWindowPosition(_sdl2Window, _x, _y);
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

    void SDL2SystemWindow::startTextInput() {
        SDL_StartTextInput();
    }

    void SDL2SystemWindow::stopTextInput() {
        SDL_StopTextInput();
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

                    //(>= SDL 2.0.5)
                    //case SDL_WINDOWEVENT_TAKE_FOCUS:
                    //    break;

                    //(>= SDL 2.0.5)
                    //case SDL_WINDOWEVENT_HIT_TEST:
                    //    break;

                    default:
                        break;
                }
                break;

            case SDL_KEYDOWN:
                if (e.key.repeat) {
                    _window->keyRepeat(mapKey(e.key.keysym.sym), mapMods(e.key.keysym.mod));
                } else {
                    _window->keyDown(mapKey(e.key.keysym.sym), mapMods(e.key.keysym.mod));
                }
                break;

            case SDL_KEYUP:
                _window->keyUp(mapKey(e.key.keysym.sym), mapMods(e.key.keysym.mod));
                break;

            case SDL_TEXTINPUT:
                _window->keyboardCharacterEvent(UnicodeString::fromUTF8(e.text.text));
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

                _window->mouseMoved(_mouseX, _mouseY, _mouseState, mapMods(SDL_GetModState()), false);
                break;

            case SDL_MOUSEBUTTONDOWN: {
                MouseButton btn;
                if (e.button.button == SDL_BUTTON_LEFT) {
                    btn = MouseButton::LEFT;
                } else if (e.button.button == SDL_BUTTON_MIDDLE) {
                    btn = MouseButton::MIDDLE;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    btn = MouseButton::RIGHT;
                } else {
                    break;
                }
                _mouseState |= 1 << btn;
                _window->mouseButton(_mouseX, _mouseY, btn, true, mapMods(SDL_GetModState()));
                //_window->mouseDown(_mouseX, _mouseY, btn, mapMods(SDL_GetModState()));
                break;
            }

            case SDL_MOUSEBUTTONUP: {
                MouseButton btn;
                if (e.button.button == SDL_BUTTON_LEFT) {
                    btn = MouseButton::LEFT;
                } else if (e.button.button == SDL_BUTTON_MIDDLE) {
                    btn = MouseButton::MIDDLE;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    btn = MouseButton::RIGHT;
                } else {
                    break;
                }
                _mouseState &= ~btn;
                _window->mouseButton(_mouseX, _mouseY, btn, false, mapMods(SDL_GetModState()));
                //_window->click(_mouseX, _mouseY, btn, mapMods(SDL_GetModState()));
                //_window->mouseUp(_mouseX, _mouseY, btn, mapMods(SDL_GetModState()));
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

    Mod SDL2SystemWindow::mapMods(int mods) {
        Mod mod{};
        mod.shift = (mods & KMOD_SHIFT) != 0;
        mod.ctrl  = (mods & KMOD_CTRL) != 0;
        mod.alt   = (mods & KMOD_ALT) != 0;
        mod.super = (mods & KMOD_GUI) != 0;
        return mod;
    }
    
    Key SDL2SystemWindow::mapKey(int keycode) {
        switch (keycode) {
            case SDLK_SPACE: return Key::SPACE;
            //case SDLK_APOSTROPHE: return Key::APOSTROPHE;
            case SDLK_COMMA: return Key::COMMA;
            case SDLK_MINUS: return Key::MINUS;
            case SDLK_PERIOD: return Key::PERIOD;
            case SDLK_SLASH: return Key::SLASH;
            case SDLK_0: return Key::ZERO;
            case SDLK_1: return Key::ONE;
            case SDLK_2: return Key::TWO;
            case SDLK_3: return Key::THREE;
            case SDLK_4: return Key::FOUR;
            case SDLK_5: return Key::FIVE;
            case SDLK_6: return Key::SIX;
            case SDLK_7: return Key::SEVEN;
            case SDLK_8: return Key::EIGHT;
            case SDLK_9: return Key::NINE;
            case SDLK_SEMICOLON: return Key::SEMICOLON;
            case SDLK_EQUALS: return Key::EQUAL;
            case SDLK_a: return Key::A;
            case SDLK_b: return Key::B;
            case SDLK_c: return Key::C;
            case SDLK_d: return Key::D;
            case SDLK_e: return Key::E;
            case SDLK_f: return Key::F;
            case SDLK_g: return Key::G;
            case SDLK_h: return Key::H;
            case SDLK_i: return Key::I;
            case SDLK_j: return Key::J;
            case SDLK_k: return Key::K;
            case SDLK_l: return Key::L;
            case SDLK_m: return Key::M;
            case SDLK_n: return Key::N;
            case SDLK_o: return Key::O;
            case SDLK_p: return Key::P;
            case SDLK_q: return Key::Q;
            case SDLK_r: return Key::R;
            case SDLK_s: return Key::S;
            case SDLK_t: return Key::T;
            case SDLK_u: return Key::U;
            case SDLK_v: return Key::V;
            case SDLK_w: return Key::W;
            case SDLK_x: return Key::X;
            case SDLK_y: return Key::Y;
            case SDLK_z: return Key::Z;
            case SDLK_LEFTBRACKET: return Key::LEFT_BRACKET;
            case SDLK_BACKSLASH: return Key::BACKSLASH;
            case SDLK_RIGHTBRACKET: return Key::RIGHT_BRACKET;
            //case SDLK_GRAVEACCENT: return Key::GRAVE_ACCENT;
            //case SDLK_WORLD_1: return Key::WORLD_1;
            //case SDLK_WORLD_2: return Key::WORLD_2;
            case SDLK_ESCAPE: return Key::ESCAPE;
            case SDLK_RETURN: return Key::ENTER;
            case SDLK_TAB: return Key::TAB;
            case SDLK_BACKSPACE: return Key::BACKSPACE;
            case SDLK_INSERT: return Key::INSERT;
            case SDLK_DELETE: return Key::DELETE;
            case SDLK_RIGHT: return Key::RIGHT;
            case SDLK_LEFT: return Key::LEFT;
            case SDLK_DOWN: return Key::DOWN;
            case SDLK_UP: return Key::UP;
            case SDLK_PAGEUP: return Key::PAGE_UP;
            case SDLK_PAGEDOWN: return Key::PAGE_DOWN;
            case SDLK_HOME: return Key::HOME;
            case SDLK_END: return Key::END;
            case SDLK_CAPSLOCK: return Key::CAPS_LOCK;
            case SDLK_SCROLLLOCK: return Key::SCROLL_LOCK;
            case SDLK_NUMLOCKCLEAR: return Key::NUM_LOCK;
            case SDLK_PRINTSCREEN: return Key::PRINT_SCREEN;
            case SDLK_PAUSE: return Key::PAUSE;
            case SDLK_F1: return Key::F1;
            case SDLK_F2: return Key::F2;
            case SDLK_F3: return Key::F3;
            case SDLK_F4: return Key::F4;
            case SDLK_F5: return Key::F5;
            case SDLK_F6: return Key::F6;
            case SDLK_F7: return Key::F7;
            case SDLK_F8: return Key::F8;
            case SDLK_F9: return Key::F9;
            case SDLK_F10: return Key::F10;
            case SDLK_F11: return Key::F11;
            case SDLK_F12: return Key::F12;
            case SDLK_F13: return Key::F13;
            case SDLK_F14: return Key::F14;
            case SDLK_F15: return Key::F15;
            case SDLK_F16: return Key::F16;
            case SDLK_F17: return Key::F17;
            case SDLK_F18: return Key::F18;
            case SDLK_F19: return Key::F19;
            case SDLK_F20: return Key::F20;
            case SDLK_F21: return Key::F21;
            case SDLK_F22: return Key::F22;
            case SDLK_F23: return Key::F23;
            case SDLK_F24: return Key::F24;
            //case SDLK_F25: return Key::F25;
            case SDLK_KP_0: return Key::KP_0;
            case SDLK_KP_1: return Key::KP_1;
            case SDLK_KP_2: return Key::KP_2;
            case SDLK_KP_3: return Key::KP_3;
            case SDLK_KP_4: return Key::KP_4;
            case SDLK_KP_5: return Key::KP_5;
            case SDLK_KP_6: return Key::KP_6;
            case SDLK_KP_7: return Key::KP_7;
            case SDLK_KP_8: return Key::KP_8;
            case SDLK_KP_9: return Key::KP_9;
            case SDLK_KP_DECIMAL: return Key::KP_DECIMAL;
            case SDLK_KP_DIVIDE: return Key::KP_DIVIDE;
            case SDLK_KP_MULTIPLY: return Key::KP_MULTIPLY;
            case SDLK_KP_MINUS: return Key::KP_SUBTRACT;
            case SDLK_KP_PLUS: return Key::KP_ADD;
            case SDLK_KP_ENTER: return Key::KP_ENTER;
            case SDLK_KP_EQUALS: return Key::KP_EQUAL;
            case SDLK_LSHIFT: return Key::LEFT_SHIFT;
            case SDLK_LCTRL: return Key::LEFT_CONTROL;
            case SDLK_LALT: return Key::LEFT_ALT;
            case SDLK_LGUI: return Key::LEFT_SUPER;
            case SDLK_RSHIFT: return Key::RIGHT_SHIFT;
            case SDLK_RCTRL: return Key::RIGHT_CONTROL;
            case SDLK_RALT: return Key::RIGHT_ALT;
            case SDLK_RGUI: return Key::RIGHT_SUPER;
            case SDLK_MENU: return Key::MENU;
            case SDLK_UNKNOWN:
            default:
                return Key::UNKNOWN;
        }
    }
}

#endif
