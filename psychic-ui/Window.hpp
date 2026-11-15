#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <optional>
#include <unordered_map>
#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "Div.hpp"
#include "Overlay.hpp"
#include "style/StyleManager.hpp"
#include "style/Style.hpp"
#include "components/Menu.hpp"
#include "signals/Signal.hpp"
#include "SystemWindow.hpp"

namespace psychic_ui {

    class Toast;
    struct ToastOptions;
    class ToastStack;
    class Tooltip;

    class Window : public Div {
    public:
        explicit Window(std::string title);
        virtual ~Window();

        Window *window() override;

        std::string getTitle() const;
        void setTitle(const std::string &title);

        bool getResizable() const;
        void setResizable(bool resizable);

        bool getDecorated() const;
        void setDecorated(bool decorated);

        void toggleMinimized();
        bool minimized() const;
        void setMinimized(bool minimized);

        void toggleMaximized();
        bool maximized() const;
        void setMaximized(bool minimized);

        void toggleFullscreen();
        bool getFullscreen() const;
        void setFullscreen(bool fullscreen);

        /**
         * Whether the system window is shown. Not the `visible` style: the window's own visibility, which
         * `close()` turns off and the application follows.
         */
        bool windowVisible() const;
        void setWindowVisible(bool value);

        /**
         * Moves the window with the mouse until `stopDrag()`, for a custom title bar. Prefer `addDragArea`,
         * which also works where applications can't place their windows (Wayland).
         */
        void startDrag();
        void stopDrag();

        /**
         * Whether the window can be moved from inside it (custom title bars). False on Wayland when the
         * system window has no way to hand the move to the compositor: use system decorations there.
         */
        bool supportsDrag() const;

        /**
         * Marks a div as a part of the window that moves it when dragged, like a title bar. Pressing on its
         * interactive children (anything with mouse button or click handlers) still reaches them.
         * Held weakly: a div that goes away stops being one.
         */
        void addDragArea(const std::shared_ptr<Div> &div);

        /**
         * Whether a press at this point (window coordinates) should move the window. The system window
         * asks it before the press is dispatched, so that the window manager or compositor moves the window.
         */
        bool isDragArea(int x, int y);

        int windowX() const;
        int windowY() const;
        void setWindowPosition(int x, int y);

        int windowWidth() const;
        int windowHeight() const;
        void setWindowSize(int width, int height);

        /**
         * Attaches the window to the system window that shows it, or detaches it with `nullptr`.
         * Skia is set up on the first frame drawn after, in the system window's GL context.
         */
        void open(SystemWindow *systemWindow);
        SystemWindow *systemWindow() const;
        void close();
        void drawAll();

        /**
         * The frame's work before painting, in order: compute the dirty styles (visible or not),
         * then lay out the tree if needed. Called by `drawAll`, needs no canvas.
         */
        void updateStylesAndLayout();

        /**
         * How many style passes a frame may run when `styleUpdated` callbacks keep invalidating styles
         */
        static constexpr int maxStylePasses = 4;

        // region Popups

        /**
         * Where the application's content goes, filling the window
         */
        Div &appContainer() const {
            return *app;
        };

        /**
         * The layer above the content where popups go (menus, select lists, tooltips, dialogs, toasts): see `Popup`
         */
        Overlay &overlay() const {
            return *_overlay;
        }

        /**
         * Open a menu (closing the open one), at a point in window coordinates: below it and to its right, flipped and
         * shifted to stay inside the window
         */
        Menu &openMenu(const MenuItems &items, int x, int y);

        /**
         * Open a menu beside a div, on a side of it (below by default), lined up with its start
         */
        Menu &openMenu(const MenuItems &items, Div &anchor, Side side = Side::Bottom, PopupAlign align = PopupAlign::Start);

        /**
         * Open a menu made by hand (closing the open one)
         */
        Menu &openMenu(const std::shared_ptr<Menu> &menu);

