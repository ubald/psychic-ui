#pragma once

#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/AppShell.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/MenuBar.hpp>
#include <psychic-ui/components/Spacer.hpp>
#include <psychic-ui/components/ToolBar.hpp>
#include <psychic-ui/skins/default/DefaultSkins.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/themes/psychic.hpp>
#include "Typography.hpp"
#include "TextInputs.hpp"
#include "Buttons.hpp"
#include "Divs.hpp"
#include "Scrollers.hpp"
#include "Ranges.hpp"
#include "Colors.hpp"
#include "Icons.hpp"
#include "Lists.hpp"
#include "Inputs.hpp"
#include "../demo-stylesheet.hpp"

namespace psychic_ui {
    /**
     * The demo: an AppShell whose sidebar lists the demo's pages, each shown in the inset page panel under its title
     * bar (with the menus) and a tool bar
     */
    class DemoWindow : public Window {
    public:
        DemoWindow();

        /**
         * Show a page by its name ("Buttons"); false when there is none
         */
        bool showPage(const std::string &name);

        /**
         * Use a theme by its name: "system" (follow the system), "pivotal-dark", "pivotal-light", "one-dark",
         * "one-light" or "psychic"; false for another name
         */
        bool useTheme(const std::string &name);

        /**
         * Show or hide the frame rate counter
         */
        void showFps(bool show);

    protected:
        void render(SkCanvas *canvas) override;

    private:
        struct Page {
            std::string                          name;
            std::string                          icon;
            std::function<std::shared_ptr<Div>()> make;
            SidebarItem                          *item{nullptr};
        };

        struct NamedTheme {
            std::string  key;
            std::string  label;
            const Theme *theme; // nullptr: follow the system
        };

        std::vector<Page>       _pages{};
        std::vector<NamedTheme> _themes{};
        AppShell                *_shell{nullptr};
        PageHeader              *_header{nullptr};
        Div                     *_content{nullptr};
        std::shared_ptr<Div>    _panel{};
        SidebarItem             *_themeItem{nullptr};
        Label                   *fpsLabel{nullptr};

        void show(const Page &page);
        void openThemeMenu();
    };

