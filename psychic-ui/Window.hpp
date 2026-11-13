#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <unordered_map>
#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "Div.hpp"
#include "Modal.hpp"
#include "style/StyleManager.hpp"
#include "style/Style.hpp"
#include "components/Menu.hpp"
#include "signals/Signal.hpp"
#include "SystemWindow.hpp"

namespace psychic_ui {

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

        void openMenu(const std::vector<std::shared_ptr<MenuItem>> &items, int x, int y);
        void closeMenu();

        /**
         * Where the application's content goes, filling the window
         */
        Div &appContainer() const {
            return *app;
        };

        Modal &modalContainer() const {
            return *modal;
        }

        Modal &menuContainer() const {
            return *menu;
        }

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
         * Use a skin set for the style manager, with its default theme: every component hatches a new skin
         */
        template<class S, class Self>
        Self &skinSet(this Self &self) {
            static_assert(std::is_base_of_v<SkinSet, S>, "S must extend SkinSet");
            static_cast<Window &>(self).updateSkinSet(std::make_unique<S>());
            return self;
        }

        /**
         * The style manager's theme
         */
        const Theme &theme() const { return styleManager()->theme(); }

        /**
         * Switch the style manager's theme: its stylesheets load again with the new values
         */
        template<class Self>
        Self &theme(this Self &self, const Theme &theme) {
            static_cast<Window &>(self).updateTheme(theme);
            return self;
        }

        // endregion

        // region Focus

        void requestFocus(Div *component);

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

        MouseEventStatus handleMouseButtonChange(int mouseX, int mouseY, MouseButton button, bool down, Mod modifiers) override;
        std::chrono::time_point<std::chrono::high_resolution_clock> _lastClick{};
        unsigned int _clickCount{0};

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

        bool handleKeyPress(Key key, Mod mod);
        bool handleKeyRepeat(Key key, Mod mod);
        bool handleKeyRelease(Key key, Mod mod);
        bool handleType(const icu::UnicodeString &character);

        // endregion


    protected:

        void updateTheme(const Theme &theme);
        void updateSkinSet(std::unique_ptr<SkinSet> skinSet);

        // region Default DIVs

        // Children of the window, which owns them
        Div   *app{nullptr};
        Modal *modal{nullptr};
        Modal *menu{nullptr};

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
        Div *scrollTargetAt(int x, int y, bool horizontal);

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

