#include <iostream>
#include <psychicui/Label.hpp>
#include <psychicui/Button.hpp>
#include <psychicui/components/TabbedContainer.hpp>
#include <psychicui/themes/default.hpp>
#include "Application.hpp"
#include "MenuBar.hpp"
#include "ToolBar.hpp"
#include "../demo-stylesheet.hpp"

namespace psychicui {

    Application::Application() :
            Window("Demo Application") {
            loadStyleSheet<PsychicUIStyleSheet>();
            loadStyleSheet<DemoStyleSheet>();

            add(std::make_shared<MenuBar>());
            add(std::make_shared<ToolBar>());


            add(std::make_shared<TabbedContainer<int>>(
                std::vector<int>({1}),
                [](const auto &tab) -> std::shared_ptr<Component> {
//                    std::cout << tab << std::endl;
                    return std::make_shared<Component>();
                }
            ));

            add(std::make_shared<Label>("Label"));
            add(std::make_shared<Button>("Button"));
            add(std::make_shared<Button>("Button"));
            add(std::make_shared<Button>("Button"));
            add(std::make_shared<Button>("Button"));
            add(std::make_shared<Button>("Button"));
        }
}