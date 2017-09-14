#include <iostream>
#include <psychic-ui/psychic-ui.hpp>
#include "demo.hpp"
#include "components/Application.hpp"

using namespace psychic_ui;

int main(int /* argc */, char ** /* argv */) {
    try {
        psychic_ui::init();

        {
            std::shared_ptr<Application> app = std::make_shared<Application>();
            app->open();
            psychic_ui::mainloop();
        }

        psychic_ui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
        MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
        std::cerr << error_msg << std::endl;
        #endif
        return -1;
    }

    return 0;
}
