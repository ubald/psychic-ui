#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Scene.hpp"

namespace psychic_ui {
    class SkinSet;
    class Window;
    struct Theme;
}

namespace psychic_ui::showcase {
    /**
     * A skin set by its key: "default", "desk" or "float"; nullptr for another
     */
    std::unique_ptr<SkinSet> makeSkinSet(const std::string &key);

    /**
     * A theme by its key, its name in kebab case ("pivotal-dark", "float-light"); nullptr for another
     */
    const Theme *findTheme(const std::string &key);

    /**
     * Give a window a style manager of its own (so scenes don't share rules), then the look: the skin set, the
     * theme and the input mode. Throws for an unknown key.
     */
    void applyLook(Window &window, const Look &look);

    /**
     * A look with the name scenes add to theirs ("default-dark")
     */
    struct NamedLook {
        std::string suffix;
        Look        look;
    };

    /**
     * Every skin set in its dark and light themes: what the site's switcher swaps between
     */
    std::vector<NamedLook> skinSetLooks();

    /**
     * A scene once per look of `skinSetLooks()` (its input mode kept), named after it: "buttons-default-dark"
     */
    std::vector<Scene> inEveryLook(const Scene &scene);

    /**
     * A scene in one skin set's dark and light themes only, named as `inEveryLook` names them
     */
    std::vector<Scene> inSkinSet(const Scene &scene, const std::string &skinSet);
}
