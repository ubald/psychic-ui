#include <algorithm>
#include <bit>
#include <cstddef>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>
#include <unicode/unistr.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "GLFWSystemWindow.hpp"
#include "../Window.hpp"

namespace psychic_ui {

    /**
     * The host's callbacks, which the adapter replaced and chains to
     */
    struct GLFWSystemWindow::Callbacks {
        GLFWcursorposfun       cursorPos{nullptr};
        GLFWmousebuttonfun     mouseButton{nullptr};
        GLFWscrollfun          scroll{nullptr};
        GLFWkeyfun             key{nullptr};
        GLFWcharfun            character{nullptr};
        GLFWdropfun            drop{nullptr};
        GLFWwindowsizefun      size{nullptr};
        GLFWframebuffersizefun framebufferSize{nullptr};
        GLFWwindowposfun       position{nullptr};
        GLFWwindowfocusfun     focus{nullptr};
        GLFWwindowiconifyfun   iconify{nullptr};
        GLFWwindowmaximizefun  maximize{nullptr};
        GLFWwindowclosefun     close{nullptr};
    };

    /**
     * The adapters, by GLFW window: the window's user pointer is the host's
     */
    namespace {
        std::vector<GLFWSystemWindow *> &adapters() {
            static std::vector<GLFWSystemWindow *> adapters{};
            return adapters;
        }
    }

    GLFWSystemWindow *GLFWSystemWindow::find(GLFWwindow *glfwWindow) {
        for (auto *adapter : adapters()) {
            if (adapter->_glfwWindow == glfwWindow) {
                return adapter;
            }
        }
        return nullptr;
    }

