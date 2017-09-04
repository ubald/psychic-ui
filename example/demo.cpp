#include <iostream>
#include <psychicui/psychicui.hpp>
#include "demo.hpp"
#include "components/Application.hpp"

using namespace psychicui;

int main(int /* argc */, char ** /* argv */) {
    try {
        psychicui::init();

        {
            std::shared_ptr<Application> app = std::make_shared<Application>();
            app->open();
            psychicui::mainloop();
        }

        psychicui::shutdown();
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