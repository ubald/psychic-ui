#include <iostream>
#include "demo.hpp"
#include "example/demo/DemoWindow.hpp"
#include <psychic-ui/applications/SDL3Application.hpp>

using namespace psychic_ui;

int main(int /* argc */, char ** /* argv */) {
    try {
        SDL3Application app;
        app.open(std::make_shared<DemoWindow>());
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "Caught a fatal error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
