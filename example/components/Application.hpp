#pragma once

#include <psychicui/Window.hpp>
#include <iostream>
#include <psychicui/components/Label.hpp>
#include <psychicui/components/Button.hpp>
#include <psychicui/components/Tabs.hpp>
#include <psychicui/components/TabContainer.hpp>
#include <psychicui/themes/default.hpp>
#include <psychicui/components/TitleBar.hpp>
#include "psychicui/components/MenuBar.hpp"
#include "ToolBar.hpp"
#include "Labels.hpp"
#include "Buttons.hpp"
#include "Divs.hpp"
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

        app->add<TitleBar>();

        std::vector<std::shared_ptr<MenuItem>> mainMenuContents{
            Menu::item(
                "File", {
                    Menu::item("New", []() {}),
                    Menu::item("Open", []() {}),
                    Menu::item("Save", []() {}),
                    Menu::item("Save As...", []() {}),
                    Menu::item("Quit", []() {}),
                }
            ),
            Menu::item("Edit", {
                    Menu::item("Copy", []() {}),
                    Menu::item("Cut", []() {}),
                    Menu::item("Paste", []() {}),
                }),
            Menu::item("View", []() {}),
            Menu::item("Window", []() {}),
            Menu::item("Help", []() {})
        };

        app->add<MenuBar>(mainMenuContents);


        app->add<ToolBar>();

        std::vector<std::pair<std::string, std::shared_ptr<Hatcher<std::shared_ptr<Div>>>>> panels{};
        panels.emplace_back(
            std::make_pair(
                "Divs",
                Hatcher<std::shared_ptr<Div>>::make([]() { return std::make_shared<Divs>(); })));
        panels.emplace_back(
            std::make_pair(
                "Labels",
                Hatcher<std::shared_ptr<Div>>::make([]() { return std::make_shared<Labels>(); })));
        panels.emplace_back(
            std::make_pair(
                "Buttons",
                Hatcher<std::shared_ptr<Div>>::make([]() { return std::make_shared<Buttons>(); })));
        panels.emplace_back(
            std::make_pair(
                "Ranges",
                Hatcher<std::shared_ptr<Div>>::make([]() { return std::make_shared<Ranges>(); })));


        app->add<TabContainer<std::pair<std::string, std::shared_ptr<Hatcher<std::shared_ptr<Div>>>>>>(
            panels,
            [](const auto &item) -> std::string {
                return item.first;
            },
            [](const auto &item) {
                return item.second->hatch();
            }
        )
            ->select(panels[2])
            ->style()
            ->set(heightPercent, 1.f);
    }
}