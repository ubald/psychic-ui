#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include "example/demo/DemoWindow.hpp"
#include <psychic-ui/applications/SDL3Application.hpp>
#include <psychic-ui/psychic-ui.hpp>

using namespace psychic_ui;

/**
 * psychic-ui-demo [--skin <default|desk|float>] [--theme <system|pivotal-dark|pivotal-light|one-dark|one-light|psychic|
 *                 desk-dark|desk-light|float-light|float-dark>] [--density <pointer|touch>] [--page <name>] [--no-fps]
 *                 [--reduced-motion] [--slow-motion]
 *                 [--open <menu|submenu|context|select|dialog|confirm|toasts|tooltip|view>]
 *                 [--keys <key,key*n,...>] [--scroll <pixels>]
 *
 * The skin set applies first, then the density, the theme, the page, the popup, the scroll and the keys, whatever
 * their order. `--density touch` keeps the window in touch mode, as if a finger were on the screen, whatever the mouse
 * does (for captures of the touch sizes).
 */
int main(int argc, char **argv) {
    try {
        SDL3Application app;
        auto            window = std::make_shared<DemoWindow>();
        app.open(window);

        std::string skin{};
        std::string density{};
        std::string theme{};
        std::string page{};
        std::string open{};
        std::string keys{};
        int         scroll = 0;
        const std::span<char *> args{argv, static_cast<std::size_t>(argc)};
        for (std::size_t i = 1; i < args.size(); ++i) {
            const std::string arg  = args[i];
            const bool        more = i + 1 < args.size();
            if (arg == "--skin" && more) {
                skin = args[++i];
            } else if (arg == "--density" && more) {
                density = args[++i];
            } else if (arg == "--theme" && more) {
                theme = args[++i];
            } else if (arg == "--page" && more) {
                page = args[++i];
            } else if (arg == "--open" && more) {
                open = args[++i];
            } else if (arg == "--keys" && more) {
                keys = args[++i];
            } else if (arg == "--scroll" && more) {
                scroll = std::stoi(args[++i]);
            } else if (arg == "--no-fps") {
                window->showFps(false);
            } else if (arg == "--reduced-motion") {
                window->reducedMotion(true);
            } else if (arg == "--slow-motion") {
                window->animationSpeed(0.1f); // Ten times slower, to look at animations
            } else {
                std::cerr << "Unknown argument: " << arg << '\n';
            }
        }

        if (!skin.empty() && !window->useSkinSet(skin)) {
            std::cerr << "Unknown skin set: " << skin << '\n';
        }
        if (density == "touch" || density == "pointer") {
            window->forceInputMode(density == "touch" ? InputMode::Touch : InputMode::Pointer);
        } else if (!density.empty()) {
            std::cerr << "Unknown density: " << density << '\n';
        }
        if (!theme.empty() && !window->useTheme(theme)) {
            std::cerr << "Unknown theme: " << theme << '\n';
        }
        if (!page.empty() && !window->showPage(page)) {
            std::cerr << "Unknown page: " << page << '\n';
        }
        // For screenshots
        if (!open.empty() && !window->openPopup(open)) {
            std::cerr << "Unknown popup: " << open << '\n';
        }
        if (scroll != 0) {
            window->scrollPage(scroll);
        }
        // For screenshots of the keyboard's focus
        if (!keys.empty() && !window->pressKeys(keys)) {
            std::cerr << "Unknown key in: " << keys << '\n';
        }

        app.run();
    } catch (const std::exception &e) {
        std::cerr << "Caught a fatal error: " << e.what() << '\n';
        return -1;
    }

    return 0;
}
