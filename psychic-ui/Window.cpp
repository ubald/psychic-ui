#include <algorithm>
#include <cmath>
#include <limits>
#include <cstring>
#include <iostream>
#include <utility>
#include <yoga/Yoga.h>
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "Window.hpp"

namespace {
    // GL_RGBA8, the default framebuffer's format
    constexpr GrGLenum GL_RGBA8_FORMAT = 0x8058;
}


namespace psychic_ui {

    Window::Window(std::string title) :
        Div::Div(),
        _title(std::move(title)) {
        setStyleManager(StyleManager::getInstance()); // NOTE: Each window could get its own style manager
        // A window starts in pointer mode. The style manager is shared: its density follows the window that changed
        // its input mode last.
        styleManager()->density(Density::Pointer);
        tag("Window");
        setWindowSize(1440, 900);

        _inlineStyle.position(Position::Absolute)
            .overflow(Overflow::Hidden);

        app = &add<Div>()
            .id("app")
            .position(Position::Absolute)
            .width(100_pct)
            .height(100_pct)
            .overflow(Overflow::Hidden);

        modal = &add<Modal>()
            .id("modal")
            .visible(false);

        menu = &add<Modal>()
            .id("menu")
            .visible(false);
        menu->mousePressed.subscribe(
            [this](const int /*mouseX*/, const int /*mouseY*/, const int /*button*/, const Mod /*modifiers*/) {
                closeMenu();
            }
        );
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
        if (_systemWindow) {
            handleColorSchemeChange(_systemWindow->colorScheme());
        }

        // Performance
        lastReport = std::chrono::high_resolution_clock::now();
    }

    SystemWindow *Window::systemWindow() const {
        return _systemWindow;
    }

    void Window::close() {
        // TODO: Find a better application-friendly close method
        setWindowVisible(false);
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
        _sk_context = GrDirectContexts::MakeGL(interface);
        if (!_sk_context) {
            throw std::runtime_error("Could not create a Skia OpenGL context");
        }
        _surfaceStale = true;
    }

    void Window::releaseSkia() {
        // The surface before the context it was made with
        _sk_surface   = nullptr;
        _sk_canvas    = nullptr;
        _sk_context   = nullptr;
        _surfaceStale = true;
    }

