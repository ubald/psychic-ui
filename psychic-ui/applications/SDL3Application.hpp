#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <vector>
#include "SDL3SystemWindow.hpp"

namespace psychic_ui {

    /**
     * Runs Psychic UI windows on SDL3: sets up SDL's video, gives each opened `Window` an SDL window with
     * an OpenGL context, and runs the event loop until every window is closed.
     *
     *     psychic_ui::SDL3Application app;
     *     app.open(std::make_shared<MyWindow>());
     *     app.run();
     *
     * To put Psychic UI in an SDL window you create yourself, use `SDL3SystemWindow` directly.
     */
    class SDL3Application {
    public:
        SDL3Application();
        ~SDL3Application();

        SDL3Application(const SDL3Application &) = delete;
        SDL3Application &operator=(const SDL3Application &) = delete;
        SDL3Application(SDL3Application &&) = delete;
        SDL3Application &operator=(SDL3Application &&) = delete;

        /**
         * Creates the window's SDL window and GL context, from the window's title, size and flags
         */
        SDL3SystemWindow &open(std::shared_ptr<Window> window);
        void close(const std::shared_ptr<Window> &window);

        /**
         * Handles events and draws the windows, until none is visible or SDL asks to quit. A window is drawn after an
         * event for it, and while it needs frames (`Window::untilNextFrame`: a style or layout change, an animation, a
         * requested frame, a timer); when none does, the loop sleeps in SDL until an event or the soonest timer.
         */
        void run();

    private:
        /**
         * An SDL window and its context, which the runner owns, with the adapter that shows the `Window` in it
         */
        struct OwnedWindow {
            SDL_Window                        *sdlWindow{nullptr};
            SDL_GLContext                     glContext{nullptr};
            std::unique_ptr<SDL3SystemWindow> systemWindow{};
            bool                              dirty{true}; // An event came for it (or it just opened): draw it

            OwnedWindow() = default;
            OwnedWindow(const OwnedWindow &) = delete;
            OwnedWindow &operator=(const OwnedWindow &) = delete;
            OwnedWindow(OwnedWindow &&) = delete;
            OwnedWindow &operator=(OwnedWindow &&) = delete;
            ~OwnedWindow();
        };

        std::vector<std::unique_ptr<OwnedWindow>> _windows{};

        OwnedWindow *find(SDL_Window *sdlWindow);

        /**
         * Hands an event to its window, which will be drawn
         */
        void dispatch(const SDL_Event &event);

        /**
         * Draws the visible windows that need it
         * @return Whether a window is visible
         */
        bool renderAll();

        /**
         * How long the loop may sleep: zero when a window needs a frame, nullopt until an event
         */
        [[nodiscard]] std::optional<std::chrono::nanoseconds> untilNextFrame() const;

        /**
         * Keeps drawing while the platform blocks the loop in a live resize (Windows, macOS)
         */
        static bool SDLCALL liveResize(void *userdata, SDL_Event *event);
    };
}
