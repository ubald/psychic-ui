#include <cmath>
#include <cstring>
#include <unicode/unistr.h>
#include "SDL3SystemWindow.hpp"
#include "../Window.hpp"

namespace psychic_ui {

    SDL3SystemWindow::SDL3SystemWindow(std::shared_ptr<Window> window, SDL_Window *sdlWindow, SDL_GLContext glContext) :
        SystemWindow(std::move(window)),
        _sdlWindow(sdlWindow),
        _glContext(glContext) {

        if (!_sdlWindow) {
            throw std::invalid_argument("SDL3SystemWindow needs an SDL window");
        }

        // The host may draw with this context too
        _sharesContext = true;

        SDL_GetWindowPosition(_sdlWindow, &_x, &_y);
        readSize();

        const SDL_WindowFlags flags = SDL_GetWindowFlags(_sdlWindow);
        _minimized = (flags & SDL_WINDOW_MINIMIZED) != 0;
        _maximized = (flags & SDL_WINDOW_MAXIMIZED) != 0;
        _focused   = (flags & SDL_WINDOW_INPUT_FOCUS) != 0;

        if (_glContext && SDL_GL_MakeCurrent(_sdlWindow, _glContext)) {
            readFramebufferFormat();
        }

        // In the order of `Cursor`
        _cursors[static_cast<int>(Cursor::Arrow)]     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
        _cursors[static_cast<int>(Cursor::IBeam)]     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
        _cursors[static_cast<int>(Cursor::Crosshair)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
        _cursors[static_cast<int>(Cursor::Hand)]      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
        _cursors[static_cast<int>(Cursor::HResize)]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
        _cursors[static_cast<int>(Cursor::VResize)]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);

        _window->open(this);
    }

    SDL3SystemWindow::~SDL3SystemWindow() {
        if (_glContext) {
            SDL_GL_MakeCurrent(_sdlWindow, _glContext);
        }
        detach();

        for (auto &cursor : _cursors) {
            if (cursor) {
                SDL_DestroyCursor(cursor);
            }
        }
    }

    SDL_Window *SDL3SystemWindow::sdlWindow() const {
        return _sdlWindow;
    }

    void SDL3SystemWindow::readSize() {
        SDL_GetWindowSize(_sdlWindow, &_width, &_height);
        if (!SDL_GetWindowSizeInPixels(_sdlWindow, &_fbWidth, &_fbHeight)) {
            _fbWidth  = _width;
            _fbHeight = _height;
        }
    }

    bool SDL3SystemWindow::render() {
        if (!_window->windowVisible()) {
            return false;
        }

        if (_glContext) {
            SDL_GL_MakeCurrent(_sdlWindow, _glContext);
        }

        _window->drawAll();

        return true;
    }

    void *SDL3SystemWindow::glProcAddress(const char *name) const {
        return reinterpret_cast<void *>(SDL_GL_GetProcAddress(name));
    }

    // region Window operations

    // SDL3 applies most of these asynchronously: the state is updated from the events that confirm them

    void SDL3SystemWindow::setSize(int width, int height) {
        SDL_SetWindowSize(_sdlWindow, width, height);
    }

    void SDL3SystemWindow::setPosition(int x, int y) {
        SDL_SetWindowPosition(_sdlWindow, x, y);
    }

    void SDL3SystemWindow::setTitle(const std::string &title) {
        SDL_SetWindowTitle(_sdlWindow, title.c_str());
    }

    void SDL3SystemWindow::setFullscreen(bool fullscreen) {
        SDL_SetWindowFullscreen(_sdlWindow, fullscreen);
    }

    bool SDL3SystemWindow::getMinimized() const {
        return _minimized;
    }

    void SDL3SystemWindow::setMinimized(bool minimized) {
        if (minimized) {
            SDL_MinimizeWindow(_sdlWindow);
        } else {
            SDL_RestoreWindow(_sdlWindow);
        }
    }

    bool SDL3SystemWindow::getMaximized() const {
        return _maximized;
    }

