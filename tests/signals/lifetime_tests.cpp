#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include "include/core/SkSurface.h"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/MenuBar.hpp>
#include <psychic-ui/components/MenuButton.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>

using namespace psychic_ui;

namespace {
    // A Window that runs the passes of `drawAll` on a raster canvas, without a GL context
    struct HeadlessWindow : Window {
        sk_sp<SkSurface> surface{SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1440, 900))};

        HeadlessWindow() : Window("Headless") {
            setWindowSize(1440, 900);
            reducedMotion(true); // Popups close at once: the tests are about lifetimes, not motion
            skinSet<PivotalSkins>();
        }

        void layout() {
            // Twice: rendering creates the children of data containers (MenuBar, Menu)
            for (int i = 0; i < 2; ++i) {
                updateStylesAndLayout();
                render(surface->getCanvas());
            }
        }

        void clickAt(int x, int y) {
            layout();
            handleMouseMove({.x = static_cast<float>(x), .y = static_cast<float>(y)}, false);
            handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = true});
            layout();
            handleMouseButtonChange({.x = static_cast<float>(x), .y = static_cast<float>(y), .button = MouseButton::LEFT, .down = false});
            layout();
        }

        static void center(const std::shared_ptr<Div> &div, int &x, int &y) {
            div->localToGlobal(x, y, div->getWidth() / 2, div->getHeight() / 2);
        }
    };

    std::shared_ptr<Div> findButton(const std::shared_ptr<Div> &root, const std::string &label) {
        if (auto button = std::dynamic_pointer_cast<Button>(root); button && button->label() == label) {
            return button;
        }
        if (auto menuButton = std::dynamic_pointer_cast<MenuButton>(root);
            menuButton && menuButton->menuItem() && menuButton->menuItem()->label == label) {
            return menuButton;
        }
        for (const auto &child: root->children()) {
            if (auto found = findButton(child, label)) {
                return found;
            }
        }
        return nullptr;
    }

    std::shared_ptr<Div> findButton(Div &root, const std::string &label) {
        return findButton(root.shared_from_this(), label);
    }

    struct FakeWindow : Div {
        Signal<> menuClosed{};
    };

    struct FakeMenuBar : Div {
        int closed = 0;

        void listen(FakeWindow &window) {
            subscribeTo(window.menuClosed, [this]() { ++closed; });
        }
    };
}

TEST_CASE("A div's handler removes the div from its parent", "[signals][lifetime]") {
    // What a menu item does through window()->closeMenu() -> menu->removeAll()
    auto menu   = std::make_shared<Div>();
    Div  *item  = &menu->add<Div>(); // Only owner: menu's children
    int  ran    = 0;
    item->clicked.subscribe([&]() { ++ran; menu->removeAll(); });
    item->clicked.subscribe([&]() { ++ran; }); // Its signal is gone, not called
    item->clicked.emit(MouseEvent{});
    REQUIRE(ran == 1);
    REQUIRE(menu->childCount() == 0);
}

TEST_CASE("A div clicked through dispatch removes itself", "[signals][lifetime]") {
    auto window = std::make_shared<HeadlessWindow>();
    auto &root  = window->appContainer();
    // Only owners: their parents
    auto &item  = root.add<Div>().width(100).height(100).add<Div>().width(100).height(100);
    int  clicked = 0;
    item.onClick([&, raw = &item]() {
        ++clicked;
        root.removeAll(); // Destroys the dispatching div and its parent
        REQUIRE(raw->clicked.subscriptionCount() == 1); // Still alive while dispatching
    });
    const std::weak_ptr<Div> weakItem = item.weak_from_this();
    window->clickAt(50, 50);
    REQUIRE(clicked == 1);
    REQUIRE(root.childCount() == 0);
    REQUIRE(weakItem.expired());
}

TEST_CASE("An observer child outlives its parent's signal at teardown", "[signals][lifetime]") {
    // Window's members, onMenuClose included, die before its children (MenuBar subscribes to menuClosed)
    auto window = std::make_shared<FakeWindow>();
    auto &bar   = window->add<FakeMenuBar>();
    bar.listen(*window);
    window->menuClosed.emit();
    REQUIRE(bar.closed == 1);
    window.reset();
}

TEST_CASE("Menus in a headless window", "[signals][lifetime][menu]") {
    auto window = std::make_shared<HeadlessWindow>();
    int  newClicked = 0;
    window->appContainer().add<MenuBar>(
        std::vector<std::shared_ptr<MenuItem>>{
            Menu::item(
                "File", {
                    Menu::item("New", [&]() { ++newClicked; }),
                    Menu::item("Recent", {Menu::item("Nothing", []() {})}),
                }
            ),
            Menu::item("Help", []() {}),
        }
    );
    window->layout();

    auto file = findButton(window, "File");
    REQUIRE(file != nullptr);

    SECTION("clicking an item closes the menu that holds it") {
        int x = 0;
        int y = 0;
        HeadlessWindow::center(file, x, y);
        window->clickAt(x, y);
        auto item = findButton(window->overlay(), "New");
        REQUIRE(item != nullptr);
        REQUIRE(std::static_pointer_cast<Button>(file)->selected());

        HeadlessWindow::center(item, x, y);
        const std::weak_ptr<Div> weakItem = item;
        item.reset();
        window->clickAt(x, y);
        REQUIRE(newClicked == 1);
        REQUIRE(weakItem.expired());
        REQUIRE(window->overlay().childCount() == 0);
        REQUIRE_FALSE(std::static_pointer_cast<Button>(file)->selected());

        // The focus moved to the removed item: clicking again must not reach it
        HeadlessWindow::center(file, x, y);
        window->clickAt(x, y);
        REQUIRE(findButton(window->overlay(), "New") != nullptr);
    }

    SECTION("an item's own onClick emitted directly") {
        int x = 0;
        int y = 0;
        HeadlessWindow::center(file, x, y);
        window->clickAt(x, y);
        auto item = findButton(window->overlay(), "New");
        REQUIRE(item != nullptr);
        Div *raw = item.get();
        item.reset();
        raw->clicked.emit(MouseEvent{});
        REQUIRE(newClicked == 1);
    }

    SECTION("hovering a submenu then closing frees its buttons") {
        int x = 0;
        int y = 0;
        HeadlessWindow::center(file, x, y);
        window->clickAt(x, y);
        auto recent = findButton(window->overlay(), "Recent");
        REQUIRE(recent != nullptr);
        HeadlessWindow::center(recent, x, y);
        window->handleMouseMove({.x = static_cast<float>(x), .y = static_cast<float>(y)}, false);
        window->layout();
        REQUIRE(findButton(window->overlay(), "Nothing") != nullptr);
        const std::weak_ptr<Div> weakRecent = recent;
        recent.reset();
        window->closeMenu();
        REQUIRE(weakRecent.expired());
    }

    SECTION("window teardown with the menu bar subscribed to onMenuClose") {
        // Members (menuClosed) die before the children (MenuBar)
        file.reset();
        window.reset();
    }
}
