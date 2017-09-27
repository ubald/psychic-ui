#ifdef WITH_SDL2

#pragma once

#include <SDL.h>
#include "../Window.hpp"
#include "../Application.hpp"

#if defined(ANDROID)
#include <GLES/gl.h>
#elif defined(UNIX)
#include <GL/gl.h>
#elif defined(APPLE)
#include <OpenGL/gl.h>
#elif defined(IOS)
#include <OpenGLES/ES2/gl.h>
#endif

namespace psychic_ui {
    class SDL2SystemWindow;

    class SDL2Application : public Application {
        friend class SDL2SystemWindow;

    public:
        static std::unordered_map<unsigned int, std::unique_ptr<SDL2SystemWindow>> sdl2Windows;
        void init() override;
        void mainloop() override;
        void open(std::shared_ptr<Window> window) override;
        void close(std::shared_ptr<Window> window) override;
        void shutdown() override;
    protected:
        bool running{false};
        void sdl2PollEvents();
    };

    class SDL2SystemWindow : public SystemWindow {
        friend class SDL2Application;

    public:
        SDL2SystemWindow(SDL2Application *application, std::shared_ptr<Window> window);
        ~SDL2SystemWindow();
        SDL_Window *sdl2Window() const;
        void handleEvent(const SDL_Event &e);
        void liveResize(int width, int height);
        bool render() override;
    protected:
        SDL2Application *_sdl2Application{nullptr};
        SDL_Window      *_sdl2Window{nullptr};
        SDL_GLContext   _sdl2GlContext{nullptr};

        /**
         * Mapping of internal cursors enum to glfw int cursor
         */
        SDL_Cursor *_cursors[6];

        void setTitle(const std::string &title) override;
        void setFullscreen(bool fullscreen) override;
        bool getMinimized() const override;
        void setMinimized(bool minimized) override;
        bool getMaximized() const override;
        void setMaximized(bool maximized) override;
        void setVisible(bool visible) override;
        void setCursor(int cursor) override;
        void startDrag() override;
        void stopDrag() override;
        void setSize(int width, int height) override;
        void setPosition(int x, int y) override;

        static Key mapKey(int keycode);
    };
}

#endif
