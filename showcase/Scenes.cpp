#include <memory>
#include <utility>
#include <vector>
#include "Scene.hpp"

#include <psychic-ui/Window.hpp>
#include "Looks.hpp"
#include "ShowcaseStyleSheet.hpp"

namespace psychic_ui::showcase {
    std::shared_ptr<Window> stageWindow(const Look &look) {
        auto window = std::make_shared<Window>("Psychic UI Showcase");
        applyLook(*window, look);
        window->loadStyleSheet<ShowcaseStyleSheet>();
        return window;
    }

    std::vector<Scene> scenes() {
        std::vector<Scene> all{};
        for (auto group: {heroScenes, controlScenes, popupScenes, collectionScenes, foundationScenes}) {
            for (Scene &scene: group()) {
                all.push_back(std::move(scene));
            }
        }
        return all;
    }
}