    void SDL3SystemWindow::setMaximized(bool maximized) {
        if (maximized) {
            SDL_MaximizeWindow(_sdlWindow);
        } else {
            SDL_RestoreWindow(_sdlWindow);
        }
    }

    void SDL3SystemWindow::setVisible(bool visible) {
        if (visible) {
            SDL_ShowWindow(_sdlWindow);
        } else {
            SDL_HideWindow(_sdlWindow);
        }
    }

    void SDL3SystemWindow::setCursor(Cursor cursor) {
        const auto index = static_cast<std::size_t>(cursor);
        if (index < std::size(_cursors) && _cursors[index]) {
            SDL_SetCursor(_cursors[index]);
        }
    }

    void SDL3SystemWindow::startDrag() {
        // Moves the window from the global mouse position, which the window's own moves don't affect
        float mouseX = 0;
        float mouseY = 0;
        SDL_GetGlobalMouseState(&mouseX, &mouseY);
        int x = 0;
        int y = 0;
        SDL_GetWindowPosition(_sdlWindow, &x, &y);
        _dragOffsetX = mouseX - static_cast<float>(x);
        _dragOffsetY = mouseY - static_cast<float>(y);
        _dragging    = true;
    }

    void SDL3SystemWindow::stopDrag() {
        _dragging = false;
    }

    bool SDL3SystemWindow::supportsDrag() const {
        // Without a hit test, `startDrag` places the window itself, which Wayland doesn't allow
        const char *driver = SDL_GetCurrentVideoDriver();
        return _hitTestInstalled || !driver || std::strcmp(driver, "wayland") != 0;
    }

    bool SDL3SystemWindow::installHitTest() {
        _hitTestInstalled = SDL_SetWindowHitTest(
            _sdlWindow,
            [](SDL_Window * /*window*/, const SDL_Point *point, void *data) {
                return static_cast<SDL3SystemWindow *>(data)->hitTest(point->x, point->y);
            },
            this
        );
        return _hitTestInstalled;
    }

    SDL_HitTestResult SDL3SystemWindow::hitTest(const int x, const int y) {
        // An undecorated window has no borders to resize it by but these
        const SDL_WindowFlags flags = SDL_GetWindowFlags(_sdlWindow);
        if ((flags & SDL_WINDOW_BORDERLESS) && (flags & SDL_WINDOW_RESIZABLE)
            && !(flags & (SDL_WINDOW_MAXIMIZED | SDL_WINDOW_FULLSCREEN))) {
            constexpr int border = 4;
            const bool left   = x < border;
            const bool right  = x >= _width - border;
            const bool top    = y < border;
            const bool bottom = y >= _height - border;
            if (top && left) { return SDL_HITTEST_RESIZE_TOPLEFT; }
            if (top && right) { return SDL_HITTEST_RESIZE_TOPRIGHT; }
            if (bottom && left) { return SDL_HITTEST_RESIZE_BOTTOMLEFT; }
            if (bottom && right) { return SDL_HITTEST_RESIZE_BOTTOMRIGHT; }
            if (top) { return SDL_HITTEST_RESIZE_TOP; }
            if (bottom) { return SDL_HITTEST_RESIZE_BOTTOM; }
            if (left) { return SDL_HITTEST_RESIZE_LEFT; }
            if (right) { return SDL_HITTEST_RESIZE_RIGHT; }
        }

        return _window->isDragArea(x, y) ? SDL_HITTEST_DRAGGABLE : SDL_HITTEST_NORMAL;
    }

    std::string SDL3SystemWindow::getClipboardText() const {
        char *text = SDL_GetClipboardText();
        std::string result = text ? text : "";
        SDL_free(text);
        return result;
    }

    void SDL3SystemWindow::setClipboardText(const std::string &text) {
        SDL_SetClipboardText(text.c_str());
    }

    void SDL3SystemWindow::startTextInput() {
        SDL_StartTextInput(_sdlWindow);
    }

    void SDL3SystemWindow::stopTextInput() {
        SDL_StopTextInput(_sdlWindow);
    }

