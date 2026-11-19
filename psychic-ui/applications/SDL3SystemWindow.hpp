#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include "../SystemWindow.hpp"
#include "../WindowScale.hpp"

namespace psychic_ui {

    /**
     * Shows a Psychic UI `Window` in an SDL3 window that has an OpenGL context.
     *
     * `SDL3Application` makes one for each window it opens. A host application with its own SDL window,
     * GL context and loop embeds Psychic UI the same way:
     *
     *     auto ui = std::make_shared<MyWindow>();
     *     psychic_ui::SDL3SystemWindow adapter(ui, sdlWindow, glContext);
     *
     *     while (running) {
     *         SDL_Event event;
     *         while (SDL_PollEvent(&event)) {
     *             adapter.handleEvent(event); // false when the event isn't for this window
     *         }
     *         adapter.render();               // makes the context current and draws
     *         SDL_GL_SwapWindow(sdlWindow);
     *     }
     *
     * A host that waits for events while nothing changes asks the window when to draw again
     * (`Window::untilNextFrame`), as `SDL3Application::run` does.
     *
     * For custom title bars (`Window::addDragArea`) and the resize borders of an undecorated window,
     * also call `adapter.installHitTest()`, which replaces the window's SDL hit test.
     *
     * The adapter never creates, shows, destroys or polls anything of the host's: destroy it before
     * the SDL window and the GL context.
     *
     * Sizes and input are in logical units: SDL's window units scaled by the display scale they don't
     * already carry (`WindowScale`). On macOS and Wayland, with `SDL_WINDOW_HIGH_PIXEL_DENSITY`, they are
     * SDL's window units; on Windows and X11, where those are pixels, they are pixels divided by the user's
     * scale. A host that creates its window on Windows makes it as many pixels as `readWindowScale` says
     * its logical size is (`SDL3Application::open` does). Positions (`setPosition`, `windowMoved`) stay
     * in SDL's desktop coordinates.
     */
    class SDL3SystemWindow : public SystemWindow {
    public:
        /**
         * @param glContext The window's context, made current to draw; `nullptr` when the host makes
         *                  a context current itself before `render()`
         */
        SDL3SystemWindow(std::shared_ptr<Window> window, SDL_Window *sdlWindow, SDL_GLContext glContext);
        ~SDL3SystemWindow() override;
        SDL3SystemWindow(const SDL3SystemWindow &) = delete;
        SDL3SystemWindow &operator=(const SDL3SystemWindow &) = delete;
        SDL3SystemWindow(SDL3SystemWindow &&) = delete;
        SDL3SystemWindow &operator=(SDL3SystemWindow &&) = delete;

        [[nodiscard]] SDL_Window *sdlWindow() const;

        /**
         * Translates an SDL event into the window's events
         * @return Whether the event was for this window
         */
        bool handleEvent(const SDL_Event &event);

        bool render() override;
        GLProc glProcAddress(const char *name) const override;

        void setTitle(const std::string &title) override;
        void setFullscreen(bool fullscreen) override;
        [[nodiscard]] bool getMinimized() const override;
        void setMinimized(bool minimized) override;
        [[nodiscard]] bool getMaximized() const override;
        void setMaximized(bool maximized) override;
        void setVisible(bool visible) override;
        void setCursor(Cursor cursor) override;
        void startDrag() override;
        void stopDrag() override;
        [[nodiscard]] bool supportsDrag() const override;
        void setSize(int width, int height) override;
        void setPosition(int x, int y) override;

        void startTextInput() override;
        void stopTextInput() override;
        void setTextInputArea(int x, int y, int width, int height, int cursor) override;

        /**
         * SDL's clock, which times its events
         */
        [[nodiscard]] std::chrono::nanoseconds gestureTime() const override;

        /**
         * Pushes an event of `wakeEventType()`, which ends an `SDL_WaitEvent`: a host that waits for events
         * draws when it gets one (the adapter ignores it)
         */
        void wake() override;

        /**
         * The event type `wake` pushes, registered with SDL on first use
         */
        static Uint32 wakeEventType();

        /**
         * SDL_GetSystemTheme. `handleEvent` passes SDL_EVENT_SYSTEM_THEME_CHANGED on to the window, whatever
         * window the event names (it names none). Virtual so that tests can make up a system.
         */
        [[nodiscard]] ColorScheme colorScheme() const override;

        [[nodiscard]] std::string getClipboardText() const override;
        void setClipboardText(const std::string &text) override;

        /**
         * Lets SDL ask `hitTest` where presses go, so the window manager or compositor moves the window
         * from its drag areas and resizes it from the borders of an undecorated window, on Wayland too
         * @return false when the platform has no hit testing; `startDrag()` then moves the window itself
         */
        bool installHitTest();

        /**
         * What a point of the window (SDL's window coordinates, as SDL asks) is for: moving the window on the `Window`'s
         * drag areas, resizing it on the borders of an undecorated resizable window, otherwise normal input
         */
        SDL_HitTestResult hitTest(int windowX, int windowY);

        /**
         * How SDL's window units map to logical units, from the window's pixel density and display scale
         */
        [[nodiscard]] static WindowScale readWindowScale(SDL_Window *sdlWindow);

        static Mod mapMods(SDL_Keymod mods);
        static Key mapKey(SDL_Keycode keycode);

    protected:
        friend class SDL3Application;

        SDL_Window    *_sdlWindow{nullptr};
        SDL_GLContext _glContext{nullptr};

        /**
         * System cursors, indexed by `Cursor`
         */
        std::array<SDL_Cursor *, 6> _cursors{};

        /**
         * Where the window was grabbed, relative to its position, while dragging it ourselves
         */
        float _dragOffsetX{0};
        float _dragOffsetY{0};

        bool _hitTestInstalled{false};

        /**
         * SDL's window units to logical units, read with the size
         */
        WindowScale _scale{};

        /**
         * `readWindowScale` of the window: read again when it is resized or its display scale changes.
         * Virtual so that tests can make up a display scale.
         */
        [[nodiscard]] virtual WindowScale windowScale() const;

        /**
         * The finger the window follows (the first one down on a touch screen), while one is down
         */
        bool         _fingerDown{false};
        SDL_FingerID _finger{0};

        /**
         * Buttons held with the pen, mouse style (the tip is the left button)
         */
        unsigned _penButtons{0};

        /**
         * What kind of touch device sent finger events: only touch screens (direct) are followed,
         * trackpads also move the mouse. Virtual so that tests can make up touch screens.
         */
        [[nodiscard]] virtual SDL_TouchDeviceType touchDeviceType(SDL_TouchID touchID) const;

        void handleFingerEvent(const SDL_TouchFingerEvent &finger);
        void handlePenButton(MouseButton button, bool down, float x, float y);

        /**
         * Files of the drop in progress, dispatched together when it completes
         */
        std::vector<std::string> _droppedFiles{};

        void readSize();
    };
}
