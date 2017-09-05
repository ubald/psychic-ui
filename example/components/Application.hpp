#pragma once

#include <psychicui/Window.hpp>
#include <iostream>
#include <psychicui/components/Label.hpp>
#include <psychicui/components/Button.hpp>
#include <psychicui/components/Tabs.hpp>
#include <psychicui/components/TabContainer.hpp>
#include <psychicui/themes/default.hpp>
#include <psychicui/components/TitleBar.hpp>
#include "MenuBar.hpp"
#include "ToolBar.hpp"
#include "Labels.hpp"
#include "Buttons.hpp"
#include "Ranges.hpp"
#include "../demo-stylesheet.hpp"

namespace psychicui {
    class Application : public Window {
    public:
        Application();
    };

    Application::Application() :
        Window("Demo Application") {
        _decorated = false;

        loadStyleSheet<PsychicUIStyleSheet>();
        loadStyleSheet<DemoStyleSheet>();

        add<TitleBar>();

        add<MenuBar>();
        add<ToolBar>();

        std::vector<std::pair<std::string, std::shared_ptr<Hatcher<std::shared_ptr<Component>>>>> panels{};
        panels.emplace_back(std::make_pair("Labels",  Hatcher<std::shared_ptr<Component>>::make([]() { return std::make_shared<Labels>();  })));
        panels.emplace_back(std::make_pair("Buttons", Hatcher<std::shared_ptr<Component>>::make([]() { return std::make_shared<Buttons>(); })));
        panels.emplace_back(std::make_pair("Ranges", Hatcher<std::shared_ptr<Component>>::make([]() { return std::make_shared<Ranges>(); })));


        add<TabContainer<std::pair<std::string, std::shared_ptr<Hatcher<std::shared_ptr<Component>>>>>>(
            panels,
            [](const auto &item) {
                return item.second->hatch();
            },
            [](const auto &item) -> std::string {
                return item.first;
            }
        )
            ->select(panels[2])
            ->style()
            ->set(heightPercent, 1.f);
    }
}