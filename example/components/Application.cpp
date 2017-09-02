#include <iostream>
#include <psychicui/Label.hpp>
#include <psychicui/Button.hpp>
#include <psychicui/components/Tabs.hpp>
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

        add(
            std::make_shared<Tabs<>>(
                std::vector<std::string>({"Tab 1", "Tab 2", "Tab 3"})
            ));

        std::vector<std::pair<std::string, std::string>> panels{
            {"test1", "component<Button>"},
            {"test2", "component<Component>"}
        };

        add(
            std::make_shared<TabbedContainer<std::pair<std::string, std::string>>>(
                panels,
                [](const auto &item) {
//                    return item.second();
                    return std::make_shared<Component>();
                },
                [](const auto &item) -> std::string {
                    return "coucou";
                }
            ));

        add(
            std::make_shared<TabbedContainer<int>>(
                std::vector<int>({1}),
                [](const auto &tab) {
                    std::cout << tab << std::endl;
                    return std::make_shared<Component>();
                },
                [](const auto &tab) {
                    std::cout << "ICI" << std::endl;
                    return std::to_string(tab);
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