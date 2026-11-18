#include <iostream>
#include "playground.hpp"
#include <psychic-ui/applications/SDL3Application.hpp>

int main(int /* argc */, char ** /* argv */) {
    try {
        psychic_ui::SDL3Application app;
        app.open(std::make_shared<psychic_ui_playground::PlaygroundWindow>());
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "Caught a fatal error: " << e.what() << '\n';
        return -1;
    }

    return 0;
}