        /**
         * Open a context menu at a point (window coordinates), like `openMenu` there, with the `context` class.
         * `Div::contextMenu(items)` calls it when a context menu is asked for on the div; `fromKeyboard` (the Menu
         * key, Shift+F10) gives the menu the focus with its first item highlighted.
         */
        Menu &openContextMenu(const MenuItems &items, float x, float y, bool fromKeyboard = false);

        /**
         * Close the open menu, with its submenus
         */
        void closeMenu();

        /**
         * Show a toast in the bottom right corner, over the ones already there (see `ToastOptions`)
         */
        Toast &toast(const ToastOptions &options);

        /**
         * The toasts' stack, made on the first toast
         */
        ToastStack &toasts();

        /**
         * The tooltip showing now, or nullptr
         */
        Tooltip *tooltip() const { return _tooltip.get(); }

        /**
         * How long the mouse rests on a div with a `tooltip` before it shows, and how soon after one hides the next
         * one shows at once (Pivotal's)
         */
        static constexpr std::chrono::nanoseconds tooltipDelay    = std::chrono::milliseconds(300);
        static constexpr std::chrono::nanoseconds tooltipWarmTime = std::chrono::milliseconds(400);

        /**
         * Advance the popups with time: a tooltip's delay, the toasts' timers. `drawAll` calls it every frame with the
         * system window's `gestureTime()`; tests call it with theirs.
         */
        void updatePopups(std::chrono::nanoseconds now);

        // endregion

        void setCursor(Cursor cursor);

        // region Signals

        using MenuOpenedSlot = std::shared_ptr<Slot<>>;
        using MenuClosedSlot = std::shared_ptr<Slot<>>;

        Signal<> menuOpened{};
        Signal<> menuClosed{};

        PSYCHIC_UI_EVENT(onMenuOpen, menuOpened)
        PSYCHIC_UI_EVENT(onMenuClose, menuClosed)

        // endregion

        // region Input Mode

        /**
         * What the user points with, as last seen: `Pointer` until a backend reports a finger on a touch screen
         * (`Touch`, until the next real mouse motion) or a pen (`Pen`). In touch mode the `:touch` pseudo-class
         * matches every div of the window.
         */
        InputMode inputMode() const { return _inputMode; }

        /**
         * Set the input mode, as the backends do: see `handleInputModeChange`
         */
        template<class Self>
        Self &inputMode(this Self &self, InputMode mode) {
            static_cast<Window &>(self).handleInputModeChange(mode);
            return self;
        }

        /**
         * Emitted with the new mode when the input mode changes
         */
        Signal<InputMode> inputModeChanged{};

        PSYCHIC_UI_EVENT(onInputModeChange, inputModeChanged)

        /**
         * Called by the backends with the device each input event came from: a change restyles the window
         * (`:touch`) and emits `inputModeChanged`, the same mode again does nothing
         */
        virtual void handleInputModeChange(InputMode mode);

        // endregion

        // region Touch

        /**
         * When a touch event happened: a monotonic clock's reading (the backend's event timestamps, the
         * system window's `gestureTime()` for frames). Only differences matter.
         */
        using GestureTime = std::chrono::nanoseconds;

        /**
         * How far a finger moves (window units) before a press becomes a drag or a pan
         */
        static constexpr float touchSlop = 8.0f;

        /**
         * How long a finger stays down, within the slop, to raise a context press (a right button press)
         */
        static constexpr GestureTime longPressDelay = std::chrono::milliseconds(500);

        /**
         * Kinetic scrolling: the speed (window units per second) a pan must have when the finger lifts to keep
         * scrolling, the share of its speed it keeps every millisecond, and the speed where it stops
         */
        static constexpr float flingMinVelocity  = 50.0f;
        static constexpr float flingDecay        = 0.998f;
        static constexpr float flingStopVelocity = 10.0f;

