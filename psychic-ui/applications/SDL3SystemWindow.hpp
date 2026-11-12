#pragma once

#include <memory>
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include "../SystemWindow.hpp"

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
     * For custom title bars (`Window::addDragArea`) and the resize borders of an undecorated window,
     * also call `adapter.installHitTest()`, which replaces the window's SDL hit test.
     *
     * The adapter never creates, shows, destroys or polls anything of the host's: destroy it before
     * the SDL window and the GL context.
     */
    class SDL3SystemWindow : public SystemWindow {
    public:
        /**
         * @param glContext The window's context, made current to draw; `nullptr` when the host makes
         *                  a context current itself before `render()`
         */
        SDL3SystemWindow(std::shared_ptr<Window> window, SDL_Window *sdlWindow, SDL_GLContext glContext);
        ~SDL3SystemWindow() override;

        SDL_Window *sdlWindow() const;

        /**
         * Translates an SDL event into the window's events
         * @return Whether the event was for this window
         */
        bool handleEvent(const SDL_Event &event);

        bool render() override;
        void *glProcAddress(const char *name) const override;

        void setTitle(const std::string &title) override;
        void setFullscreen(bool fullscreen) override;
        bool getMinimized() const override;
        void setMinimized(bool minimized) override;
        bool getMaximized() const override;
        void setMaximized(bool maximized) override;
        void setVisible(bool visible) override;
        void setCursor(Cursor cursor) override;
        void startDrag() override;
        void stopDrag() override;
        bool supportsDrag() const override;
        void setSize(int width, int height) override;
        void setPosition(int x, int y) override;

        void startTextInput() override;
        void stopTextInput() override;

        std::string getClipboardText() const override;
        void setClipboardText(const std::string &text) override;

        /**
         * Lets SDL ask `hitTest` where presses go, so the window manager or compositor moves the window
         * from its drag areas and resizes it from the borders of an undecorated window, on Wayland too
         * @return false when the platform has no hit testing; `startDrag()` then moves the window itself
         */
        bool installHitTest();

        /**
         * What a point of the window (window coordinates) is for: moving the window on the `Window`'s
         * drag areas, resizing it on the borders of an undecorated resizable window, otherwise normal input
         */
        SDL_HitTestResult hitTest(int x, int y);

        static Mod mapMods(SDL_Keymod mods);
        static Key mapKey(SDL_Keycode keycode);

    protected:
        friend class SDL3Application;

        SDL_Window    *_sdlWindow{nullptr};
        SDL_GLContext _glContext{nullptr};

        /**
         * System cursors, indexed by `Cursor`
         */
        SDL_Cursor *_cursors[6]{};

        /**
         * Where the window was grabbed, relative to its position, while dragging it ourselves
         */
        float _dragOffsetX{0};
        float _dragOffsetY{0};

        bool _hitTestInstalled{false};

        /**
         * Files of the drop in progress, dispatched together when it completes
         */
        std::vector<std::string> _droppedFiles{};

        void readSize();
    };
}
