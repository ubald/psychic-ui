#include <cstring>
#include <iostream>
#include <utility>
#include "GrBackendSurface.h"
#include "Window.hpp"
#include "SkSurface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"


namespace psychic_ui {

    Window::Window(std::string title) :
        Div::Div(),
        _title(std::move(title)) {
        setStyleManager(StyleManager::getInstance()); // NOTE: Each window could get its own style manager
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
            [this](const int /*mouseX*/, const int /*mouseY*/, const int /*button*/, const Mod /*modifiers*/) {
                closeMenu();
            }
        );
        menu->style()
            ->set(visible, false);
    }

    Window::~Window() {
        releaseSkia();
    }

    // region Hierarchy

    Window *Window::window() {
        return this;
    }

    // endregion

    // region Lifecycle

    void Window::open(SystemWindow *systemWindow) {
        // Skia's GPU objects belong to the previous system window's GL context
        releaseSkia();
        _systemWindow = systemWindow;

        // Performance
        lastReport = std::chrono::high_resolution_clock::now();
    }

    SystemWindow *Window::systemWindow() const {
        return _systemWindow;
    }

    void Window::close() {
        // TODO: Find a better application-friendly close method
        _visible = false;
    }

    void Window::initSkia() {
        // The GL functions come from the system window's loader (SDL, GLFW), not from Skia's native
        // interface, which only knows GLX on Linux and fails in an EGL context (Wayland)
        auto interface = GrGLMakeAssembledInterface(
            _systemWindow,
            [](void *ctx, const char name[]) -> GrGLFuncPtr {
                // Skia also asks for eglQueryString, only to list EGL extensions it can do without. A GLX
                // loader returns a stub for any name, which crashes when called, so it gets none.
                if (std::strncmp(name, "egl", 3) == 0) {
                    return nullptr;
                }
                return reinterpret_cast<GrGLFuncPtr>(static_cast<SystemWindow *>(ctx)->glProcAddress(name));
            }
        );
        if (!interface) {
            throw std::runtime_error("Could not get the OpenGL functions Skia needs");
        }
        _sk_context = GrContext::MakeGL(interface).release();
        if (!_sk_context) {
            throw std::runtime_error("Could not create a Skia OpenGL context");
        }
        _surfaceStale = true;
    }

    void Window::releaseSkia() {
        delete _sk_surface;
        _sk_surface = nullptr;
        _sk_canvas  = nullptr;
        delete _sk_context;
        _sk_context   = nullptr;
        _surfaceStale = true;
    }

    void Window::getSkiaSurface() {
        if (!_systemWindow) {
            throw std::runtime_error("Skia surface requested without a context");
        }

        delete _sk_surface;
        _sk_surface   = nullptr;
        _sk_canvas    = nullptr;
        _surfaceStale = false;

        GrGLFramebufferInfo framebufferInfo{};
        framebufferInfo.fFBOID = 0;  // assume default framebuffer
        framebufferInfo.fFormat = GR_GL_RGBA8;


        // In pixels; `drawAll` scales the canvas to the window's units
        GrBackendRenderTarget backendRenderTarget(
            _systemWindow->getFramebufferWidth(),
            _systemWindow->getFramebufferHeight(),
            _systemWindow->getSamples(),
            _systemWindow->getStencilBits(),
            framebufferInfo
        );

        // setup SkSurface
        // To use distance field text, use commented out SkSurfaceProps instead
        // SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
        //                      SkSurfaceProps::kLegacyFontHost_InitType);
        SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

        _sk_surface = SkSurface::MakeFromBackendRenderTarget(
            _sk_context,
            backendRenderTarget,
            kBottomLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            nullptr,
            &props
        ).release();
        if (!_sk_surface) {
            SkDebugf("SkSurface::MakeFromBackendRenderTarget returned null\n");
            return;
        }
        _sk_canvas = _sk_surface->getCanvas();
    }

    // endregion

    // region Window Attributes

    std::string Window::getTitle() const {
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

    bool Window::supportsDrag() const {
        return _systemWindow && _systemWindow->supportsDrag();
    }

    void Window::addDragArea(const std::shared_ptr<Div> &div) {
        std::erase_if(_dragAreas, [](const auto &area) { return area.expired(); });
        _dragAreas.push_back(div);
    }

    /**
     * Whether the point, in `div`'s coordinates, is on a visible descendant that handles mouse buttons or clicks
     */
    static bool interactiveChildAt(const Div &div, const int x, const int y) {
        for (const auto &child : div.children()) {
            const int localX = x - child->x();
            const int localY = y - child->y();
            if (!child->getVisible() || !child->mouseEnabled()
                || localX < 0 || localY < 0 || localX >= child->getWidth() || localY >= child->getHeight()) {
                continue;
            }
            if (child->onMouseButton.hasSubscriptions() || child->onMouseDown.hasSubscriptions()
                || child->onMouseUp.hasSubscriptions() || child->onClick.hasSubscriptions()
                || interactiveChildAt(*child, localX, localY)) {
                return true;
            }
        }
        return false;
    }

    bool Window::isDragArea(const int x, const int y) {
        // An open menu or modal takes every press
        if (menu->getVisible() || modal->getVisible()) {
            return false;
        }

        for (const auto &weak : _dragAreas) {
            auto area = weak.lock();
            if (!area || !area->getVisible() || area->window() != this) {
                continue;
            }

            int localX = 0;
            int localY = 0;
            area->globalToLocal(localX, localY, x, y);
            if (localX >= 0 && localY >= 0 && localX < area->getWidth() && localY < area->getHeight()) {
                return !interactiveChildAt(*area, localX, localY);
            }
        }
        return false;
    }

    int Window::windowX() const {
        return _systemWindow ? _systemWindow->getX() : 0;
    }

    int Window::windowY() const {
        return _systemWindow ? _systemWindow->getY() : 0;
    }

    void Window::setWindowPosition(const int x, const int y) {
        if (_systemWindow) {
            _systemWindow->setPosition(x, y);
        }
    }

    int Window::windowWidth() const {
        return _systemWindow ? _systemWindow->getWidth() : _windowWidth;
    }

    int Window::windowHeight() const {
        return _systemWindow ? _systemWindow->getHeight() : _windowHeight;
    }

    void Window::setWindowSize(const int width, const int height) {
        _windowWidth  = width;
        _windowHeight = height;
        if (_systemWindow) {
            _systemWindow->setSize(width, height);
        }
    }

    // endregion

    // region Draw

    void Window::updateStylesAndLayout() {
        // Styles first, since they are the layout's input
        if (!_styleManager->valid()) {
            updateStyleRecursive();
            _styleManager->setValid();
        }

        // `styleUpdated` callbacks can invalidate styles, so run the pass again until nothing is dirty,
        // but a bounded number of times so that a callback that always invalidates can't hang the frame:
        // what is left waits for the next frame.
        for (int pass = 0; pass < maxStylePasses && styleDirty(); ++pass) {
            updateDirtyStyles();
        }

        // Then the layout, at the window's current size: the root has no width or height of its own,
        // it fills the size given here
        const int width  = windowWidth();
        const int height = windowHeight();
        if (YGNodeIsDirty(_yogaNode) || width != _width || height != _height) {
            #ifdef DEBUG_LAYOUT
            if (debugLayout) {
                std::cout << "Layout dirty!" << std::endl;
            }
            #endif
            YGNodeCalculateLayout(_yogaNode, width, height, YGDirectionLTR);
            layoutUpdated();
        }
    }

    void Window::drawAll() {
        if (!_visible || !_systemWindow) {
            return;
        }

        updateStylesAndLayout();

        // Skia is set up here rather than in `open` because a GL context is only known to be current
        // while drawing (an embedding host makes it current for its own frame)
        if (!_sk_context) {
            initSkia();
        } else if (_systemWindow->sharesContext()) {
            // The host drew with the same context since our last frame
            _sk_context->resetContext();
        }
        if (_surfaceStale) {
            getSkiaSurface();
        }
        if (!_sk_canvas) {
            return;
        }

        _sk_canvas->clear(0x00000000);
        // Layout is in window units, the surface in pixels
        _sk_canvas->save();
        const float pixelRatio = _systemWindow->getPixelRatio();
        _sk_canvas->scale(pixelRatio, pixelRatio);
        render(_sk_canvas);
        _sk_canvas->restore();
        _sk_canvas->flush();

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

    // region Focus

    void Window::requestFocus(Div *component) {
        // Build the path
        std::vector<Div *> path{};
        Div                *c = component;
        while (c != nullptr) {
            path.insert(path.begin(), c);
            c = c->parent();
        }

        // Compare with current and unfocus
        for (const auto &focused: _focusPath) {
            auto it = std::find(path.cbegin(), path.cend(), focused);
            if (it == path.cend()) {
                focused->setFocused(false);
                focused->onBlur();
            }
        }

        // Set new focus path
        _focusPath = std::move(path);
        for (const auto &focused: _focusPath) {
            focused->setFocused(true);
            focused->onFocus();
        }
    }

    // endregion

    // region MouseEvents

    MouseEventStatus Window::mouseButton(int mouseX, int mouseY, MouseButton button, bool down, Mod modifiers) {
        auto res = Div::mouseButton(mouseX, mouseY, button, down, modifiers);

        if (down) {
            mouseDown(mouseX, mouseY, button, modifiers);
        } else {
            click(mouseX, mouseY, button, modifiers);

            if (button == MouseButton::LEFT) {
                auto   now   = std::chrono::high_resolution_clock::now();
                double delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastClick).count();
                if (delta <= 500) {
                    ++_clickCount;
                    doubleClick(mouseX, mouseY, _clickCount, modifiers);
                } else {
                    _clickCount = 1;
                }
                _lastClick = now;
            }

            mouseUp(mouseX, mouseY, button, modifiers);
        }

        return res;
    }

    // endregion

    // region Clipboard

    std::string Window::clipboardText() const {
        return _systemWindow ? _systemWindow->getClipboardText() : std::string{};
    }

    void Window::setClipboardText(const std::string &text) {
        if (_systemWindow) {
            _systemWindow->setClipboardText(text);
        }
    }

    // endregion

    // region Keyboard Events

    void Window::startTextInput() {
        if (_systemWindow) {
            _systemWindow->startTextInput();
        }
    }

    void Window::stopTextInput() {
        if (_systemWindow) {
            _systemWindow->stopTextInput();
        }
    }

    bool Window::keyDown(Key key, Mod mod) {
        // Go backwards since we want to cancel as soon as possible when a child handles it
        for (auto focused = _focusPath.rbegin(); focused != _focusPath.rend(); ++focused) {
            // Everyone in the focus path gets the key events, focusEnabled or not
            Div *div = (*focused);
            if (div->onKeyDown.hasSubscriptions()) {
                div->onKeyDown(key, mod);
                return true;
            }
        }
        return false;
    }

    bool Window::keyRepeat(Key key, Mod mod) {
        // Go backwards since we want to cancel as soon as possible when a child handles it
        for (auto focused = _focusPath.rbegin(); focused != _focusPath.rend(); ++focused) {
            // Everyone in the focus path gets the key events, focusEnabled or not
            Div *div = (*focused);
            if (div->onKeyRepeat.hasSubscriptions()) {
                div->onKeyRepeat(key, mod);
                return true;
            }
        }
        return false;
    }

    bool Window::keyUp(Key key, Mod mod) {
        // Go backwards since we want to cancel as soon as possible when a child handles it
        for (auto focused = _focusPath.rbegin(); focused != _focusPath.rend(); ++focused) {
            // Everyone in the focus path gets the key events, focusEnabled or not
            Div *div = (*focused);
            if (div->onKeyUp.hasSubscriptions()) {
                div->onKeyUp(key, mod);
                return true;
            }
        }
        return false;
    }

    bool Window::keyboardCharacterEvent(const icu::UnicodeString &character) {
        // Go backwards since we want to cancel as soon as possible when a child handles it
        for (auto focused = _focusPath.rbegin(); focused != _focusPath.rend(); ++focused) {
            // Only the focusEnabled divs get the character events
            Div *div = (*focused);
            if (div->onCharacter.hasSubscriptions()) {
                div->onCharacter(character);
                return true;
            }
        }
        return false;
    }

    // endregion

    // region Callback Delegates

    void Window::windowMoved(const int x, const int y) {
        // std::cout << "Moved" << std::endl;
    }

    void Window::windowResized(const int width, const int height) {
        // The system window already holds the new size, the next frame lays out the root at it
        _windowWidth  = width;
        _windowHeight = height;

        // The next frame gets a new surface, at the new framebuffer size
        _surfaceStale = true;
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

}
