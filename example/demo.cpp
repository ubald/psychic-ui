#include <iostream>
#include <string>
#include "demo.hpp"
#include "example/demo/DemoWindow.hpp"
#include <psychic-ui/applications/SDL3Application.hpp>

using namespace psychic_ui;

/**
 * psychic-ui-demo [--theme <system|pivotal-dark|pivotal-light|one-dark|one-light|psychic>] [--page <name>] [--no-fps]
 */
int main(int argc, char **argv) {
    try {
        SDL3Application app;
        auto            window = std::make_shared<DemoWindow>();
        app.open(window);

        for (int i = 1; i < argc; ++i) {
            const std::string arg  = argv[i];
            const bool        more = i + 1 < argc;
            if (arg == "--theme" && more) {
                if (!window->useTheme(argv[++i])) {
                    std::cerr << "Unknown theme: " << argv[i] << std::endl;
                }
            } else if (arg == "--page" && more) {
                if (!window->showPage(argv[++i])) {
                    std::cerr << "Unknown page: " << argv[i] << std::endl;
                }
            } else if (arg == "--no-fps") {
                window->showFps(false);
            } else {
                std::cerr << "Unknown argument: " << arg << std::endl;
            }
        }

        app.run();
    } catch (const std::exception &e) {
        std::cerr << "Caught a fatal error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
