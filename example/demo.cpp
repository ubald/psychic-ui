#include <psychicui/psychicui.hpp>
#include <psychicui/Screen.hpp>
#include <psychicui/Button.hpp>
#include <memory>
#include <stdexcept>
#include <iostream>

using namespace psychicui;

class DemoApplication : public Screen {
public:
    DemoApplication() :
        Screen("Demo Application") {
        auto button = new Button(nullptr, "Test");
        button->setSize(Vector2i(20, 20));
        addChild(button);
    }
};

int main(int /* argc */, char ** /* argv */) {
    try {
        psychicui::init();

        {
            std::unique_ptr<DemoApplication> app = std::make_unique<DemoApplication>();
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