#include <psychicui/psychicui.hpp>
#include <psychicui/Window.hpp>
#include <psychicui/Button.hpp>
#include <memory>
#include <stdexcept>
#include <iostream>

using namespace psychicui;

class TitleBar: public Widget {
public:
    TitleBar() :
        Widget() {
        setHeight(style()->titleBarHeight);
        }
};

class DemoApplication : public Window {
public:
    std::shared_ptr<Button> button2;

    DemoApplication() :
        Window("Demo Application") {
        //_decorated = false;

        auto titleBar = std::make_shared<TitleBar>();
        addChild(titleBar);

        auto button = std::make_shared<Button>("Test");
        button->setSize(Vector2i(20, 20));
        addChild(button);

        button2 = std::make_shared<Button>("Coucou");
        button2->setPosition(Vector2i(40, 40));
        button2->setSize(Vector2i(40, 40));
        addChild(button2);
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