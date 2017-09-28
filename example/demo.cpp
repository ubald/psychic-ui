#include <iostream>
#include "demo.hpp"
#include "example/components/DemoWindow.hpp"

#ifdef USE_GLFW
#include <psychic-ui/applications/GLFWApplication.hpp>
using AppImpl = GLFWApplication;
#endif

#ifdef USE_SDL2
#include <psychic-ui/applications/SDL2Application.hpp>
using AppImpl = SDL2Application;
#endif

using namespace psychic_ui;

int main(int /* argc */, char ** /* argv */) {
    try {
        auto app = std::make_unique<AppImpl>();
        app->init();
        app->open(std::make_shared<DemoWindow>());
        app->mainloop();
        app->shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return -1;
    }

    return 0;
}
