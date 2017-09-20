#ifdef WITH_SDL2

#pragma once

#include <SDL.h>
#include "../Window.hpp"
#include "../Application.hpp"

#if defined(SK_BUILD_FOR_ANDROID)
#include <GLES/gl.h>
#elif defined(SK_BUILD_FOR_UNIX)
#include <GL/gl.h>
#elif defined(SK_BUILD_FOR_MAC)
#include <OpenGL/gl.h>
#elif defined(SK_BUILD_FOR_IOS)
#include <OpenGLES/ES2/gl.h>
#endif

namespace psychic_ui {
    class SDL2SystemWindow;

    class SDL2Application : public Application {
        friend class SDL2SystemWindow;

    public:
        void init() override;
        void mainloop() override;
        void open(std::shared_ptr<Window> window) override;
        void close(std::shared_ptr<Window> window) override;
        void shutdown() override;
    protected:
        static std::unordered_map<unsigned int, std::unique_ptr<SDL2SystemWindow>> sdl2Windows;

        bool running{false};
    };

    class SDL2SystemWindow : public SystemWindow {
        friend class SDL2Application;

    public:
        SDL2SystemWindow(SDL2Application *application, std::shared_ptr<Window> window);
        ~SDL2SystemWindow();
        SDL_Window *sdl2Window() const;
        bool render() override;
    protected:
        SDL2Application *_sdl2Application{nullptr};
        SDL_Window      *_sdl2Window{nullptr};
        SDL_GLContext   _sdl2GlContext{nullptr};

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
    };
}

#endif
