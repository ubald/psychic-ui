// The demo's window and pages are header-only: this is the one file that includes them
#include "example/demo.hpp"
#include "example/demo/DemoWindow.hpp"

#include "../Compose.hpp"
#include "../Looks.hpp"
#include "../Scene.hpp"
#include "../Stage.hpp"

namespace psychic_ui::showcase {
    using namespace psychic_ui::literals;

    namespace {
        /**
         * The demo app (example/), under the look, on one of its pages
         */
        std::function<std::shared_ptr<Window>(const Look &)> demoWindow(const std::string &page) {
            return [page](const Look &look) -> std::shared_ptr<Window> {
                auto window = std::make_shared<DemoWindow>();
                applyLook(*window, look);
                window->loadStyleSheet<DemoStyleSheet>();
                window->useTheme(look.theme); // Again, through the demo, so its theme menu and sidebar say it
                window->forceInputMode(look.input);
                window->showFps(false);
                window->showPage(page);
                // The demo's layout debugging switch is for developing it, not for showing it
                if (Div *debug = compose::findIf(*window, [](Div &div) {
                    auto *button = dynamic_cast<Button *>(&div);
                    return button && button->label() == "Debug Layout";
                })) {
                    debug->display(Display::None);
                }
                return window;
            };
        }
    }

    std::vector<Scene> heroScenes() {
        std::vector<Scene> scenes{};
        for (const NamedLook &named: skinSetLooks()) {
            // The Float board's mini app, made of library components, on its own
            scenes.push_back({
                .name = "canvas-" + named.suffix,
                .title = "A design tool's canvas: floating panels, a tree, an inspector and a tool bar",
                .width = 1280,
                .height = 800,
                .look = named.look,
                .window = [](const Look &look) {
                    auto window = stageWindow(look);
                    window->loadStyleSheet<DemoStyleSheet>();
                    return window;
                },
                .build = [](Window &window) {
                    window.appContainer().add<CanvasPage>().width(100_pct).height(100_pct);
                },
                .pose = [](Stage &stage) {
                    // The mouse resting on a tool, its tooltip shown
                    Div *tool = compose::findIf(stage.window(), [](Div &div) { return div.tooltip() == "Fixture"; });
                    if (tool) {
                        stage.hover(*tool);
                        stage.advance(std::chrono::milliseconds(400));
                    }
                },
            });

            // The whole demo app: the sidebar, the page's title bar and menus, its tool bar and a page of lists
            scenes.push_back({
                .name = "app-" + named.suffix,
                .title = "The demo app: an AppShell with its sidebar, menus, tool bar, list, tree and data grid",
                .width = 1280,
                .height = 800,
                .look = named.look,
                .window = demoWindow("lists"),
            });
        }
        return scenes;
    }
}
