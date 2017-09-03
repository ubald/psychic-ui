#include <iostream>
#include <psychicui/Label.hpp>
#include <psychicui/Button.hpp>
#include <psychicui/components/Tabs.hpp>
#include <psychicui/components/TabbedContainer.hpp>
#include <psychicui/themes/default.hpp>
#include "Application.hpp"
#include "MenuBar.hpp"
#include "ToolBar.hpp"
#include "Labels.hpp"
#include "Buttons.hpp"
#include "../demo-stylesheet.hpp"

namespace psychicui {

    Application::Application() :
        Window("Demo Application") {
        loadStyleSheet<PsychicUIStyleSheet>();
        loadStyleSheet<DemoStyleSheet>();

        add(std::make_shared<MenuBar>());
        add(std::make_shared<ToolBar>());

        std::vector<std::pair<std::string, std::shared_ptr<Hatcher>>> panels{
            {"Labels",  std::make_shared<Hatcher>([]() { return std::make_shared<Labels>(); })},
            {"Buttons", std::make_shared<Hatcher>([]() { return std::make_shared<Buttons>(); })}
        };

        add<TabbedContainer<std::pair<std::string, std::shared_ptr<Hatcher>>>>(
            panels,
            [](const auto &item) {
                return item.second->hatch();
            },
            [](const auto &item) -> std::string {
                return item.first;
            }
        )->select(panels[1]);
    }
}