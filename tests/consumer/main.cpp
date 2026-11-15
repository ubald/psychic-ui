// An application of another project: it finds its fonts next to its executable, whatever the current directory,
// and reaches the UI thread through Window::post
#include <cstdio>
#include <memory>
#include <string>
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include <psychic-ui/Window.hpp>
#include <psychic-ui/skins/default/DefaultSkins.hpp>
#include <psychic-ui/style/StyleManager.hpp>

using namespace psychic_ui;

namespace {
    std::string family(const sk_sp<SkTypeface> &typeface) {
        SkString name;
        typeface->getFamilyName(&name);
        return name.c_str();
    }
}

int main() {
    // No system window: the window's styles and tasks work without one
    auto window = std::make_shared<Window>("Consumer");
    window->skinSet<DefaultSkins>();

    // The skin set loaded the theme's fonts from res/, which psychic_ui_copy_resources copied next to the executable.
    // A font that didn't load falls back to the system's, or its default font, so load the file again under a name no
    // system font has: only the file gives it the theme font's family rather than the default one.
    StyleManager     *manager = window->styleManager();
    const Font       &regular = manager->theme().type.regular;
    const std::string fallback = family(manager->font("Psychic UI Consumer Unknown"));
    manager->loadFont("Psychic UI Consumer Regular", std::string{regular.path});
    const std::string loaded = family(manager->font("Psychic UI Consumer Regular"));
    if (loaded == fallback || loaded != family(manager->font(std::string{regular.name}))) {
        std::fprintf(stderr, "The theme's regular font (%.*s) didn't load from %s: got %s\n",
                     static_cast<int>(regular.path.size()), regular.path.data(),
                     manager->resourceRoot().string().c_str(), loaded.c_str());
        return 1;
    }

    bool ran = false;
    window->post([&ran] { ran = true; });
    window->runPosted();
    if (!ran) {
        std::fprintf(stderr, "A posted task didn't run in runPosted\n");
        return 1;
    }
    return 0;
}