    void SDL3SystemWindow::setTextInputArea(const int x, const int y, const int width, const int height, const int cursor) {
        const SDL_Rect rect{x, y, width, height};
        SDL_SetTextInputArea(_sdlWindow, &rect, cursor);
    }

    std::chrono::nanoseconds SDL3SystemWindow::gestureTime() const {
        return std::chrono::nanoseconds{SDL_GetTicksNS()};
    }

    // endregion

    // region Events

    static MouseButton mapButton(Uint8 button) {
        switch (button) {
            case SDL_BUTTON_LEFT: return MouseButton::LEFT;
            case SDL_BUTTON_MIDDLE: return MouseButton::MIDDLE;
            case SDL_BUTTON_RIGHT: return MouseButton::RIGHT;
            default: return static_cast<MouseButton>(0);
        }
    }

    SDL_TouchDeviceType SDL3SystemWindow::touchDeviceType(const SDL_TouchID touchID) const {
        return SDL_GetTouchDeviceType(touchID);
    }

    /**
     * Where a mouse event came from: SDL makes mouse events of touches and pens too, which we make our own
     */
    static bool fromMouse(const SDL_MouseID which) {
        return which != SDL_TOUCH_MOUSEID && which != SDL_PEN_MOUSEID;
    }

    void SDL3SystemWindow::handleFingerEvent(const SDL_TouchFingerEvent &finger) {
        // Trackpads move the mouse too, a pen's touches come as pen events and the mouse's (SDL's hint) as mouse events
        if (finger.touchID == SDL_PEN_TOUCHID || finger.touchID == SDL_MOUSE_TOUCHID
            || touchDeviceType(finger.touchID) != SDL_TOUCH_DEVICE_DIRECT) {
            return;
        }

        const auto  time = std::chrono::nanoseconds{finger.timestamp};
        const float x    = finger.x * static_cast<float>(_width);
        const float y    = finger.y * static_cast<float>(_height);

        if (finger.type == SDL_EVENT_FINGER_DOWN) {
            if (_fingerDown) {
                return; // Only the first finger down counts
            }
            _fingerDown = true;
            _finger     = finger.fingerID;
            _window->handleTouchPress(x, y, time);
            return;
        }

        if (!_fingerDown || finger.fingerID != _finger) {
            return;
        }
        switch (finger.type) {
            case SDL_EVENT_FINGER_MOTION:
                _window->handleTouchMove(x, y, time);
                break;
            case SDL_EVENT_FINGER_UP:
                _fingerDown = false;
                _window->handleTouchRelease(x, y, time);
                break;
            case SDL_EVENT_FINGER_CANCELED:
                _fingerDown = false;
                _window->handleTouchCancel(time);
                break;
            default:
                break;
        }
    }

    void SDL3SystemWindow::handlePenButton(const MouseButton button, const bool down, const float x, const float y) {
        _window->handleInputModeChange(InputMode::Pen);
        if (button == 0 || ((_penButtons & button) != 0) == down) {
            return;
        }
        _penButtons = down ? (_penButtons | button) : (_penButtons & ~button);
        _mouseX     = static_cast<int>(x);
        _mouseY     = static_cast<int>(y);
        _window->handleMouseButtonChange(_mouseX, _mouseY, button, down, mapMods(SDL_GetModState()));
    }