        /**
         * A pan that rests this long before the finger lifts doesn't fling
         */
        static constexpr GestureTime flingRestDelay = std::chrono::milliseconds(100);

        /**
         * The primary finger of a direct touch screen, from the backends (window coordinates), which turns
         * the input mode to touch. The window makes mouse events of it: a tap clicks where the finger
         * landed; past `touchSlop` it pans the scrollable div under the finger when it can scroll along the
         * drag's main axis (with kinetic scrolling after), otherwise it presses where the finger landed and
         * drags; held `longPressDelay` without moving is a right button press, released when the finger lifts.
         * A finger landing while kinetic scrolling runs only stops it.
         */
        void handleTouchPress(float x, float y, GestureTime time);
        void handleTouchMove(float x, float y, GestureTime time);
        void handleTouchRelease(float x, float y, GestureTime time);

        /**
         * The system took the touch away: a press in progress is released without a click, no fling
         */
        void handleTouchCancel(GestureTime time);

        /**
         * Advance what goes on without events: a long press fires, kinetic scrolling moves and slows down.
         * `drawAll` calls it every frame with the system window's `gestureTime()`; tests call it with theirs.
         */
        void updateGestures(GestureTime now);

        /**
         * Whether kinetic scrolling is moving a div
         */
        bool flinging() const { return _flinging; }

        // endregion

        // region Style

        /**
         * Helper to forward the stylesheet loading to the style manager
         * and refresh our children's runtime styles
         * @tparam T :StyleSheet
         */
        template<typename T>
        void loadStyleSheet(bool reset = false) {
            styleManager()->loadStyleSheet<T>(reset);
            updateRuntimeStyles();
        }

        /**
         * Use a skin set for the style manager: every component hatches a new skin. The theme: a theme picked with
         * `theme(t)` stays picked, itself if it is one of the set's (`SkinSet::themes()`), else the set's theme of
         * the same scheme (its light or dark theme); a window following the system keeps following it, with the
         * pair given to `followSystemTheme` if both are the set's, else the set's own (`SkinSet::theme(scheme)`).
         */
        template<class S, class Self>
        Self &skinSet(this Self &self) {
            static_assert(std::is_base_of_v<SkinSet, S>, "S must extend SkinSet");
            static_cast<Window &>(self).updateSkinSet(std::make_unique<S>());
            return self;
        }

        template<class Self>
        Self &skinSet(this Self &self, std::unique_ptr<SkinSet> skinSet) {
            static_cast<Window &>(self).updateSkinSet(std::move(skinSet));
            return self;
        }

        /**
         * The style manager's theme
         */
        const Theme &theme() const { return styleManager()->theme(); }

        /**
         * Switch the style manager's theme: its stylesheets load again with the new values.
         * The window stops following the system's light or dark mode.
         */
        template<class Self>
        Self &theme(this Self &self, const Theme &theme) {
            static_cast<Window &>(self).updateTheme(theme);
            return self;
        }

        /**
         * Follow the system's light or dark mode (what a new window does): the skin set's light theme while the
         * system is light, its default theme (dark) while it is dark or doesn't say; switched live as the system
         * changes (`handleColorSchemeChange`). With a pair, those two themes instead.
         */
        template<class Self>
        Self &followSystemTheme(this Self &self) {
            static_cast<Window &>(self).updateFollowSystemTheme(nullptr, nullptr);
            return self;
        }

        template<class Self>
        Self &followSystemTheme(this Self &self, const Theme &light, const Theme &dark) {
            static_cast<Window &>(self).updateFollowSystemTheme(&light, &dark);
            return self;
        }

        /**
         * Whether the theme follows the system (until `theme(t)` picks one)
         */
        bool followsSystemTheme() const { return _followSystemTheme; }

        /**
         * The system's light or dark mode, as its system window last reported it (`Unknown` before, or when the
         * platform doesn't say)
         */
        ColorScheme colorScheme() const { return _colorScheme; }

