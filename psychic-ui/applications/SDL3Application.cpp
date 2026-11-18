#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_version.h>
#include <SDL3/SDL_video.h>
#include "SDL3Application.hpp"
#include "../Window.hpp"
#include "SDL3SystemWindow.hpp"

namespace psychic_ui {

    namespace {
        std::runtime_error sdlError(const std::string &what) {
            return std::runtime_error(what + ": " + SDL_GetError());
        }
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
        if (!window->windowVisible()) {
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
        #ifdef __APPLE__
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

        owned->systemWindow = std::make_unique<SDL3SystemWindow>(std::move(window), owned->sdlWindow, owned->glContext);
        // Nothing else draws with this context
        owned->systemWindow->setSharesContext(false);
        const bool hitTest = owned->systemWindow->installHitTest();

        // With SDL_LOGGING=app=debug
        using GetString = const char *(*)(unsigned);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): SDL hands GL procedures out untyped; called through its real type
        const auto glGetString = reinterpret_cast<GetString>(SDL_GL_GetProcAddress("glGetString"));
        constexpr unsigned GL_RENDERER_ = 0x1F01;
        constexpr unsigned GL_VERSION_  = 0x1F02;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): SDL's logging is printf-style, its arguments match the format
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "Psychic UI: SDL %d.%d.%d, %s video driver, OpenGL %s (%s), %d samples, %d stencil bits, %s, pixel density %.2f",
            SDL_VERSIONNUM_MAJOR(SDL_GetVersion()), SDL_VERSIONNUM_MINOR(SDL_GetVersion()), SDL_VERSIONNUM_MICRO(SDL_GetVersion()),
            SDL_GetCurrentVideoDriver(),
            glGetString ? glGetString(GL_VERSION_) : "?",
            glGetString ? glGetString(GL_RENDERER_) : "?",
            owned->systemWindow->getSamples(),
            owned->systemWindow->getStencilBits(),
            hitTest ? "hit testing" : "no hit testing",
            owned->systemWindow->getPixelRatio()
        );

        auto &systemWindow = *owned->systemWindow;
        _windows.push_back(std::move(owned));
        return systemWindow;
    }

    void SDL3Application::close(const std::shared_ptr<Window> &window) {
        std::erase_if(_windows, [&window](const auto &owned) { return owned->systemWindow->window() == window; });
    }

    SDL3Application::OwnedWindow *SDL3Application::find(SDL_Window *sdlWindow) {
        auto it = std::ranges::find_if(_windows, [sdlWindow](const auto &owned) { return owned->sdlWindow == sdlWindow; });
        return it == _windows.end() ? nullptr : it->get();
    }

    bool SDL3Application::renderAll() {
        bool                           visible = false;
        const std::chrono::nanoseconds now{SDL_GetTicksNS()};
        for (auto &owned : _windows) {
            const auto &window = owned->systemWindow->window();
            if (!window->windowVisible()) {
                continue;
            }
            visible = true;
            // After input, or when the window has something new (a style, a layout, an animation, a timer)
            const auto wait = window->untilNextFrame(now);
            if (owned->dirty || (wait && *wait <= std::chrono::nanoseconds::zero())) {
                owned->dirty = false;
                if (owned->systemWindow->render()) {
                    SDL_GL_SwapWindow(owned->sdlWindow);
                }
            }
        }
        return visible;
    }

    std::optional<std::chrono::nanoseconds> SDL3Application::untilNextFrame() const {
        const std::chrono::nanoseconds          now{SDL_GetTicksNS()};
        std::optional<std::chrono::nanoseconds> soonest{};
        for (const auto &owned : _windows) {
            const auto &window = owned->systemWindow->window();
            if (!window->windowVisible()) {
                continue;
            }
            if (owned->dirty) {
                return std::chrono::nanoseconds::zero();
            }
            const auto wait = window->untilNextFrame(now);
            if (wait && (!soonest || *wait < *soonest)) {
                soonest = wait;
            }
        }
        return soonest;
    }

    void SDL3Application::dispatch(const SDL_Event &event) {
        if (event.type == SDL_EVENT_QUIT) {
            // Closing the last window or an interrupt: the windows that agree close, which ends the loop
            for (auto &owned : _windows) {
                if (owned->systemWindow->window()->windowShouldClose()) {
                    owned->systemWindow->window()->close();
                }
            }
            return;
        }
        if (event.type == SDL_EVENT_SYSTEM_THEME_CHANGED) {
            // The system went light or dark: every window follows
            for (auto &owned: _windows) {
                owned->systemWindow->handleEvent(event);
                owned->dirty = true;
            }
            return;
        }
        if (event.type == SDL3SystemWindow::wakeEventType()) {
            return; // A window asked for a frame: it says so when asked (untilNextFrame)
        }
        if (auto *owned = find(SDL_GetWindowFromEvent(&event))) {
            owned->systemWindow->handleEvent(event);
            owned->dirty = true; // Input may change anything: draw after it
        }
    }

    void SDL3Application::run() {
        while (renderAll()) {
            SDL_Event event;
            // Nothing to draw: sleep until an event comes, or the soonest timer (a tooltip's delay, a toast's timeout)
            const auto wait = untilNextFrame();
            if (!wait) {
                if (SDL_WaitEvent(&event)) {
                    dispatch(event);
                }
            } else if (*wait > std::chrono::nanoseconds::zero()) {
                // Rounded up: waking before the timer would only wait again
                const auto ms = std::chrono::ceil<std::chrono::milliseconds>(*wait).count();
                if (SDL_WaitEventTimeout(&event, static_cast<Sint32>(std::min<long long>(ms, INT32_MAX)))) {
                    dispatch(event);
                }
            }
            while (SDL_PollEvent(&event)) {
                dispatch(event);
            }
        }
    }

    bool SDLCALL SDL3Application::liveResize(void *userdata, SDL_Event *event) {
        // Exposed while live resizing (data1 = 1): the loop is blocked, so draw from here
        if (event->type == SDL_EVENT_WINDOW_EXPOSED && event->window.data1 == 1) {
            auto *app = static_cast<SDL3Application *>(userdata);
            if (auto *owned = app->find(SDL_GetWindowFromEvent(event))) {
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