    GLFWSystemWindow::GLFWSystemWindow(std::shared_ptr<Window> window, GLFWwindow *glfwWindow) :
        SystemWindow(std::move(window)),
        _glfwWindow(glfwWindow),
        _previous(std::make_unique<Callbacks>()) {

        if (!_glfwWindow) {
            throw std::invalid_argument("GLFWSystemWindow needs a GLFW window");
        }
        if (find(_glfwWindow)) {
            throw std::invalid_argument("This GLFW window already has a Psychic UI adapter");
        }

        // The host may draw with this context too
        _sharesContext = true;

        glfwGetWindowPos(_glfwWindow, &_x, &_y);
        readSize();
        _minimized = glfwGetWindowAttrib(_glfwWindow, GLFW_ICONIFIED) != 0;
        _maximized = glfwGetWindowAttrib(_glfwWindow, GLFW_MAXIMIZED) != 0;
        _focused   = glfwGetWindowAttrib(_glfwWindow, GLFW_FOCUSED) != 0;

        if (glfwGetWindowAttrib(_glfwWindow, GLFW_CLIENT_API) != GLFW_NO_API) {
            glfwMakeContextCurrent(_glfwWindow);
            readFramebufferFormat();
        }

        // In the order of `Cursor`
        _cursors[static_cast<int>(Cursor::Arrow)]     = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        _cursors[static_cast<int>(Cursor::IBeam)]     = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        _cursors[static_cast<int>(Cursor::Crosshair)] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        _cursors[static_cast<int>(Cursor::Hand)]      = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
        _cursors[static_cast<int>(Cursor::HResize)]   = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);
        _cursors[static_cast<int>(Cursor::VResize)]   = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);

        adapters().push_back(this);

        // Each callback calls the host's first, then ours
        auto &p = *_previous;
        p.cursorPos = glfwSetCursorPosCallback(_glfwWindow, [](GLFWwindow *w, double x, double y) {
            if (auto *a = find(w)) {
                if (a->_previous->cursorPos) { a->_previous->cursorPos(w, x, y); }
                a->cursorPosEvent(x, y);
            }
        });
        p.mouseButton = glfwSetMouseButtonCallback(_glfwWindow, [](GLFWwindow *w, int button, int action, int mods) {
            if (auto *a = find(w)) {
                if (a->_previous->mouseButton) { a->_previous->mouseButton(w, button, action, mods); }
                a->mouseButtonEvent(button, action, mods);
            }
        });
        p.scroll = glfwSetScrollCallback(_glfwWindow, [](GLFWwindow *w, double x, double y) {
            if (auto *a = find(w)) {
                if (a->_previous->scroll) { a->_previous->scroll(w, x, y); }
                a->scrollEvent(x, y);
            }
        });
        p.key = glfwSetKeyCallback(_glfwWindow, [](GLFWwindow *w, int key, int scancode, int action, int mods) {
            if (auto *a = find(w)) {
                if (a->_previous->key) { a->_previous->key(w, key, scancode, action, mods); }
                a->keyEvent(key, scancode, action, mods);
            }
        });
        p.character = glfwSetCharCallback(_glfwWindow, [](GLFWwindow *w, unsigned int codepoint) {
            if (auto *a = find(w)) {
                if (a->_previous->character) { a->_previous->character(w, codepoint); }
                a->charEvent(codepoint);
            }
        });
        p.drop = glfwSetDropCallback(_glfwWindow, [](GLFWwindow *w, int count, const char **paths) {
            if (auto *a = find(w)) {
                if (a->_previous->drop) { a->_previous->drop(w, count, paths); }
                a->dropEvent(count, paths);
            }
        });
        p.size = glfwSetWindowSizeCallback(_glfwWindow, [](GLFWwindow *w, int width, int height) {
            if (auto *a = find(w)) {
                if (a->_previous->size) { a->_previous->size(w, width, height); }
                a->sizeEvent();
            }
        });
        p.framebufferSize = glfwSetFramebufferSizeCallback(_glfwWindow, [](GLFWwindow *w, int width, int height) {
            if (auto *a = find(w)) {
                if (a->_previous->framebufferSize) { a->_previous->framebufferSize(w, width, height); }
                a->sizeEvent();
            }
        });
        p.position = glfwSetWindowPosCallback(_glfwWindow, [](GLFWwindow *w, int x, int y) {
            if (auto *a = find(w)) {
                if (a->_previous->position) { a->_previous->position(w, x, y); }
                a->positionEvent(x, y);
            }
        });
        p.focus = glfwSetWindowFocusCallback(_glfwWindow, [](GLFWwindow *w, int focused) {
            if (auto *a = find(w)) {
                if (a->_previous->focus) { a->_previous->focus(w, focused); }
                a->focusEvent(focused);
            }
        });
        p.iconify = glfwSetWindowIconifyCallback(_glfwWindow, [](GLFWwindow *w, int iconified) {
            if (auto *a = find(w)) {
                if (a->_previous->iconify) { a->_previous->iconify(w, iconified); }
                a->iconifyEvent(iconified);
            }
        });
        p.maximize = glfwSetWindowMaximizeCallback(_glfwWindow, [](GLFWwindow *w, int maximized) {
            if (auto *a = find(w)) {
                if (a->_previous->maximize) { a->_previous->maximize(w, maximized); }
                a->maximizeEvent(maximized);
            }
        });
        p.close = glfwSetWindowCloseCallback(_glfwWindow, [](GLFWwindow *w) {
            if (auto *a = find(w)) {
                if (a->_previous->close) { a->_previous->close(w); }
                a->closeEvent();
            }
        });

        _window->open(this);
    }

    GLFWSystemWindow::~GLFWSystemWindow() {
        if (glfwGetWindowAttrib(_glfwWindow, GLFW_CLIENT_API) != GLFW_NO_API) {
            glfwMakeContextCurrent(_glfwWindow);
        }
        detach();

        // Give the host its callbacks back
        const auto &p = *_previous;
        glfwSetCursorPosCallback(_glfwWindow, p.cursorPos);
        glfwSetMouseButtonCallback(_glfwWindow, p.mouseButton);
        glfwSetScrollCallback(_glfwWindow, p.scroll);
        glfwSetKeyCallback(_glfwWindow, p.key);
        glfwSetCharCallback(_glfwWindow, p.character);
        glfwSetDropCallback(_glfwWindow, p.drop);
        glfwSetWindowSizeCallback(_glfwWindow, p.size);
        glfwSetFramebufferSizeCallback(_glfwWindow, p.framebufferSize);
        glfwSetWindowPosCallback(_glfwWindow, p.position);
        glfwSetWindowFocusCallback(_glfwWindow, p.focus);
        glfwSetWindowIconifyCallback(_glfwWindow, p.iconify);
        glfwSetWindowMaximizeCallback(_glfwWindow, p.maximize);
        glfwSetWindowCloseCallback(_glfwWindow, p.close);

        std::erase(adapters(), this);

        for (auto &cursor : _cursors) {
            if (cursor) {
                glfwDestroyCursor(cursor);
            }
        }
    }

    GLFWwindow *GLFWSystemWindow::glfwWindow() const {
        return _glfwWindow;
    }

    void GLFWSystemWindow::readSize() {
        glfwGetWindowSize(_glfwWindow, &_width, &_height);
        glfwGetFramebufferSize(_glfwWindow, &_fbWidth, &_fbHeight);
    }

    bool GLFWSystemWindow::render() {
        if (!_window->windowVisible()) {
            // Closed from inside: let the host's loop know
            glfwSetWindowShouldClose(_glfwWindow, GLFW_TRUE);
            return false;
        }

        glfwMakeContextCurrent(_glfwWindow);
        _window->drawAll();
        return true;
    }

    void *GLFWSystemWindow::glProcAddress(const char *name) const {
        return std::bit_cast<void *>(glfwGetProcAddress(name));
    }

    // region Window operations

    void GLFWSystemWindow::setSize(int width, int height) {
        glfwSetWindowSize(_glfwWindow, width, height);
    }

    void GLFWSystemWindow::setPosition(int x, int y) {
        glfwSetWindowPos(_glfwWindow, x, y);
    }

    void GLFWSystemWindow::setTitle(const std::string &title) {
        glfwSetWindowTitle(_glfwWindow, title.c_str());
    }

    void GLFWSystemWindow::setFullscreen(bool fullscreen) {
        if (fullscreen) {
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
            if (!mode) {
                return;
            }
            glfwGetWindowPos(_glfwWindow, &_windowedX, &_windowedY);
            glfwGetWindowSize(_glfwWindow, &_windowedWidth, &_windowedHeight);
            glfwSetWindowMonitor(_glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else if (glfwGetWindowMonitor(_glfwWindow)) {
            glfwSetWindowMonitor(_glfwWindow, nullptr, _windowedX, _windowedY, _windowedWidth, _windowedHeight, GLFW_DONT_CARE);
        }
    }

    bool GLFWSystemWindow::getMinimized() const {
        return _minimized;
    }

    void GLFWSystemWindow::setMinimized(bool minimized) {
        if (minimized) {
            glfwIconifyWindow(_glfwWindow);
        } else {
            glfwRestoreWindow(_glfwWindow);
        }
    }

    bool GLFWSystemWindow::getMaximized() const {
        return _maximized;
    }

    void GLFWSystemWindow::setMaximized(bool maximized) {
        if (maximized) {
            glfwMaximizeWindow(_glfwWindow);
        } else {
            glfwRestoreWindow(_glfwWindow);
        }
    }

    void GLFWSystemWindow::setVisible(bool visible) {
        if (visible) {
            glfwShowWindow(_glfwWindow);
        } else {
            glfwHideWindow(_glfwWindow);
        }
    }

    void GLFWSystemWindow::setCursor(Cursor cursor) {
        if (static_cast<std::size_t>(cursor) < _cursors.size()) {
            glfwSetCursor(_glfwWindow, _cursors.at(static_cast<std::size_t>(cursor)));
        }
    }

    bool GLFWSystemWindow::supportsDrag() const {
        // GLFW has no way to hand a move to the compositor, and Wayland doesn't let it place windows
        return glfwGetPlatform() != GLFW_PLATFORM_WAYLAND;
    }

    void GLFWSystemWindow::wake() {
        glfwPostEmptyEvent();
    }

    void GLFWSystemWindow::startDrag() {
        if (!supportsDrag()) {
            if (!_warnedAboutDrag) {
                _warnedAboutDrag = true;
                std::cerr << "Psychic UI: GLFW can't move windows on Wayland; use system decorations "
                             "instead of a custom title bar (see Window::supportsDrag)" << '\n';
            }
            return;
        }
        glfwGetCursorPos(_glfwWindow, &_dragX, &_dragY);
        _dragging = true;
    }

    void GLFWSystemWindow::stopDrag() {
        _dragging = false;
    }

    std::string GLFWSystemWindow::getClipboardText() const {
        const char *text = glfwGetClipboardString(_glfwWindow);
        return text ? text : "";
    }

    void GLFWSystemWindow::setClipboardText(const std::string &text) {
        glfwSetClipboardString(_glfwWindow, text.c_str());
    }

    // endregion

    // region Events

    void GLFWSystemWindow::cursorPosEvent(double x, double y) {
        _mouseX = static_cast<float>(x);
        _mouseY = static_cast<float>(y);

        if (_dragging) {
            // The grabbed point follows the mouse; once moved, the cursor is back at it in window coordinates
            int windowX = 0;
            int windowY = 0;
            glfwGetWindowPos(_glfwWindow, &windowX, &windowY);
            glfwSetWindowPos(_glfwWindow, windowX + static_cast<int>(x - _dragX), windowY + static_cast<int>(y - _dragY));
        }

        _window->handleMouseMove({.x = _mouseX, .y = _mouseY, .modifiers = _modifiers}, false);
    }

    void GLFWSystemWindow::mouseButtonEvent(int button, int action, int mods) {
        _modifiers = mapMods(mods);

        MouseButton btn = MouseButton::NONE;
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: btn = MouseButton::LEFT; break;
            case GLFW_MOUSE_BUTTON_MIDDLE: btn = MouseButton::MIDDLE; break;
            case GLFW_MOUSE_BUTTON_RIGHT: btn = MouseButton::RIGHT; break;
            default: return;
        }

        // GLFW doesn't count clicks: the window does
        _window->handleMouseButtonChange({
            .x = _mouseX, .y = _mouseY, .button = btn, .down = action == GLFW_PRESS, .modifiers = _modifiers
        });
    }

    void GLFWSystemWindow::scrollEvent(double x, double y) {
        // GLFW gives no modifiers with the scroll: the ones of the last key or button event
        _window->handleMouseScroll({.x = _mouseX, .y = _mouseY, .modifiers = _modifiers, .scrollX = x, .scrollY = y});
    }

    void GLFWSystemWindow::keyEvent(int key, int /*scancode*/, int action, int mods) {
        _modifiers = mapMods(mods);
        switch (action) {
            case GLFW_PRESS: _window->handleKeyPress(mapKey(key), _modifiers); break;
            case GLFW_REPEAT: _window->handleKeyRepeat(mapKey(key), _modifiers); break;
            case GLFW_RELEASE: _window->handleKeyRelease(mapKey(key), _modifiers); break;
            default: break;
        }
    }

    void GLFWSystemWindow::charEvent(unsigned int codepoint) {
        _window->handleType(icu::UnicodeString(static_cast<UChar32>(codepoint)));
    }

    void GLFWSystemWindow::dropEvent(int count, const char **paths) {
        const std::span<const char *> dropped{paths, static_cast<std::size_t>(count)};
        _window->dropEvent(std::vector<std::string>(dropped.begin(), dropped.end()));
    }

    void GLFWSystemWindow::sizeEvent() {
        readSize();
        // Minimized
        if (_width == 0 || _height == 0) {
            return;
        }
        _window->windowResized(_width, _height);
    }

    void GLFWSystemWindow::positionEvent(int x, int y) {
        _x = x;
        _y = y;
        _window->windowMoved(_x, _y);
    }

    void GLFWSystemWindow::focusEvent(int focused) {
        _focused = focused == GLFW_TRUE;
        if (_focused) {
            _window->windowActivated();
        } else {
            _window->windowDeactivated();
        }
    }

    void GLFWSystemWindow::iconifyEvent(int iconified) {
        _minimized = iconified == GLFW_TRUE;
        if (_minimized) {
            _window->windowMinimized();
        } else {
            _window->windowRestored();
        }
    }

    void GLFWSystemWindow::maximizeEvent(int maximized) {
        _maximized = maximized == GLFW_TRUE;
    }

    void GLFWSystemWindow::closeEvent() {
        // The host decides whether the window closes; the UI can only keep it open
        if (!_window->windowShouldClose()) {
            glfwSetWindowShouldClose(_glfwWindow, GLFW_FALSE);
        }
    }

    // endregion

    // region Mapping

    Mod GLFWSystemWindow::mapMods(int mods) {
        Mod mod{};
        mod.shift = (mods & GLFW_MOD_SHIFT) != 0;
        mod.ctrl  = (mods & GLFW_MOD_CONTROL) != 0;
        mod.alt   = (mods & GLFW_MOD_ALT) != 0;
        mod.super = (mods & GLFW_MOD_SUPER) != 0;
        return mod;
    }

    Key GLFWSystemWindow::mapKey(int key) {
        switch (key) {
            case GLFW_KEY_SPACE: return Key::SPACE;
            case GLFW_KEY_APOSTROPHE: return Key::APOSTROPHE;
            case GLFW_KEY_COMMA: return Key::COMMA;
            case GLFW_KEY_MINUS: return Key::MINUS;
            case GLFW_KEY_PERIOD: return Key::PERIOD;
            case GLFW_KEY_SLASH: return Key::SLASH;
            case GLFW_KEY_SEMICOLON: return Key::SEMICOLON;
            case GLFW_KEY_EQUAL: return Key::EQUAL;
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
            default: break;
        }
        // GLFW's contiguous ranges, in the same order as `Key`'s
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
            return static_cast<Key>(static_cast<int>(Key::ZERO) + key - GLFW_KEY_0);
        }
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
            return static_cast<Key>(static_cast<int>(Key::A) + key - GLFW_KEY_A);
        }
        if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25) {
            return static_cast<Key>(static_cast<int>(Key::F1) + key - GLFW_KEY_F1);
        }
        if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) {
            return static_cast<Key>(static_cast<int>(Key::KP_0) + key - GLFW_KEY_KP_0);
        }
        return Key::UNKNOWN;
    }

    // endregion
}