        /**
         * The theme following the system picks now
         */
        const Theme &systemTheme() const;

        /**
         * Called by the system window when the system goes light or dark (and when it attaches): a window
         * following the system switches themes, then `colorSchemeChanged` is emitted. The same scheme again
         * does nothing.
         */
        virtual void handleColorSchemeChange(ColorScheme scheme);

        Signal<ColorScheme> colorSchemeChanged{};

        PSYCHIC_UI_EVENT(onColorSchemeChange, colorSchemeChanged)

        // endregion

        // region Focus

        /**
         * The deepest div of the focus path, or nullptr
         */
        Div *focusedDiv() const { return _focusPath.empty() ? nullptr : _focusPath.back(); }

        /**
         * The divs under `div` (it included) that Tab stops at, in document order: focusEnabled, enabled, visible
         * and displayed, their ancestors up to `div` too
         */
        static void focusables(Div *div, std::vector<Div *> &list);

        /**
         * Focus a div (and the path to it). `visible` is whether the focus should show (`:focus-visible`):
         * true when it moved with the keyboard, false for a press.
         */
        void requestFocus(Div *component, bool visible = false);

        /**
         * Whether the focus moved with the keyboard, so that the focused div shows a focus ring (`:focus-visible`)
         */
        bool focusVisible() const { return _focusVisible; }

        /**
         * Move the focus to the next div that takes it (focusEnabled, enabled, visible), in document order,
         * or the previous one with `backwards`; wraps around. Tab and Shift+Tab do it. While a popup that traps the
         * focus is open (a menu, a dialog), only among its divs.
         * @return Whether a div took the focus
         */
        bool focusNext(bool backwards = false);

        // endregion

        // region Window Delegate

        virtual void windowMoved(int x, int y);
        virtual void windowResized(int width, int height);
        virtual void windowActivated();
        virtual void windowDeactivated();
        virtual void windowMinimized();
        virtual void windowRestored();
        virtual bool windowShouldClose();


        virtual bool dropEvent(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }

        // endregion

        // region Mouse

        // The backends call these with the event in window coordinates. The window fills in `windowX` and
        // `windowY`, `buttons` (from the presses and releases it saw) and, on a press, `clickCount` when the
        // backend left it 0. A press goes through mouseButtonChanged, mousePressed, then contextMenuRequested for
        // the right button (unless `contextMenuOnRelease`); a release through mouseButtonChanged, clicked (left
        // button), doubleClicked (from the second press in a row), mouseReleased, then contextMenuRequested for the
        // right button with `contextMenuOnRelease`.

        /**
         * Also follows the div under the mouse with a `tooltip`, to show it after `tooltipDelay`
         */
        MouseEventStatus handleMouseMove(const MouseEvent &event, bool handled) override;
        MouseEventStatus handleMouseScroll(const MouseEvent &event) override;

        /**
         * A press first closes the popups it is outside of (`Overlay::pressOutside`); one that closed a modal popup
         * goes no further, and neither does its release.
         * A press on nothing that takes presses goes to the nearest div whose `hitSlop` reaches it, as if it
         * landed on its edge, and so does its release while still within that slop
         */
        MouseEventStatus handleMouseButtonChange(const MouseEvent &event) override;

        /**
         * Whether the right button asks for a context menu on its release, as on Windows (the default there), rather
         * than on its press, as on macOS and Linux (the default elsewhere). A touch long press always asks on the
         * press (it is held), the keyboard when its key goes down.
         */
        /**
         * Whether a left press with Control (and no other modifier) is a right press, for the whole press and its
         * release (the right button, a context menu, no click), as Cocoa applications do on macOS (the default
         * there; AppKit delivers it as a left press with Control, and SDL leaves it so by default). Not for touches.
         */
        bool ctrlClickIsRightClick() const { return _ctrlClickIsRightClick; }

