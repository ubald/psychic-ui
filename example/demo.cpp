#include <psychicui/psychicui.hpp>
#include <psychicui/Window.hpp>
#include <psychicui/Button.hpp>
#include <iostream>
#include "demo-stylesheet.hpp"

using namespace psychicui;

class TitleBar: public Component {
public:
    TitleBar() :
        Component() {

        }
};

class DemoApplication : public Window {
public:
    DemoApplication() :
        Window("Demo Application") {
        loadStyleSheet<DemoStyleSheet>();

        addChild(std::make_shared<Label>("Label"));
        addChild(std::make_shared<Button>("Button"));
    }
};

int main(int /* argc */, char ** /* argv */) {
    try {
        psychicui::init();

        {
            std::shared_ptr<DemoApplication> app = std::make_shared<DemoApplication>();
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