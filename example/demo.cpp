#include <psychicui/psychicui.hpp>
#include <psychicui/Window.hpp>
#include <psychicui/Button.hpp>
#include <psychicui/StyleManager.hpp>
#include <memory>
#include <stdexcept>
#include <iostream>

using namespace psychicui;

auto customStyle = StyleManager::getStyleDeclaration(".custom");

class TitleBar: public Component {
public:
    TitleBar() :
        Component() {
        setHeight(style()->getValue(titleBarHeight));
        }
};

class DemoApplication : public Window {
public:
    std::shared_ptr<Button> button2;

    DemoApplication() :
        Window("Demo Application") {

        setPadding(240);

//        auto hgroup1 = std::make_shared<Component>();
//        hgroup1->setLayout(Horizontal);
//        hgroup1->setPadding(6);
//        std::cout << hgroup1->flex() << " " << hgroup1->flexShrink() << " " << hgroup1->flexGrow() << std::endl;
//        addChild(hgroup1);
//
//        auto component11 = std::make_shared<Component>();
//        component11->setPadding(24);
//        hgroup1->addChild(component11);
//
//        auto label11 = std::make_shared<Label>("Simple Padded Label");
//        //label11->setPadding(24);
//        component11->addChild(label11);
//
//        auto component12 = std::make_shared<Component>();
//        component12->setPadding(24);
//        component12->setWidth(100);
//        hgroup1->addChild(component12);
//
//        auto label12 = std::make_shared<Label>("Simple padded label that should setWrap beautifully");
//        //label12->setPadding(24);
//        component12->addChild(label12);
//
//        auto hgroup2 = std::make_shared<Component>();
//        hgroup2->setLayout(Horizontal);
//        hgroup2->setPadding(6);
//        addChild(hgroup2);
//
//        auto component21 = std::make_shared<Component>();
//        component21->setPadding(24);
//        component21->setWidth(100);
//        hgroup2->addChild(component21);
//
//        auto component22 = std::make_shared<Component>();
//        component22->setPadding(24);
//        component22->setWidth(100);
//        hgroup2->addChild(component22);
//
//        auto label21 = std::make_shared<Label>("Simple Label");
////        label21->setPosition(10, 10);
//        label21->setSize(100, 200);
//        label21->setPadding(24);
//        addChild(label21);


        auto label23 = std::make_shared<Label>("Simple Padded Label");
//        label23->setPadding(24);
        label23->setClassNames({"some", "label"});
        addChild(label23);

        label23->getComputedStyle();


        auto label22 = std::make_shared<Label>("Simple padded label that should setWrap beautifully");
//        label22->setPadding(24);
        addChild(label22);

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