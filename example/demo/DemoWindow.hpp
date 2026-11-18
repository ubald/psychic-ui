#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/AppShell.hpp>
#include <psychic-ui/components/PageHeader.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/MenuBar.hpp>
#include <psychic-ui/components/Spacer.hpp>
#include <psychic-ui/components/ToolBar.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>
#include <psychic-ui/skins/desk/DeskSkins.hpp>
#include <psychic-ui/skins/float/FloatSkins.hpp>
#include "Typography.hpp"
#include "TextInputs.hpp"
#include "Buttons.hpp"
#include "Divs.hpp"
#include "Scrollers.hpp"
#include "Canvas.hpp"
#include "Ranges.hpp"
#include "Colors.hpp"
#include "Icons.hpp"
#include "Lists.hpp"
#include "Inputs.hpp"
#include "Popups.hpp"
#include "Motion.hpp"
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
         * Show a page by its name ("Buttons", or its key, "text-input"); false when there is none
         */
        bool showPage(const std::string &name);

        /**
         * Use a skin set by its key ("default", "desk", "float"; see `_skinSets`); false for another name
         */
        bool useSkinSet(const std::string &key);

        /**
         * Use a theme by its key, its name in kebab case ("pivotal-dark", "float-light"), or "system" (follow the
         * system); the current set's themes first, then the other sets'. False for another name.
         */
        bool useTheme(const std::string &key);

        /**
         * Keep the window in this input mode whatever the devices report, and so the style manager at its density
         * (for captures of the touch sizes without a touch screen); nullopt follows the devices again
         */
        void forceInputMode(std::optional<InputMode> mode);

        /**
         * Show or hide the frame rate counter. While it shows, the window draws every frame (to count them); hidden,
         * it only draws when something changes.
         */
        void showFps(bool show);

        /**
         * Press keys, for screenshots: names separated by commas (tab, shift+tab, up, down, left, right, home, end,
         * space, enter, escape), a name followed by `*n` pressed n times; false for an unknown name
         */
        bool pressKeys(const std::string &keys);

        /**
         * Scroll the page's first scrolling div down by this many pixels, for screenshots
         */
        void scrollPage(int pixels);

        /**
         * Show the Popups page and open one of its popups (see `Popups::openPopup`); false for an unknown name
         */
        bool openPopup(const std::string &name);

    protected:
        void render(SkCanvas *canvas) override;
        void handleInputModeChange(InputMode mode) override;

    private:
        struct Page {
            std::string                          name;
            std::string                          icon;
            std::function<std::shared_ptr<Div>()> make;
            SidebarItem                          *item{nullptr};
        };

        struct NamedSkinSet {
            std::string                               key;
            std::function<std::unique_ptr<SkinSet>()> make;
        };

        template<class S>
        static NamedSkinSet named(const std::string &key) {
            return {key, [] { return std::make_unique<S>(); }};
        }

        // The skin sets the View menu and `--skin` offer: adding one is one line
        const std::vector<NamedSkinSet> _skinSets{
            named<PivotalSkins>("default"),
            named<DeskSkins>("desk"),
            named<FloatSkins>("float"),
        };

        std::vector<Page>               _pages{};
        std::shared_ptr<MenuRadioGroup> _skinSetChoice{};
        std::shared_ptr<MenuRadioGroup> _themeChoice{};
        std::shared_ptr<SubMenuItem>    _themeMenu{}; // View → Theme, remade for each skin set
        AppShell                *_shell{nullptr};
        PageHeader              *_header{nullptr};
        Div                     *_content{nullptr};
        std::shared_ptr<Div>    _panel{};
        SidebarItem             *_themeItem{nullptr};
        Label                   *fpsLabel{nullptr};
        std::optional<InputMode> _forcedInputMode{};

        void show(const Page &page);
        void openThemeMenu();
        void themesChanged();

        static std::string keyOf(std::string_view name);
    };

    inline DemoWindow::DemoWindow() :
        Window("Psychic UI") {
        _decorated = false;

        skinSet<PivotalSkins>();          // Its theme follows the system: Pivotal Light or Pivotal Dark
        loadStyleSheet<DemoStyleSheet>(); // Over the skin set's rules; both load again on a theme change

        // View → Skin Set and View → Theme: radio groups, switching live (the menu stays open)
        _skinSetChoice = std::make_shared<MenuRadioGroup>("default", [this](const std::string &key) { useSkinSet(key); });
        _themeChoice   = std::make_shared<MenuRadioGroup>("system", [this](const std::string &key) { useTheme(key); });
        MenuItems skinSetItems{};
        for (const NamedSkinSet &set: _skinSets) {
            skinSetItems.push_back(Menu::radio(_skinSetChoice, std::string{set.make()->name()}, set.key));
        }
        _themeMenu = Menu::item("Theme", MenuItems{}, "", "sun");

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
            {"Canvas", "hash", page([] { return std::make_shared<CanvasPage>(); })},
            {"Motion", "play", page([] { return std::make_shared<MotionPanel>(); })}, // In Layout, after Canvas
            {"Colors", "eyedropper", page([] { return std::make_shared<Colors>(); })},
            {"Icons", "star", page([] { return std::make_shared<IconsPanel>(); })},
            {"Inputs", "settings", page([] { return std::make_shared<Inputs>(); })}, // In Controls, after Text Input
            {"Lists", "list", page([] { return std::make_shared<Lists>(); })},
            {"Popups", "layout", page([] { return std::make_shared<Popups>(); })},
        };

        auto &controls = sidebar.section("Controls");
        auto &layout   = sidebar.section("Layout");
        auto &assets   = sidebar.section("Assets");
        assets.grow(1); // Pushes the last section to the bottom
        for (Page &p: _pages) {
            SidebarSection *section = &controls;
            if (p.name == "Divs" || p.name == "Scroller" || p.name == "Canvas" || p.name == "Motion" || p.name == "Typography") {
                section = &layout;
            } else if (p.name == "Colors" || p.name == "Icons") {
                section = &assets;
            }
            p.item = &section->item(p.name, p.icon);
            if (p.name == "Icons") {
                p.item->count(std::to_string(IconRegistry::shared().names().size()));
            }
        }

        auto &bottom = sidebar.section();
        _themeItem = &bottom.item("Theme", "sun");
        _themeItem->selectable(false); // Opens a menu
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

        const std::vector<std::shared_ptr<MenuItem>> menus{
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
                    Menu::item("Skin Set", skinSetItems, "", "layers"),
                    _themeMenu,
                    Menu::separator(),
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
                .fontSmoothing(FontSmoothing::None)
                .position(Position::Absolute)
                .right(4)
                .bottom(0);

        themesChanged();
        showPage("Buttons");
    }

    inline void DemoWindow::show(const Page &page) {
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

    inline bool DemoWindow::showPage(const std::string &name) {
        const auto page = std::ranges::find_if(_pages, [&name](const Page &p) { return p.name == name || keyOf(p.name) == name; });
        if (page == _pages.end()) {
            return false;
        }
        _shell->sidebar().select(page->item);
        return true;
    }

    inline std::string DemoWindow::keyOf(std::string_view name) {
        std::string key{name};
        for (char &c: key) {
            c = c == ' ' ? '-' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return key;
    }

    inline bool DemoWindow::useSkinSet(const std::string &key) {
        const auto set = std::ranges::find_if(_skinSets, [&key](const NamedSkinSet &named) { return named.key == key; });
        if (set == _skinSets.end()) {
            return false;
        }
        skinSet(set->make()); // The library settles the theme (see Window::skinSet)
        themesChanged();
        return true;
    }

    inline bool DemoWindow::useTheme(const std::string &key) {
        if (key == "system") {
            followSystemTheme();
            themesChanged();
            return true;
        }
        // The current set's themes, then the others' (any theme works with any set)
        std::vector<const Theme *> candidates = styleManager()->skinSet()->themes();
        for (const NamedSkinSet &set: _skinSets) {
            for (const Theme *other: set.make()->themes()) {
                candidates.push_back(other);
            }
        }
        const auto candidate = std::ranges::find_if(candidates, [&key](const Theme *t) { return keyOf(t->name) == key || t->name == key; });
        if (candidate == candidates.end()) {
            return false;
        }
        theme(**candidate);
        themesChanged();
        return true;
    }

    inline void DemoWindow::themesChanged() {
        // The current set's themes, after System
        MenuItems items{Menu::radio(_themeChoice, "System", "system", "", "monitor"), Menu::separator()};
        for (const Theme *own: styleManager()->skinSet()->themes()) {
            items.push_back(
                Menu::radio(_themeChoice, std::string{own->name}, keyOf(own->name), "",
                            own->scheme == ColorScheme::Light ? "sun" : "moon")
            );
        }
        _themeMenu->items = items;

        // What is chosen now: switching skin sets may have changed the theme
        for (const NamedSkinSet &set: _skinSets) {
            if (set.make()->name() == styleManager()->skinSet()->name()) {
                _skinSetChoice->value = set.key;
            }
        }
        _themeChoice->value = followsSystemTheme() ? "system" : keyOf(theme().name);
        _themeItem->count(followsSystemTheme() ? "System" : std::string{theme().name});
    }

    inline void DemoWindow::openThemeMenu() {
        // Beside the item, its bottom level with the item's
        openMenu(_themeMenu->items, *_themeItem, Side::Right, PopupAlign::End).sideOffset(8);
    }

    inline bool DemoWindow::openPopup(const std::string &name) {
        showPage("Popups");
        updateStylesAndLayout(); // The page's anchors are laid out first
        auto popups = std::dynamic_pointer_cast<Popups>(_panel);
        return popups && popups->openPopup(name);
    }

    inline bool DemoWindow::pressKeys(const std::string &keys) {
        static const std::vector<std::pair<std::string, Key>> names{
            {"tab", Key::TAB}, {"up", Key::UP}, {"down", Key::DOWN}, {"left", Key::LEFT}, {"right", Key::RIGHT},
            {"home", Key::HOME}, {"end", Key::END}, {"space", Key::SPACE}, {"enter", Key::ENTER}, {"escape", Key::ESCAPE},
        };
        std::stringstream list(keys);
        std::string       name;
        while (std::getline(list, name, ',')) {
            int count = 1;
            if (const auto star = name.find('*'); star != std::string::npos) {
                count = std::stoi(name.substr(star + 1));
                name  = name.substr(0, star);
            }
            Mod mod{};
            if (name.starts_with("shift+")) {
                mod.shift = true;
                name      = name.substr(6);
            }
            const auto found = std::ranges::find_if(names, [&](const auto &n) { return n.first == name; });
            if (found == names.end()) {
                return false;
            }
            for (int i = 0; i < count; ++i) {
                updateStylesAndLayout(); // What the key goes to is laid out (a dialog has its focus)
                handleKeyPress(found->second, mod);
            }
        }
        return true;
    }

    inline void DemoWindow::scrollPage(const int pixels) {
        updateStylesAndLayout();
        std::function<Div *(Div &)> scroller = [&](Div &div) -> Div * {
            if (div.computedStyle().overflow() == Overflow::Scroll) {
                return &div;
            }
            for (const auto &child: div.children()) {
                if (Div *found = scroller(*child)) {
                    return found;
                }
            }
            return nullptr;
        };
        if (Div *found = _panel ? scroller(*_panel) : nullptr) {
            found->scrollBy(0, -pixels);
        }
    }

    inline void DemoWindow::forceInputMode(const std::optional<InputMode> mode) {
        _forcedInputMode = mode;
        if (mode) {
            Window::handleInputModeChange(*mode);
        }
    }

    inline void DemoWindow::handleInputModeChange(const InputMode mode) {
        Window::handleInputModeChange(_forcedInputMode.value_or(mode));
    }

    inline void DemoWindow::showFps(const bool show) {
        fpsLabel->visible(show);
    }

    inline void DemoWindow::render(SkCanvas *canvas) {
        if (fpsLabel->visible()) {
            std::stringstream stream;
            stream << std::fixed << std::setprecision(3) << fps;
            fpsLabel->text("FPS: " + stream.str());
            requestFrame(); // A frame rate needs frames: the counter keeps them coming
        }
        Window::render(canvas);
    }
}
