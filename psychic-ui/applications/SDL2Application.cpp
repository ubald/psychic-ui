#ifdef WITH_SDL2

#include "SDL2Application.hpp"

namespace psychic_ui {

    /**
    * Log an SDL error with some error message to the output stream of our choice
    * @param os The output stream to write the message to
    * @param msg The error message to write, format will be msg error: SDL_GetError()
    */
    void logSDLError(std::ostream &os, const std::string &msg) {
        os << msg << " error: " << SDL_GetError() << std::endl;
    }
    // APPLICATION

    std::unordered_map<unsigned int, std::unique_ptr<SDL2SystemWindow>> SDL2Application::sdl2Windows{};

    void SDL2Application::init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            logSDLError(std::cout, "SDL_Init");
            throw std::runtime_error("Could not initialize SDL2!");
        }
    }

    void SDL2Application::mainloop() {
        if (running) {
            throw std::runtime_error("Main loop is already running!");
        }

        running = true;

        while (running) {
            SDL_Event e{};
            //while (SDL_PollEvent(&e) != 0) {
            //    if (e.type == SDL_QUIT) {
            //        quit = true;
            //    }
            //    if (e.type == SDL_KEYDOWN) {
            //        quit = true;
            //    }
            //    if (e.type == SDL_MOUSEBUTTONDOWN) {
            //        quit = true;
            //    }
            //}

            int       numScreens = 0;
            for (auto &kv : sdl2Windows) {
                if (kv.second->render()) {
                    numScreens++;
                }
            }

            // Cleanup dirty managers
            for (auto &kv : sdl2Windows) {
                kv.second->window()->styleManager()->setValid();
            }

            if (numScreens == 0) {
                running = false;
                break;
            }
        }
    }

    void SDL2Application::open(std::shared_ptr<Window> window) {
        auto systemWindow = std::make_unique<SDL2SystemWindow>(this, window);
        sdl2Windows[SDL_GetWindowID(systemWindow->_sdl2Window)] = std::move(systemWindow);
    }

    void SDL2Application::close(std::shared_ptr<Window> window) {
    }

    void SDL2Application::shutdown() {
        SDL_Quit();
    }

    // WINDOW

    SDL2SystemWindow::SDL2SystemWindow(SDL2Application *application, std::shared_ptr<Window> window) :
        SystemWindow(application, window), _sdl2Application(application) {

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, _stencilBits);

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

        // If you want multisampling, uncomment the below lines and set a sample count
        static const int kMsaaSampleCount = 0; //4;
        // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, kMsaaSampleCount);


        _sdl2Window = SDL_CreateWindow(
            window->getTitle().c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            _width,
            _height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE //| SDL_WINDOW_BORDERLESS
        );

        if (!_sdl2Window) {
            logSDLError(std::cout, "SDL_CreateWindow");
            SDL_Quit();
            throw std::runtime_error("Could not create SDL2 window!");
        }

        SDL_SetWindowData(_sdl2Window, "psychic", window.get());

        _sdl2GlContext = SDL_GL_CreateContext(_sdl2Window);
        if (!_sdl2GlContext) {
            logSDLError(std::cout, "SDL_GL_CreateContext");
            SDL_Quit();
            throw std::runtime_error("Could not create SDL2 GL context!");
        }

        int success = SDL_GL_MakeCurrent(_sdl2Window, _sdl2GlContext);
        if (success != 0) {
            logSDLError(std::cout, "SDL_GL_CreateContext");
            SDL_Quit();
            throw std::runtime_error("Could not make SDL2 GL context current!");
        }

        window->open(this);
    }

    SDL2SystemWindow::~SDL2SystemWindow() {
        if (_sdl2GlContext) {
            SDL_GL_DeleteContext(_sdl2GlContext);
        }
        if (_sdl2Window) {
            SDL_DestroyWindow(_sdl2Window);
        }
    }

    bool SDL2SystemWindow::render() {
        if (!_window->getVisible()) {
            return false;
        }

        glViewport(0, 0, _width, _height);
        glClearColor(1, 1, 1, 1);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        _window->drawAll();

        SDL_GL_SwapWindow(_sdl2Window);
    }

    SDL_Window *SDL2SystemWindow::sdl2Window() const {
        return _sdl2Window;
    }

    void SDL2SystemWindow::setSize(int width, int height) {
        _width  = width;
        _height = height;
    }

    void SDL2SystemWindow::setTitle(const std::string &title) {

    }

    void SDL2SystemWindow::setFullscreen(bool fullscreen) {

    }

    bool SDL2SystemWindow::getMinimized() const {
        return _minimized;
    }

    void SDL2SystemWindow::setMinimized(bool minimized) {
        if (_minimized != minimized) {
            _minimized = minimized;
        }
    }

    bool SDL2SystemWindow::getMaximized() const {
        return _maximized;
    }

    void SDL2SystemWindow::setMaximized(bool maximized) {
        if (_maximized != maximized) {
            _maximized = maximized;
        }
    }

    void SDL2SystemWindow::setVisible(bool visible) {

    }

    void SDL2SystemWindow::setCursor(int cursor) {

    }

    void SDL2SystemWindow::startDrag() {

    }

    void SDL2SystemWindow::stopDrag() {

    }

}

#endif