        template<class Self>
        Self &ctrlClickIsRightClick(this Self &self, detail::BoolArg rightClick) {
            static_cast<Window &>(self)._ctrlClickIsRightClick = rightClick.value;
            return self;
        }

        bool contextMenuOnRelease() const { return _contextMenuOnRelease; }

        template<class Self>
        Self &contextMenuOnRelease(this Self &self, detail::BoolArg onRelease) {
            static_cast<Window &>(self)._contextMenuOnRelease = onRelease.value;
            return self;
        }

        /**
         * The div a press here goes to through its `hitSlop`, and the point on it the press moves to; nullptr
         * when the press lands on something that takes presses, or no slop reaches it
         */
        Div *hitSlopTarget(float x, float y, float &targetX, float &targetY);

        /**
         * The deepest div with a tooltip under a point (window coordinates), as a press would find it
         */
        Div *tooltipTargetAt(float x, float y);

        /**
         * Presses of the same button in a row count as one more click when this close in time and space
         */
        static constexpr std::chrono::milliseconds multiClickDelay{500};
        static constexpr float                     multiClickDistance = 4.0f;

        // endregion

        // region Clipboard

        /**
         * The system clipboard's text, in UTF-8
         */
        std::string clipboardText() const;
        void setClipboardText(const std::string &text);

        // endregion

        // region Keyboard

        void startTextInput();
        void stopTextInput();

        /**
         * Where text is being typed, for the system window (`SystemWindow::setTextInputArea`): text fields
         * tell it in touch mode, so that the on-screen keyboard leaves their caret visible
         */
        void setTextInputArea(int x, int y, int width, int height, int cursor);

        bool handleKeyPress(Key key, Mod mod);
        bool handleKeyRepeat(Key key, Mod mod);
        bool handleKeyRelease(Key key, Mod mod);
        bool handleType(const icu::UnicodeString &character);

        // endregion


    protected:

        void updateTheme(const Theme &theme);
        void applyTheme(const Theme &theme);
        void updateSkinSet(std::unique_ptr<SkinSet> skinSet);
        void updateFollowSystemTheme(const Theme *light, const Theme *dark);
        void applySystemTheme();

        // region Default DIVs

        // Children of the window, which owns them
        Div     *app{nullptr};
        Overlay *_overlay{nullptr};

        // endregion

        // region Popups

        std::shared_ptr<ToastStack> _toasts{};
        std::shared_ptr<Tooltip>    _tooltip{};     // Open, or nullptr
        std::weak_ptr<Div>          _tooltipTarget{}; // The div under the mouse with a tooltip
        std::optional<std::chrono::nanoseconds> _tooltipSince{}; // When the mouse came to rest on it (the next frame)
        bool                        _tooltipSuppressed{false}; // Pressed or typed: none until the mouse moves to another
        bool                        _tooltipWarm{false};       // One just hid: the next one shows at once
        std::optional<std::chrono::nanoseconds> _warmSince{};
        bool                        _touchTooltip{false};      // Shown by a long press, until the finger lifts
        #ifdef __APPLE__
        bool                        _ctrlClickIsRightClick{true};
        #else
        bool                        _ctrlClickIsRightClick{false};
        #endif
        bool                        _ctrlClickHeld{false}; // The left button is down as a right one
        #ifdef _WIN32
        bool                        _contextMenuOnRelease{true};
        #else
        bool                        _contextMenuOnRelease{false};
        #endif
        bool                        _pressSwallowed{false};    // A press closed a modal popup: its release goes nowhere
        MouseButton                 _swallowedButton{MouseButton::LEFT};

        void showTooltip(Div &target);
        void hideTooltip(bool warm);

        // endregion

        // region Rendering

        SystemWindow *_systemWindow{nullptr};
        sk_sp<GrDirectContext> _sk_context{nullptr};
        sk_sp<SkSurface>       _sk_surface{nullptr};
        SkCanvas               *_sk_canvas{nullptr};

        // endregion

        // region Window

