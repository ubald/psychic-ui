#include <cmath>
#include <functional>
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Inspector.hpp>
#include <psychic-ui/components/ToolBar.hpp>
#include <psychic-ui/skins/desk/DeskSkins.hpp>
#include <psychic-ui/skins/float/FloatSkins.hpp>
#include <psychic-ui/themes/desk.hpp>
#include <psychic-ui/themes/float.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/themes/psychic.hpp>
#include "../../example/demo-stylesheet.hpp"
#include "../../example/demo/Canvas.hpp"
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    /**
     * The demo's Canvas page filling a window with a style manager of its own, so the demo's rules stay out of the
     * other tests' shared one
     */
    struct Fixture {
        std::shared_ptr<TestWindow> window{make()};
        FakeSystemWindow            systemWindow{window, 1000, 700};
        CanvasPage                  *page{nullptr};

        static std::shared_ptr<TestWindow> make() {
            auto window = std::make_shared<TestWindow>();
            window->setStyleManager(std::make_shared<StyleManager>());
            window->skinSet<DefaultSkins>();
            window->loadStyleSheet<DemoStyleSheet>();
            return window;
        }

        Fixture() {
            page = &window->appContainer().add<CanvasPage>().size(100_pct, 100_pct);
            layout();
        }

        void layout() { systemWindow.layout(); }

        /**
         * The first div under the page that the predicate accepts, depth first
         */
        Div *find(const std::function<bool(Div *)> &accept) const {
            std::function<Div *(Div *)> search = [&](Div *div) -> Div * {
                for (const auto &child: div->children()) {
                    if (accept(child.get())) {
                        return child.get();
                    }
                    if (Div *found = search(child.get())) {
                        return found;
                    }
                }
                return nullptr;
            };
            return search(page);
        }

        template<class T>
        T *find() const {
            return dynamic_cast<T *>(find([](Div *div) { return dynamic_cast<T *>(div) != nullptr; }));
        }

        Button *currentTool() const {
            return dynamic_cast<Button *>(find([](Div *div) {
                auto *button = dynamic_cast<Button *>(div);
                return button && button->parent() && dynamic_cast<ToolBar *>(button->parent()) && button->selected();
            }));
        }
    };

    void checkCanvas(const Fixture &f, const Theme &theme) {
        INFO(theme.name);
        const Palette &p = theme.palette;
        CHECK(f.page->computedStyle().backgroundColor() == p.frame);
        CHECK(f.page->computedStyle().dotGridColor() == p.canvasDot);

        // The inspector floats at the top right, the tool bar centered at the bottom
        auto *inspector = f.find<Inspector>();
        auto *tools     = f.find<ToolBar>();
        REQUIRE(inspector != nullptr);
        REQUIRE(tools != nullptr);
        const float width = f.page->getWidth();
        CHECK(inspector->x() + inspector->getWidth() == width - theme.metrics.space(4));
        CHECK(std::abs(tools->x() + tools->getWidth() / 2 - width / 2) < 1);
        CHECK(tools->parent()->y() + tools->parent()->getHeight() == f.page->getHeight() - theme.metrics.space(5));

        // With the theme's panel elevation, no border
        CHECK(inspector->computedStyle().shadowColor() == theme.elevation.panel.color);
        CHECK(inspector->computedStyle().shadowBlur() == theme.elevation.panel.blur);
        CHECK(inspector->computedStyle().shadow2Color() == theme.elevation.panelContact.color);
        CHECK(inspector->computedStyle().shadow2Spread() == theme.elevation.panelContact.spread);
        CHECK(inspector->computedStyle().border() == 0);
        CHECK(tools->computedStyle().shadowColor() == theme.elevation.panel.color);
        CHECK(tools->computedStyle().border() == 0);
        CHECK(inspector->computedStyle().borderRadius() == theme.metrics.radius.surface);

        // One tool at a time, in the accent
        Button *current = f.currentTool();
        REQUIRE(current != nullptr);
        CHECK(current->skin()->computedStyle().backgroundColor() == p.accent);
        CHECK(current->skin()->computedStyle().color() == p.onAccent);
    }
}

TEST_CASE("the demo's Canvas page under every skin set", "[demo][canvas]") {
    Fixture f;

    SECTION("the default set") {
        checkCanvas(f, themes::pivotalDark);
        for (const Theme *theme: {&themes::pivotalLight, &themes::oneDark, &themes::oneLight, &themes::psychic}) {
            f.window->theme(*theme);
            f.layout();
            checkCanvas(f, *theme);
        }
    }

    SECTION("Float") {
        f.window->skinSet<FloatSkins>();
        f.layout();
        checkCanvas(f, themes::floatLight);
        f.window->theme(themes::floatDark);
        f.layout();
        checkCanvas(f, themes::floatDark);
    }

    SECTION("Desk, at both densities") {
        f.window->skinSet<DeskSkins>();
        f.layout();
        checkCanvas(f, themes::deskDark);
        f.window->theme(themes::deskLight);
        f.layout();
        checkCanvas(f, themes::deskLight);
        f.window->handleInputModeChange(InputMode::Touch);
        f.layout();
        checkCanvas(f, themes::deskLight);
        f.window->handleInputModeChange(InputMode::Pointer);
    }

    SECTION("picking another tool") {
        auto *tools = f.find<ToolBar>();
        REQUIRE(tools != nullptr);
        auto *text = dynamic_cast<Button *>(tools->children()[3].get());
        REQUIRE(text != nullptr);
        text->clicked.emit(text->mouseEventAtCenter());
        f.layout();
        CHECK(f.currentTool() == text);
        int selected = 0;
        for (const auto &child: tools->children()) {
            if (auto *tool = dynamic_cast<Button *>(child.get()); tool && tool->selected()) {
                ++selected;
            }
        }
        CHECK(selected == 1);
    }
}