    void Window::getSkiaSurface() {
        if (!_systemWindow) {
            throw std::runtime_error("Skia surface requested without a context");
        }

        _sk_surface   = nullptr;
        _sk_canvas    = nullptr;
        _surfaceStale = false;

        GrGLFramebufferInfo framebufferInfo{};
        framebufferInfo.fFBOID  = 0; // The default framebuffer
        framebufferInfo.fFormat = GL_RGBA8_FORMAT;

        // In pixels; `drawAll` scales the canvas to the window's units
        GrBackendRenderTarget backendRenderTarget = GrBackendRenderTargets::MakeGL(
            _systemWindow->getFramebufferWidth(),
            _systemWindow->getFramebufferHeight(),
            _systemWindow->getSamples(),
            _systemWindow->getStencilBits(),
            framebufferInfo
        );

        // Horizontal RGB subpixels, for LCD text. Skia's default text gamma (sRGB, contrast 0.5) thickens light text on
        // dark and thins dark text on light, so the same font looked a weight heavier in a dark theme; Chrome's values
        // on Linux (contrast 0.2, gamma 1.2) keep the weight the same both ways, as Pivotal looks in a browser.
        constexpr SkScalar textContrast = 0.2f;
        constexpr SkScalar textGamma    = 1.2f;
        SkSurfaceProps props(0, kRGB_H_SkPixelGeometry, textContrast, textGamma);

        _sk_surface = SkSurfaces::WrapBackendRenderTarget(
            _sk_context.get(),
            backendRenderTarget,
            kBottomLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            nullptr,
            &props
        );
        if (!_sk_surface) {
            std::cerr << "Psychic UI: Skia could not wrap the window's framebuffer" << std::endl;
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

    bool Window::windowVisible() const {
        return _windowVisible;
    }

    void Window::setWindowVisible(bool value) {
        if (_windowVisible != value) {
            _windowVisible = value;
            if (_systemWindow) {
                _systemWindow->setVisible(_windowVisible);
            }
        }
    }

    void Window::setCursor(Cursor cursor) {
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
            if (!child->visible() || !child->mouseEnabled()
                || localX < 0 || localY < 0 || localX >= child->getWidth() || localY >= child->getHeight()) {
                continue;
            }
            if (child->mouseButtonChanged.hasSubscriptions() || child->mousePressed.hasSubscriptions()
                || child->mouseReleased.hasSubscriptions() || child->clicked.hasSubscriptions()
                || interactiveChildAt(*child, localX, localY)) {
                return true;
            }
        }
        return false;
    }

    bool Window::isDragArea(const int x, const int y) {
        // An open menu or modal takes every press
        if (menu->visible() || modal->visible()) {
            return false;
        }

        for (const auto &weak : _dragAreas) {
            auto area = weak.lock();
            if (!area || !area->visible() || area->window() != this) {
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
        if (!_windowVisible || !_systemWindow) {
            return;
        }

        updateGestures(_systemWindow->gestureTime());

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
        _sk_context->flushAndSubmit();

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
        menu->add<Menu>(items)
            .left(lx)
            .top(ly);
        menu->visible(true);
        menuOpened.emit();
    }

    void Window::closeMenu() {
        menu->removeAll();
        menu->visible(false);
        menuClosed.emit();
    }

    // endregion

    void Window::updateTheme(const Theme &theme) {
        _followSystemTheme = false;
        applyTheme(theme);
    }

    void Window::applyTheme(const Theme &theme) {
        styleManager()->theme(theme);
        updateRuntimeStyles(); // Clearing the rules dropped the runtime ones too
    }

    void Window::updateSkinSet(std::unique_ptr<SkinSet> skinSet) {
        // With its default theme, or its light one when following a light system
        styleManager()->skinSet(std::move(skinSet), _followSystemTheme ? _colorScheme : ColorScheme::Unknown);
        updateRuntimeStyles(); // Clearing the rules dropped the runtime ones too
        if (_followSystemTheme && (_lightTheme || _darkTheme)) {
            applySystemTheme(); // The window's own pair rather than the set's
        }
    }

    void Window::updateFollowSystemTheme(const Theme *light, const Theme *dark) {
        _followSystemTheme = true;
        _lightTheme        = light ? std::optional<Theme>{*light} : std::nullopt;
        _darkTheme         = dark ? std::optional<Theme>{*dark} : std::nullopt;
        applySystemTheme();
    }

    const Theme &Window::systemTheme() const {
        if (_colorScheme == ColorScheme::Light) {
            if (_lightTheme) {
                return *_lightTheme;
            }
        } else if (_darkTheme) {
            return *_darkTheme;
        }
        const SkinSet *set = styleManager()->skinSet();
        return set ? set->theme(_colorScheme) : styleManager()->theme();
    }

    void Window::applySystemTheme() {
        const Theme &theme = systemTheme();
        if (theme.name != styleManager()->theme().name || theme.scheme != styleManager()->theme().scheme) {
            applyTheme(Theme{theme}); // A copy: `theme` may be the style manager's own
        }
    }

    void Window::handleColorSchemeChange(const ColorScheme scheme) {
        if (scheme == _colorScheme) {
            return;
        }
        _colorScheme = scheme;
        if (_followSystemTheme) {
            applySystemTheme();
        }
        colorSchemeChanged.emit(_colorScheme);
    }

    // region Input Mode

    void Window::handleInputModeChange(const InputMode mode) {
        if (mode == _inputMode) {
            return;
        }
        const bool touchChanged = (mode == InputMode::Touch) != (_inputMode == InputMode::Touch);
        _inputMode = mode;
        if (touchChanged) {
            // The stylesheets size controls for fingers or for a pointer, and `:touch` matches differently
            if (styleManager()->density(mode == InputMode::Touch ? Density::Touch : Density::Pointer)) {
                updateRuntimeStyles(); // Reloading the stylesheets dropped the runtime rules
            }
            styleManager()->invalidate();
        }
        inputModeChanged.emit(_inputMode);
    }

    // endregion

    // region Touch

    namespace {
        int coordinate(const float value) {
            return static_cast<int>(std::floor(value));
        }

        float seconds(const Window::GestureTime time) {
            return std::chrono::duration<float>(time).count();
        }

        /**
         * The divs a press at this point (in `div`'s parent's coordinates) goes through, outermost first,
         * found the way the mouse events are dispatched
         */
        bool hitPath(Div *div, const int x, const int y, std::vector<Div *> &path) {
            if (!div->visible() || !div->boundsContains(x, y)) {
                return false;
            }
            path.push_back(div);
            if (div->mouseChildren()) {
                const int localX = x - div->x() - div->scrollX();
                const int localY = y - div->y() - div->scrollY();
                const auto &children = div->children();
                for (auto child = children.rbegin(); child != children.rend(); ++child) {
                    if (hitPath(child->get(), localX, localY, path)) {
                        break;
                    }
                }
            }
            return true;
        }
    }

    Div *Window::scrollTargetAt(const int x, const int y, const bool horizontal) {
        std::vector<Div *> path{};
        hitPath(this, x, y, path);
        for (auto div = path.rbegin(); div != path.rend(); ++div) {
            if ((*div)->computedStyle().overflow() == Overflow::Scroll
                && (horizontal ? (*div)->getWidth() < (*div)->contentWidth() : (*div)->getHeight() < (*div)->contentHeight())) {
                return *div;
            }
        }
        return nullptr;
    }

    void Window::handleTouchPress(const float x, const float y, const GestureTime time) {
        handleInputModeChange(InputMode::Touch);
        if (_touchPhase != TouchPhase::None) {
            return; // One finger at a time
        }
        _touchStartX = _touchX = x;
        _touchStartY = _touchY = y;
        _touchStart  = _touchTime = time;
        if (_flinging) {
            // A finger stops kinetic scrolling, and does nothing else
            _flinging   = false;
            _touchPhase = TouchPhase::Ignored;
            return;
        }
        _touchPhase = TouchPhase::Pending;
    }

    void Window::touchPressButton(const MouseButton button) {
        // Where the finger landed, getting there first as a mouse would
        const int x = coordinate(_touchStartX);
        const int y = coordinate(_touchStartY);
        handleMouseMove(x, y, 0, Mod{}, false);
        _touchPhase  = TouchPhase::Pressing;
        _touchButton = button;
        handleMouseButtonChange(x, y, button, true, Mod{});
    }

    void Window::touchEnd() {
        // Nothing stays under the mouse once the finger is gone
        handleMouseExit(coordinate(_touchX), coordinate(_touchY), 0, Mod{});
        _touchPhase = TouchPhase::None;
    }

    bool Window::panBy(const float dx, const float dy) {
        auto target = _panTarget.lock();
        if (!target) {
            return true;
        }
        _panRemainderX += dx;
        _panRemainderY += dy;
        const float stepX = std::trunc(_panRemainderX);
        const float stepY = std::trunc(_panRemainderY);
        _panRemainderX -= stepX;
        _panRemainderY -= stepY;
        if (stepX == 0 && stepY == 0) {
            return false;
        }
        // The content follows the finger
        return !target->scrollBy(static_cast<int>(stepX), static_cast<int>(stepY));
    }

    void Window::handleTouchMove(const float x, const float y, const GestureTime time) {
        if (_touchPhase == TouchPhase::None || _touchPhase == TouchPhase::Ignored) {
            return;
        }
        updateGestures(time); // A long press may be due first

        const float dx = x - _touchX;
        const float dy = y - _touchY;
        const float dt = seconds(time - _touchTime);
        _touchX    = x;
        _touchY    = y;
        _touchTime = time;

        switch (_touchPhase) {
            case TouchPhase::Pending: {
                const float fromStartX = x - _touchStartX;
                const float fromStartY = y - _touchStartY;
                if (std::hypot(fromStartX, fromStartY) < touchSlop) {
                    break;
                }
                const bool horizontal = std::abs(fromStartX) > std::abs(fromStartY);
                if (Div *target = scrollTargetAt(coordinate(_touchStartX), coordinate(_touchStartY), horizontal)) {
                    _touchPhase    = TouchPhase::Panning;
                    _panTarget     = target->weak_from_this();
                    _panVelocityX  = 0;
                    _panVelocityY  = 0;
                    _panRemainderX = 0;
                    _panRemainderY = 0;
                    panBy(fromStartX, fromStartY);
                } else {
                    touchPressButton(MouseButton::LEFT);
                    handleMouseMove(coordinate(x), coordinate(y), _touchButton, Mod{}, false);
                }
                break;
            }

            case TouchPhase::Pressing:
                handleMouseMove(coordinate(x), coordinate(y), _touchButton, Mod{}, false);
                break;

            case TouchPhase::Panning:
                panBy(dx, dy);
                if (dt > 0) {
                    // Mostly the latest movement's speed, smoothed a little
                    _panVelocityX = 0.6f * dx / dt + 0.4f * _panVelocityX;
                    _panVelocityY = 0.6f * dy / dt + 0.4f * _panVelocityY;
                }
                break;

            default:
                break;
        }
    }

    void Window::handleTouchRelease(const float x, const float y, const GestureTime time) {
        if (_touchPhase == TouchPhase::None) {
            return;
        }
        if (x != _touchX || y != _touchY) {
            handleTouchMove(x, y, time);
        }
        updateGestures(time); // A long press may be due first

        switch (_touchPhase) {
            case TouchPhase::Pending:
                // A tap: a click where the finger landed
                touchPressButton(MouseButton::LEFT);
                handleMouseButtonChange(coordinate(_touchStartX), coordinate(_touchStartY), MouseButton::LEFT, false, Mod{});
                break;

            case TouchPhase::Pressing:
                handleMouseButtonChange(coordinate(_touchX), coordinate(_touchY), _touchButton, false, Mod{});
                break;

            case TouchPhase::Panning:
                if (time - _touchTime <= flingRestDelay
                    && std::hypot(_panVelocityX, _panVelocityY) >= flingMinVelocity) {
                    _flinging  = true;
                    _flingTime = time;
                }
                break;

            default:
                break;
        }
        touchEnd();
    }

    void Window::handleTouchCancel(const GestureTime /*time*/) {
        if (_touchPhase == TouchPhase::Pressing) {
            // Released, without a click
            Div::handleMouseRelease(coordinate(_touchX), coordinate(_touchY), _touchButton, Mod{});
        }
        if (_touchPhase != TouchPhase::None) {
            touchEnd();
        }
    }

    void Window::updateGestures(const GestureTime now) {
        if (_touchPhase == TouchPhase::Pending && now - _touchStart >= longPressDelay) {
            // A long press: the context press, a right button press, released when the finger lifts
            touchPressButton(MouseButton::RIGHT);
        }

        if (!_flinging) {
            return;
        }
        const float dt = seconds(now - _flingTime);
        if (dt <= 0) {
            return;
        }
        _flingTime = now;
        // The velocity decays exponentially, v(t) = v0 * flingDecay^(t in ms), so the distance is its integral
        const float rate     = -std::log(flingDecay) * 1000.0f; // Per second
        const float kept     = std::exp(-rate * dt);
        const float distance = (1.0f - kept) / rate;
        const bool  blocked  = panBy(_panVelocityX * distance, _panVelocityY * distance);
        _panVelocityX *= kept;
        _panVelocityY *= kept;
        if (blocked || std::hypot(_panVelocityX, _panVelocityY) < flingStopVelocity) {
            _flinging = false;
        }
    }

    // endregion


    // region Focus

    void Window::requestFocus(Div *component, const bool visible) {
        if (visible != _focusVisible) {
            _focusVisible = visible;
            for (Div *div: _focusPath) {
                div->setFocused(false); // Restyled as it is focused again below: `:focus-visible` changed
            }
        }

        // Build the path
        std::vector<Div *> path{};
        Div                *c = component;
        while (c != nullptr) {
            path.insert(path.begin(), c);
            c = c->parent();
        }

        // Compare with current and unfocus
        for (Div *div: _focusPath) {
            auto it = std::find(path.cbegin(), path.cend(), div);
            if (it == path.cend()) {
                div->setFocused(false);
                div->blurred.emit();
            }
        }

        // Set new focus path
        _focusPath = std::move(path);
        for (Div *div: _focusPath) {
            div->setFocused(true);
            div->focused.emit();
        }
    }

    // endregion

    // region MouseEvents

    namespace {
        /**
         * Whether a div handles presses itself
         */
        bool takesPresses(const Div &div) {
            return div.mouseEnabled()
                   && (div.mouseButtonChanged.hasSubscriptions() || div.mousePressed.hasSubscriptions()
                       || div.mouseReleased.hasSubscriptions() || div.clicked.hasSubscriptions());
        }

        struct SlopSearch {
            int   x;
            int   y;
            Div   *target{nullptr};
            float distance{0};
            // The target's visible part (window coordinates), where the press moves to
            int   left{0};
            int   top{0};
            int   right{0};
            int   bottom{0};
        };

        /**
         * Find the div with a hit slop reaching the point whose center is nearest, among `div` (whose parent's
         * content starts at originX, originY in window coordinates) and its descendants, within the clip of the
         * divs that clip their children
         */
        void findSlopTarget(Div *div, int originX, int originY, int clipLeft, int clipTop, int clipRight, int clipBottom, SlopSearch &search) {
            if (!div->visible()) {
                return;
            }
            const int left   = originX + div->x();
            const int top    = originY + div->y();
            const int right  = left + div->getWidth();
            const int bottom = top + div->getHeight();

            const float slop = div->computedStyle().hitSlop();
            if (slop > 0 && div->mouseEnabled() && div->enabled()) {
                const int visibleLeft   = std::max(left, clipLeft);
                const int visibleTop    = std::max(top, clipTop);
                const int visibleRight  = std::min(right, clipRight);
                const int visibleBottom = std::min(bottom, clipBottom);
                const auto reach = static_cast<int>(std::ceil(slop));
                if (visibleLeft < visibleRight && visibleTop < visibleBottom
                    && search.x >= std::max(left - reach, clipLeft) && search.x < std::min(right + reach, clipRight)
                    && search.y >= std::max(top - reach, clipTop) && search.y < std::min(bottom + reach, clipBottom)) {
                    const float distance = std::hypot(
                        static_cast<float>(search.x) - static_cast<float>(left + right) / 2.0f,
                        static_cast<float>(search.y) - static_cast<float>(top + bottom) / 2.0f
                    );
                    // On equal distances, the one on top (later in the tree)
                    if (!search.target || distance <= search.distance) {
                        search.target   = div;
                        search.distance = distance;
                        search.left     = visibleLeft;
                        search.top      = visibleTop;
                        search.right    = visibleRight;
                        search.bottom   = visibleBottom;
                    }
                }
            }

            if (div->mouseChildren()) {
                if (div->computedStyle().overflow() != Overflow::Visible) {
                    clipLeft   = std::max(clipLeft, left);
                    clipTop    = std::max(clipTop, top);
                    clipRight  = std::min(clipRight, right);
                    clipBottom = std::min(clipBottom, bottom);
                }
                for (const auto &child : div->children()) {
                    findSlopTarget(child.get(), left + div->scrollX(), top + div->scrollY(), clipLeft, clipTop, clipRight, clipBottom, search);
                }
            }
        }
    }

    Div *Window::hitSlopTarget(const int x, const int y, int &targetX, int &targetY) {
        // What the press lands on keeps it, if anything there takes presses
        std::vector<Div *> path{};
        hitPath(this, x, y, path);
        if (std::any_of(path.begin(), path.end(), [](const Div *div) { return takesPresses(*div); })) {
            return nullptr;
        }

        SlopSearch search{x, y};
        constexpr int unbounded = std::numeric_limits<int>::max() / 2;
        findSlopTarget(this, 0, 0, -unbounded, -unbounded, unbounded, unbounded, search);
        if (!search.target) {
            return nullptr;
        }
        targetX = std::clamp(x, search.left, search.right - 1);
        targetY = std::clamp(y, search.top, search.bottom - 1);
        return search.target;
    }

    MouseEventStatus Window::handleMouseButtonChange(int mouseX, int mouseY, MouseButton button, bool down, Mod modifiers) {
        // Through hit slop, the press and its release land on the target's edge
        if (down) {
            int   targetX = mouseX;
            int   targetY = mouseY;
            Div *target   = hitSlopTarget(mouseX, mouseY, targetX, targetY);
            _slopTarget   = target ? target->weak_from_this() : std::weak_ptr<Div>{};
            mouseX        = targetX;
            mouseY        = targetY;
        } else if (auto target = _slopTarget.lock()) {
            _slopTarget = {};
            int targetX = mouseX;
            int targetY = mouseY;
            if (hitSlopTarget(mouseX, mouseY, targetX, targetY) == target.get()) {
                mouseX = targetX;
                mouseY = targetY;
            }
        }

        auto res = Div::handleMouseButtonChange(mouseX, mouseY, button, down, modifiers);

        if (down) {
            handleMousePress(mouseX, mouseY, button, modifiers);
        } else {
            handleClick(mouseX, mouseY, button, modifiers);

            if (button == MouseButton::LEFT) {
                auto   now   = std::chrono::high_resolution_clock::now();
                double delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastClick).count();
                if (delta <= 500) {
                    ++_clickCount;
                    handleDoubleClick(mouseX, mouseY, _clickCount, modifiers);
                } else {
                    _clickCount = 1;
                }
                _lastClick = now;
            }

            handleMouseRelease(mouseX, mouseY, button, modifiers);
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

    void Window::setTextInputArea(const int x, const int y, const int width, const int height, const int cursor) {
        if (_systemWindow) {
            _systemWindow->setTextInputArea(x, y, width, height, cursor);
        }
    }

    namespace {
        /**
         * The divs that take the focus, in document order
         */
        void focusables(Div *div, std::vector<Div *> &list) {
            if (!div->enabled() || !div->visible() || div->computedStyle().display() == Display::None) {
                return;
            }
            if (div->focusEnabled()) {
                list.push_back(div);
            }
            for (auto &child: div->children()) {
                focusables(child.get(), list);
            }
        }
    }

    bool Window::focusNext(const bool backwards) {
        std::vector<Div *> list{};
        // The open menu first, then the modals, otherwise the app
        if (menu->visible() && !menu->children().empty()) {
            focusables(menu, list);
        } else if (!modal->children().empty()) {
            focusables(modal, list);
        } else {
            focusables(app, list);
        }
        if (list.empty()) {
            return false;
        }
        Div *current = nullptr;
        for (auto it = _focusPath.rbegin(); it != _focusPath.rend(); ++it) {
            if ((*it)->focusEnabled()) {
                current = *it;
                break;
            }
        }
        auto       found = std::find(list.begin(), list.end(), current);
        const auto count = static_cast<std::ptrdiff_t>(list.size());
        std::ptrdiff_t index;
        if (found == list.end()) {
            index = backwards ? count - 1 : 0;
        } else {
            index = (std::distance(list.begin(), found) + (backwards ? count - 1 : 1)) % count;
        }
        requestFocus(list[static_cast<std::size_t>(index)], true);
        return true;
    }

    bool Window::handleKeyPress(Key key, Mod mod) {
        // Tab moves the focus, before anyone else sees it
        if (key == Key::TAB && !mod.ctrl && !mod.alt && !mod.super) {
            return focusNext(mod.shift);
        }
        // Go backwards since we want to cancel as soon as possible when a child handles it
        for (auto focused = _focusPath.rbegin(); focused != _focusPath.rend(); ++focused) {
            // Everyone in the focus path gets the key events, focusEnabled or not
            Div *div = (*focused);
            if (div->keyPressed.hasSubscriptions()) {
                div->keyPressed.emit(key, mod);
                return true;
            }
        }
        return false;
    }

    bool Window::handleKeyRepeat(Key key, Mod mod) {
        // Go backwards since we want to cancel as soon as possible when a child handles it
        for (auto focused = _focusPath.rbegin(); focused != _focusPath.rend(); ++focused) {
            // Everyone in the focus path gets the key events, focusEnabled or not
            Div *div = (*focused);
            if (div->keyRepeated.hasSubscriptions()) {
                div->keyRepeated.emit(key, mod);
                return true;
            }
        }
        return false;
    }

    bool Window::handleKeyRelease(Key key, Mod mod) {
        // Go backwards since we want to cancel as soon as possible when a child handles it
        for (auto focused = _focusPath.rbegin(); focused != _focusPath.rend(); ++focused) {
            // Everyone in the focus path gets the key events, focusEnabled or not
            Div *div = (*focused);
            if (div->keyReleased.hasSubscriptions()) {
                div->keyReleased.emit(key, mod);
                return true;
            }
        }
        return false;
    }

    bool Window::handleType(const icu::UnicodeString &character) {
        // Go backwards since we want to cancel as soon as possible when a child handles it
        for (auto focused = _focusPath.rbegin(); focused != _focusPath.rend(); ++focused) {
            // Only the focusEnabled divs get the character events
            Div *div = (*focused);
            if (div->typed.hasSubscriptions()) {
                div->typed.emit(character);
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
