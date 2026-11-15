#include <iostream>
#include <string>
#include "demo.hpp"
#include "example/demo/DemoWindow.hpp"
#include <psychic-ui/applications/SDL3Application.hpp>

using namespace psychic_ui;

/**
 * psychic-ui-demo [--skin <default|float>] [--theme <system|pivotal-dark|pivotal-light|one-dark|one-light|psychic|
 *                 float-light|float-dark>] [--page <name>] [--no-fps]
 *                 [--open <menu|submenu|context|select|dialog|confirm|toasts|tooltip|view>]
 *                 [--keys <key,key*n,...>] [--scroll <pixels>]
 *
 * The skin set applies first, then the theme, the page, the popup, the scroll and the keys, whatever their order.
 */
int main(int argc, char **argv) {
    try {
        SDL3Application app;
        auto            window = std::make_shared<DemoWindow>();
        app.open(window);

        std::string skin{};
        std::string theme{};
        std::string page{};
        std::string open{};
        std::string keys{};
        int         scroll = 0;
        for (int i = 1; i < argc; ++i) {
            const std::string arg  = argv[i];
            const bool        more = i + 1 < argc;
            if (arg == "--skin" && more) {
                skin = argv[++i];
            } else if (arg == "--theme" && more) {
                theme = argv[++i];
            } else if (arg == "--page" && more) {
                page = argv[++i];
            } else if (arg == "--open" && more) {
                open = argv[++i];
            } else if (arg == "--keys" && more) {
                keys = argv[++i];
            } else if (arg == "--scroll" && more) {
                scroll = std::stoi(argv[++i]);
            } else if (arg == "--no-fps") {
                window->showFps(false);
            } else {
                std::cerr << "Unknown argument: " << arg << std::endl;
            }
        }

        if (!skin.empty() && !window->useSkinSet(skin)) {
            std::cerr << "Unknown skin set: " << skin << std::endl;
        }
        if (!theme.empty() && !window->useTheme(theme)) {
            std::cerr << "Unknown theme: " << theme << std::endl;
        }
        if (!page.empty() && !window->showPage(page)) {
            std::cerr << "Unknown page: " << page << std::endl;
        }
        // For screenshots
        if (!open.empty() && !window->openPopup(open)) {
            std::cerr << "Unknown popup: " << open << std::endl;
        }
        if (scroll != 0) {
            window->scrollPage(scroll);
        }
        // For screenshots of the keyboard's focus
        if (!keys.empty() && !window->pressKeys(keys)) {
            std::cerr << "Unknown key in: " << keys << std::endl;
        }

        app.run();
    } catch (const std::exception &e) {
        std::cerr << "Caught a fatal error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