        std::string _title;
        bool        _windowVisible{true};
        bool        _fullscreen{false};
        bool        _resizable{true};
        bool        _decorated{true};


        // endregion

        // region Mouse

        /**
         * Cursor to use when this component has the mouse focus
         */
        Cursor _cursor{Cursor::Arrow};

        InputMode _inputMode{InputMode::Pointer};

        bool _focusVisible{false};

        ColorScheme          _colorScheme{ColorScheme::Unknown};
        bool                 _followSystemTheme{true};
        std::optional<Theme> _lightTheme{}; // The pair followSystemTheme was given, else the skin set's
        std::optional<Theme> _darkTheme{};

        /**
         * The div the last press went to through its hit slop, for its release
         */
        std::weak_ptr<Div> _slopTarget{};

        // The buttons held, and the last press, for counting clicks
        unsigned                                           _buttons{0};
        std::chrono::time_point<std::chrono::steady_clock> _lastPress{};
        MouseButton                                        _lastPressButton{MouseButton::NONE};
        float                                              _lastPressX{0};
        float                                              _lastPressY{0};
        unsigned                                           _clickCount{0};
        GestureTime                                        _lastTap{}; // On the touch clock

        /**
         * The window's touch gestures as mouse events, from a finger at x, y
         */
        MouseEvent touchEvent(float x, float y, MouseButton button = MouseButton::NONE, bool down = false) const;

        /**
         * The Menu key or Shift+F10: a context menu request on the focused div, at its center
         */
        bool requestContextMenuFromKeyboard();

        // endregion

        // region Touch

        enum class TouchPhase {
            None,     // No finger down
            Pending,  // Down, within the slop: a tap, a long press, a drag or a pan
            Pressing, // Pressing a mouse button (_touchButton): a drag or a long press
            Panning,  // Scrolling _panTarget
            Ignored   // Stopped kinetic scrolling: nothing more until it lifts
        };

        TouchPhase                _touchPhase{TouchPhase::None};
        MouseButton               _touchButton{MouseButton::LEFT};
        float                     _touchStartX{0};
        float                     _touchStartY{0};
        float                     _touchX{0};
        float                     _touchY{0};
        GestureTime               _touchStart{};
        GestureTime               _touchTime{};
        std::weak_ptr<Div>        _panTarget{};
        float                     _panVelocityX{0};
        float                     _panVelocityY{0};
        float                     _panRemainderX{0};
        float                     _panRemainderY{0};
        bool                      _flinging{false};
        GestureTime               _flingTime{};

        void touchPressButton(MouseButton button);
        void touchEnd();
        /**
         * Scroll the pan target, keeping the fractions for the next move
         * @return Whether it couldn't move at all (at the end of its content)
         */
        bool panBy(float dx, float dy);

        /**
         * The deepest div under a point that can scroll along that axis, the way a press would find it
         */
        Div *scrollTargetAt(float x, float y, bool horizontal);

        //int _mouseX{0};
        //int _mouseY{0};

        // endregion

        // region Initialization

        void initSkia();
        void getSkiaSurface();
        void releaseSkia();

        /**
         * The Skia surface no longer matches the framebuffer (it was resized): the next frame makes a new one
         */
        bool _surfaceStale{true};

        // endregion

        // region Focus

        std::vector<Div *> _focusPath{};

        std::vector<std::weak_ptr<Div>> _dragAreas{};

        // endregion

        // region Events

        // endregion

        // Performances
        double fps{0.0f};

        /**
         * Window size, read through `windowWidth()`/`windowHeight()` (the system window's once opened):
         * the size the root is laid out at
         */
        int _windowWidth{0};
        int _windowHeight{0};

    private:
        // Dont allow direct manipulation by others than the window itself
        using Div::add;
        using Div::remove;
        using Div::removeAll;

        // Performance
        std::chrono::time_point<std::chrono::high_resolution_clock> lastReport;
        int                                                         frames = 0;
    };
}

