#include <algorithm>
#include <stdexcept>
#include <string>
#include "SDL3Application.hpp"
#include "../Window.hpp"

namespace psychic_ui {

    static std::runtime_error sdlError(const std::string &what) {
        return std::runtime_error(what + ": " + SDL_GetError());
    }

    SDL3Application::SDL3Application() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw sdlError("Could not initialize SDL");
        }
        SDL_AddEventWatch(liveResize, this);
    }

    SDL3Application::~SDL3Application() {
        SDL_RemoveEventWatch(liveResize, this);
        _windows.clear();
        SDL_Quit();
    }

    SDL3Application::OwnedWindow::~OwnedWindow() {
        systemWindow.reset(); // Frees Skia's objects with the context current
        if (glContext) {
            SDL_GL_DestroyContext(glContext);
        }
        if (sdlWindow) {
            SDL_DestroyWindow(sdlWindow);
        }
    }

    SDL3SystemWindow &SDL3Application::open(std::shared_ptr<Window> window) {
        SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (!window->getVisible()) {
            flags |= SDL_WINDOW_HIDDEN;
        }
        if (window->getResizable()) {
            flags |= SDL_WINDOW_RESIZABLE;
        }
        if (!window->getDecorated()) {
            flags |= SDL_WINDOW_BORDERLESS;
        }
        if (window->getFullscreen()) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        // OpenGL 3.3 core, double buffered with a stencil buffer for Skia. No alpha, sRGB or
        // "accelerated visual" requirement: X servers without them (Xvfb) have no matching visual.
        SDL_GL_ResetAttributes();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        #if defined(__APPLE__)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
        #endif
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        auto owned = std::make_unique<OwnedWindow>();
        owned->sdlWindow = SDL_CreateWindow(
            window->getTitle().c_str(),
            window->windowWidth(),
            window->windowHeight(),
            flags
        );
        if (!owned->sdlWindow) {
            throw sdlError("Could not create an SDL window");
        }

        owned->glContext = SDL_GL_CreateContext(owned->sdlWindow);
        if (!owned->glContext) {
            throw sdlError("Could not create an OpenGL context");
        }

        // Vsync, adaptive where the driver has it
        if (!SDL_GL_SetSwapInterval(-1)) {
            SDL_GL_SetSwapInterval(1);
        }

        owned->systemWindow = std::make_unique<SDL3SystemWindow>(window, owned->sdlWindow, owned->glContext);
        // Nothing else draws with this context
        owned->systemWindow->_sharesContext = false;
        owned->systemWindow->installHitTest();

        auto &systemWindow = *owned->systemWindow;
        _windows.push_back(std::move(owned));
        return systemWindow;
    }

    void SDL3Application::close(const std::shared_ptr<Window> &window) {
        std::erase_if(_windows, [&window](const auto &owned) { return owned->systemWindow->window() == window; });
    }

    SDL3Application::OwnedWindow *SDL3Application::find(SDL_Window *sdlWindow) {
        auto it = std::find_if(_windows.begin(), _windows.end(), [sdlWindow](const auto &owned) { return owned->sdlWindow == sdlWindow; });
        return it == _windows.end() ? nullptr : it->get();
    }

    bool SDL3Application::renderAll() {
        bool any = false;
        for (auto &owned : _windows) {
            if (owned->systemWindow->render()) {
                SDL_GL_SwapWindow(owned->sdlWindow);
                any = true;
            }
        }
        return any;
    }

    void SDL3Application::run() {
        while (true) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    // Closing the last window or an interrupt: the windows that agree close, which ends the loop
                    for (auto &owned : _windows) {
                        if (owned->systemWindow->window()->windowShouldClose()) {
                            owned->systemWindow->window()->close();
                        }
                    }
                    continue;
                }
                if (auto owned = find(SDL_GetWindowFromEvent(&event))) {
                    owned->systemWindow->handleEvent(event);
                }
            }

            if (!renderAll()) {
                break;
            }
        }
    }

    bool SDLCALL SDL3Application::liveResize(void *userdata, SDL_Event *event) {
        // Exposed while live resizing (data1 = 1): the loop is blocked, so draw from here
        if (event->type == SDL_EVENT_WINDOW_EXPOSED && event->window.data1 == 1) {
            auto app = static_cast<SDL3Application *>(userdata);
            if (auto owned = app->find(SDL_GetWindowFromEvent(event))) {
                // The resize events are still queued: read the size now
                owned->systemWindow->readSize();
                owned->systemWindow->_window->windowResized(owned->systemWindow->getWidth(), owned->systemWindow->getHeight());
                if (owned->systemWindow->render()) {
                    SDL_GL_SwapWindow(owned->sdlWindow);
                }
            }
        }
        return true;
    }
}