    DemoWindow::DemoWindow() :
        Window("Psychic UI") {
        _decorated = false;

        skinSet<DefaultSkins>();          // Its theme follows the system: Pivotal Light or Pivotal Dark
        loadStyleSheet<DemoStyleSheet>(); // Over the skin set's rules; both load again on a theme change

        _themes = {
            {"system", "System", nullptr},
            {"pivotal-dark", "Pivotal Dark", &themes::pivotalDark},
            {"pivotal-light", "Pivotal Light", &themes::pivotalLight},
            {"one-dark", "One Dark", &themes::oneDark},
            {"one-light", "One Light", &themes::oneLight},
            {"psychic", "Psychic", &themes::psychic},
        };

        _shell = &app->add<AppShell>();
        Sidebar &sidebar = _shell->sidebar()
                                 .title("Psychic UI")
                                 .mark("P");

        // region Pages

        const auto page = [](auto make) {
            return [make]() -> std::shared_ptr<Div> { return make(); };
        };

        _pages = {
            {"Buttons", "square-check", page([] { return std::make_shared<Buttons>(); })},
            {"Ranges", "sliders", page([] { return std::make_shared<Ranges>(); })},
            {"Text Input", "pencil", page([] { return std::make_shared<TextInputs>(); })},
            {"Typography", "file-text", page([] { return std::make_shared<TypographyPanel>(); })},
            {"Divs", "layers", page([] { return std::make_shared<Divs>(); })},
            {"Scroller", "arrow-up-down", page([] { return std::make_shared<Scrollers>(); })},
            {"Colors", "eyedropper", page([] { return std::make_shared<Colors>(); })},
            {"Icons", "star", page([] { return std::make_shared<IconsPanel>(); })},
            {"Inputs", "settings", page([] { return std::make_shared<Inputs>(); })}, // In Controls, after Text Input
            {"Lists", "list", page([] { return std::make_shared<Lists>(); })},
        };

        auto &controls = sidebar.section("Controls");
        auto &layout   = sidebar.section("Layout");
        auto &assets   = sidebar.section("Assets");
        assets.grow(1); // Pushes the last section to the bottom
        for (Page &p: _pages) {
            SidebarSection &section = p.name == "Divs" || p.name == "Scroller" || p.name == "Typography"
                                      ? layout
                                      : p.name == "Colors" || p.name == "Icons" ? assets : controls;
            p.item = &section.item(p.name, p.icon);
        }
        _pages[7].item->count(std::to_string(IconRegistry::shared().names().size()));

        auto &bottom = sidebar.section();
        _themeItem = &bottom.item("Theme", "sun");
        _themeItem->count("System")
                  .selectable(false); // Opens a menu
        _themeItem->onClick([this]() { openThemeMenu(); });

        sidebar.onChange(
            [this](SidebarItem *item) {
                for (const Page &p: _pages) {
                    if (p.item == item) {
                        show(p);
                        return;
                    }
                }
            }
        );

        // endregion

        // region Page panel

        Div &panel = _shell->page();

        std::vector<std::shared_ptr<MenuItem>> menus{
            Menu::item(
                "File", {
                    Menu::item("New", []() {}, "Ctrl+N", "plus"),
                    Menu::item("Open", []() {}, "Ctrl+O", "folder-open"),
                    Menu::item(
                        "Open Recent", {
                            Menu::item("My Resume3.docx", []() {}, "", "file"),
                            Menu::item("My Resume3.1-Final.docx", []() {}, "", "file"),
                            Menu::item("My Resume-FINAL-REAL.docx", []() {}, "", "file"),
                        }, "", "history"
                    ),
                    Menu::item("Save", []() {}, "Ctrl+S", "save"),
                    Menu::item("Save As…", []() {}, "Ctrl+Shift+S"),
                    Menu::item("Quit", [this]() { close(); }, "Ctrl+Q", "sign-out"),
                }
            ),
            Menu::item(
                "Edit", {
                    Menu::item("Copy", []() {}, "Ctrl+C", "copy"),
                    Menu::item("Cut", []() {}, "Ctrl+X"),
                    Menu::item("Paste", []() {}, "Ctrl+V", "clipboard"),
                }
            ),
            Menu::item(
                "View", {
                    Menu::item(
                        "Theme", {
                            Menu::item("System", [this]() { useTheme("system"); }, "", "monitor"),
                            Menu::item("Pivotal Dark", [this]() { useTheme("pivotal-dark"); }, "", "moon"),
                            Menu::item("Pivotal Light", [this]() { useTheme("pivotal-light"); }, "", "sun"),
                            Menu::item("One Dark", [this]() { useTheme("one-dark"); }),
                            Menu::item("One Light", [this]() { useTheme("one-light"); }),
                            Menu::item("Psychic", [this]() { useTheme("psychic"); })
                        }, "", "eye"
                    ),
                    Menu::item(
                        "Panels", {
                            Menu::item("Show Inspector", []() {}, "", "sidebar"),
                            Menu::item("Show Status Bar", []() {}, "", "layout")
                        }
                    ),
                }
            ),
            Menu::item("Help", []() {})
        };

        _header = &panel.add<PageHeader>();
        _header->add<MenuBar>(menus);

        auto &toolBar = panel.add<ToolBar>();
        toolBar.add<Button>("New").icon("plus");
        toolBar.add<Button>("Start").icon("play");
        toolBar.add<Button>("Stop").icon("pause");
        toolBar.add<Spacer>();
        #ifdef DEBUG_LAYOUT
        toolBar.add<Button>("Debug Layout")
               .toggle(true)
               .onChange([](bool selected) { Div::debugLayout = selected; });
        #endif
        toolBar.add<Button>("Help").icon("question");

        // The page's own content takes what is left of the panel, never more: its scrollers end with it
        _content = &panel.add<Div>();
        _content->addClassName("page-content")
                .grow(1)
                .shrink(1)
                .basis(0);

        // endregion

        fpsLabel = &app->add<Label>();
        fpsLabel->fontFamily("stan0755")
                .fontSize(8)
                .textAntiAlias(false)
                .position(Position::Absolute)
                .right(4)
                .bottom(0);

        showPage("Buttons");
    }

    void DemoWindow::show(const Page &page) {
        if (_panel) {
            _content->remove(_panel);
        }
        _panel = page.make();
        _panel->grow(1)
              .shrink(1)
              .basis(0);
        _content->add(_panel);
        _header->title(page.name)
                .icon(page.icon);
    }

    bool DemoWindow::showPage(const std::string &name) {
        for (Page &p: _pages) {
            if (p.name == name) {
                _shell->sidebar().select(p.item);
                return true;
            }
        }
        return false;
    }

    bool DemoWindow::useTheme(const std::string &name) {
        for (const NamedTheme &named: _themes) {
            if (named.key == name || named.label == name) {
                if (named.theme) {
                    theme(*named.theme);
                } else {
                    followSystemTheme();
                }
                _themeItem->count(named.label);
                return true;
            }
        }
        return false;
    }

    void DemoWindow::openThemeMenu() {
        std::vector<std::shared_ptr<MenuItem>> items{};
        for (const NamedTheme &named: _themes) {
            items.push_back(Menu::item(named.label, [this, key = named.key]() { useTheme(key); }));
        }
        // Beside the item, its last row level with it: the menu's rows and its padding and border above
        const Metrics &m      = theme().metrics;
        const int     height = static_cast<int>(items.size() * m.pointer.rowSm + 2 * (m.space(1) + m.border));
        int           x      = 0;
        int           y      = 0;
        _themeItem->localToGlobal(x, y, _themeItem->getWidth() + 8, _themeItem->getHeight() - height);
        openMenu(items, x, y);
    }

    void DemoWindow::showFps(const bool show) {
        fpsLabel->visible(show);
    }

    void DemoWindow::render(SkCanvas *canvas) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(3) << fps;
        fpsLabel->text("FPS: " + stream.str());
        Window::render(canvas);
    }
}