    bool SDL3SystemWindow::handleEvent(const SDL_Event &e) {
        if (SDL_GetWindowFromEvent(&e) != _sdlWindow) {
            return false;
        }

        switch (e.type) {
            case SDL_EVENT_WINDOW_MOVED:
                _x = e.window.data1;
                _y = e.window.data2;
                _window->windowMoved(_x, _y);
                break;

            case SDL_EVENT_WINDOW_RESIZED: {
                _width  = e.window.data1;
                _height = e.window.data2;
                // SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED follows when the pixel size changed too
                const float density = SDL_GetWindowPixelDensity(_sdlWindow);
                _fbWidth  = static_cast<int>(std::lround(static_cast<float>(_width) * (density > 0 ? density : 1.0f)));
                _fbHeight = static_cast<int>(std::lround(static_cast<float>(_height) * (density > 0 ? density : 1.0f)));
                _window->windowResized(_width, _height);
                break;
            }

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                _fbWidth  = e.window.data1;
                _fbHeight = e.window.data2;
                _window->windowResized(_width, _height);
                break;

            case SDL_EVENT_WINDOW_MINIMIZED:
                _minimized = true;
                _window->windowMinimized();
                break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
                _maximized = true;
                break;

            case SDL_EVENT_WINDOW_RESTORED:
                _minimized = false;
                _maximized = false;
                _window->windowRestored();
                break;

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                _focused = true;
                _window->windowActivated();
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                _focused = false;
                _window->windowDeactivated();
                break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (_window->windowShouldClose()) {
                    _window->close();
                }
                break;

            case SDL_EVENT_KEY_DOWN:
                if (e.key.repeat) {
                    _window->handleKeyRepeat(mapKey(e.key.key), mapMods(e.key.mod));
                } else {
                    _window->handleKeyPress(mapKey(e.key.key), mapMods(e.key.mod));
                }
                break;

            case SDL_EVENT_KEY_UP:
                _window->handleKeyRelease(mapKey(e.key.key), mapMods(e.key.mod));
                break;

            case SDL_EVENT_TEXT_INPUT: {
                // One code point at a time, like typed characters, even when an input method commits several
                const icu::UnicodeString text = icu::UnicodeString::fromUTF8(e.text.text);
                for (int32_t i = 0; i < text.length(); i = text.moveIndex32(i, 1)) {
                    _window->handleType(icu::UnicodeString(text.char32At(i)));
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
                if (!fromMouse(e.motion.which)) {
                    break; // Made of a touch or a pen, which we follow ourselves
                }
                // A real mouse moved: back to the pointer density
                _window->handleInputModeChange(InputMode::Pointer);
                _mouseX = static_cast<int>(e.motion.x);
                _mouseY = static_cast<int>(e.motion.y);

                if (_dragging) {
                    float mouseX = 0;
                    float mouseY = 0;
                    SDL_GetGlobalMouseState(&mouseX, &mouseY);
                    SDL_SetWindowPosition(
                        _sdlWindow,
                        static_cast<int>(mouseX - _dragOffsetX),
                        static_cast<int>(mouseY - _dragOffsetY)
                    );
                }

                _window->handleMouseMove(_mouseX, _mouseY, _mouseState, mapMods(SDL_GetModState()), false);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                const MouseButton button = mapButton(e.button.button);
                if (button == 0 || !fromMouse(e.button.which)) {
                    break;
                }
                _window->handleInputModeChange(InputMode::Pointer);
                const bool down = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
                _mouseX = static_cast<int>(e.button.x);
                _mouseY = static_cast<int>(e.button.y);
                if (down) {
                    _mouseState |= button;
                } else {
                    _mouseState &= ~button;
                }
                _window->handleMouseButtonChange(_mouseX, _mouseY, button, down, mapMods(SDL_GetModState()));
                break;
            }

            case SDL_EVENT_DROP_BEGIN:
                _droppedFiles.clear();
                break;

            case SDL_EVENT_DROP_FILE:
                if (e.drop.data) {
                    _droppedFiles.emplace_back(e.drop.data);
                }
                break;

            case SDL_EVENT_DROP_COMPLETE:
                if (!_droppedFiles.empty()) {
                    _window->dropEvent(_droppedFiles);
                    _droppedFiles.clear();
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL: {
                if (e.wheel.which == SDL_TOUCH_MOUSEID) {
                    break;
                }
                const float direction = e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f;
                _window->handleMouseScroll(
                    static_cast<int>(e.wheel.mouse_x),
                    static_cast<int>(e.wheel.mouse_y),
                    e.wheel.x * direction,
                    e.wheel.y * direction
                );
                break;
            }

            case SDL_EVENT_FINGER_DOWN:
            case SDL_EVENT_FINGER_MOTION:
            case SDL_EVENT_FINGER_UP:
            case SDL_EVENT_FINGER_CANCELED:
                handleFingerEvent(e.tfinger);
                break;

            // A pen works like a mouse, its tip being the left button, but reports the pen mode
            case SDL_EVENT_PEN_PROXIMITY_IN:
            case SDL_EVENT_PEN_PROXIMITY_OUT:
            case SDL_EVENT_PEN_AXIS:
                _window->handleInputModeChange(InputMode::Pen);
                break;

            case SDL_EVENT_PEN_DOWN:
            case SDL_EVENT_PEN_UP:
                handlePenButton(MouseButton::LEFT, e.ptouch.down, e.ptouch.x, e.ptouch.y);
                break;

            case SDL_EVENT_PEN_BUTTON_DOWN:
            case SDL_EVENT_PEN_BUTTON_UP:
                // The barrel buttons: the first one is the right button, the second the middle one
                handlePenButton(
                    e.pbutton.button == 1 ? MouseButton::RIGHT : e.pbutton.button == 2 ? MouseButton::MIDDLE : static_cast<MouseButton>(0),
                    e.pbutton.down, e.pbutton.x, e.pbutton.y
                );
                break;

            case SDL_EVENT_PEN_MOTION:
                _window->handleInputModeChange(InputMode::Pen);
                _mouseX = static_cast<int>(e.pmotion.x);
                _mouseY = static_cast<int>(e.pmotion.y);
                _window->handleMouseMove(_mouseX, _mouseY, _penButtons, mapMods(SDL_GetModState()), false);
                break;

            default:
                break;
        }

        return true;
    }

    // endregion

    // region Mapping

    Mod SDL3SystemWindow::mapMods(SDL_Keymod mods) {
        Mod mod{};
        mod.shift = (mods & SDL_KMOD_SHIFT) != 0;
        mod.ctrl  = (mods & SDL_KMOD_CTRL) != 0;
        mod.alt   = (mods & SDL_KMOD_ALT) != 0;
        mod.super = (mods & SDL_KMOD_GUI) != 0;
        return mod;
    }

    Key SDL3SystemWindow::mapKey(SDL_Keycode keycode) {
        switch (keycode) {
            case SDLK_SPACE: return Key::SPACE;
            case SDLK_APOSTROPHE: return Key::APOSTROPHE;
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
            case SDLK_A: return Key::A;
            case SDLK_B: return Key::B;
            case SDLK_C: return Key::C;
            case SDLK_D: return Key::D;
            case SDLK_E: return Key::E;
            case SDLK_F: return Key::F;
            case SDLK_G: return Key::G;
            case SDLK_H: return Key::H;
            case SDLK_I: return Key::I;
            case SDLK_J: return Key::J;
            case SDLK_K: return Key::K;
            case SDLK_L: return Key::L;
            case SDLK_M: return Key::M;
            case SDLK_N: return Key::N;
            case SDLK_O: return Key::O;
            case SDLK_P: return Key::P;
            case SDLK_Q: return Key::Q;
            case SDLK_R: return Key::R;
            case SDLK_S: return Key::S;
            case SDLK_T: return Key::T;
            case SDLK_U: return Key::U;
            case SDLK_V: return Key::V;
            case SDLK_W: return Key::W;
            case SDLK_X: return Key::X;
            case SDLK_Y: return Key::Y;
            case SDLK_Z: return Key::Z;
            case SDLK_LEFTBRACKET: return Key::LEFT_BRACKET;
            case SDLK_BACKSLASH: return Key::BACKSLASH;
            case SDLK_RIGHTBRACKET: return Key::RIGHT_BRACKET;
            case SDLK_GRAVE: return Key::GRAVE_ACCENT;
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
            case SDLK_MENU:
            case SDLK_APPLICATION: return Key::MENU;
            default: return Key::UNKNOWN;
        }
    }

    // endregion
}
