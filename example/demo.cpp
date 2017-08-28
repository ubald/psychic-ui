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

        setPadding(240);

//        auto hgroup1 = std::make_shared<Widget>();
//        hgroup1->setLayout(Horizontal);
//        hgroup1->setPadding(6);
//        std::cout << hgroup1->flex() << " " << hgroup1->flexShrink() << " " << hgroup1->flexGrow() << std::endl;
//        addChild(hgroup1);
//
//        auto widget11 = std::make_shared<Widget>();
//        widget11->setPadding(24);
//        hgroup1->addChild(widget11);
//
//        auto label11 = std::make_shared<Label>("Simple Padded Label");
//        //label11->setPadding(24);
//        widget11->addChild(label11);
//
//        auto widget12 = std::make_shared<Widget>();
//        widget12->setPadding(24);
//        widget12->setWidth(100);
//        hgroup1->addChild(widget12);
//
//        auto label12 = std::make_shared<Label>("Simple padded label that should setWrap beautifully");
//        //label12->setPadding(24);
//        widget12->addChild(label12);
//
//        auto hgroup2 = std::make_shared<Widget>();
//        hgroup2->setLayout(Horizontal);
//        hgroup2->setPadding(6);
//        addChild(hgroup2);
//
//        auto widget21 = std::make_shared<Widget>();
//        widget21->setPadding(24);
//        widget21->setWidth(100);
//        hgroup2->addChild(widget21);
//
//        auto widget22 = std::make_shared<Widget>();
//        widget22->setPadding(24);
//        widget22->setWidth(100);
//        hgroup2->addChild(widget22);
//
//        auto label21 = std::make_shared<Label>("Simple Label");
////        label21->setPosition(10, 10);
//        label21->setSize(100, 200);
//        label21->setPadding(24);
//        addChild(label21);


        auto label23 = std::make_shared<Label>("Simple Padded Label");
//        label23->setPadding(24);
        addChild(label23);

        auto label22 = std::make_shared<Label>("Simple padded label that should setWrap beautifully");
//        label22->setPadding(24);
        addChild(label22);

    }
};

int main(int /* argc */, char ** argv) {
    std::cout << argv[0] << std::endl;
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