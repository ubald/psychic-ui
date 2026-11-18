#include "Looks.hpp"

#include <cctype>
#include <stdexcept>
#include <vector>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/skins/desk/DeskSkins.hpp>
#include <psychic-ui/skins/float/FloatSkins.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/themes/desk.hpp>
#include <psychic-ui/themes/float.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/themes/psychic.hpp>

namespace psychic_ui::showcase {
    std::unique_ptr<SkinSet> makeSkinSet(const std::string &key) {
        if (key == "default") {
            return std::make_unique<PivotalSkins>();
        }
        if (key == "desk") {
            return std::make_unique<DeskSkins>();
        }
        if (key == "float") {
            return std::make_unique<FloatSkins>();
        }
        return nullptr;
    }

    const Theme *findTheme(const std::string &key) {
        static const std::vector<const Theme *> all{
            &themes::pivotalDark, &themes::pivotalLight, &themes::oneDark, &themes::oneLight, &themes::psychic,
            &themes::deskDark, &themes::deskLight, &themes::floatLight, &themes::floatDark,
        };
        for (const Theme *theme: all) {
            std::string name{theme->name};
            for (char &c: name) {
                c = c == ' ' ? '-' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            if (name == key) {
                return theme;
            }
        }
        return nullptr;
    }

    void applyLook(Window &window, const Look &look) {
        window.setStyleManager(std::make_shared<StyleManager>());
        auto set = makeSkinSet(look.skinSet);
        if (!set) {
            throw std::invalid_argument("Unknown skin set: " + look.skinSet);
        }
        window.skinSet(std::move(set));
        if (!look.theme.empty()) {
            const Theme *theme = findTheme(look.theme);
            if (!theme) {
                throw std::invalid_argument("Unknown theme: " + look.theme);
            }
            window.theme(*theme);
        }
        window.inputMode(look.input);
    }

    std::vector<NamedLook> skinSetLooks() {
        return {
            {"default-dark", {.skinSet = "default", .theme = "pivotal-dark"}},
            {"default-light", {.skinSet = "default", .theme = "pivotal-light"}},
            {"desk-dark", {.skinSet = "desk", .theme = "desk-dark"}},
            {"desk-light", {.skinSet = "desk", .theme = "desk-light"}},
            {"float-dark", {.skinSet = "float", .theme = "float-dark"}},
            {"float-light", {.skinSet = "float", .theme = "float-light"}},
        };
    }

    std::vector<Scene> inEveryLook(const Scene &scene) {
        return inSkinSet(scene, "");
    }

    std::vector<Scene> inSkinSet(const Scene &scene, const std::string &skinSet) {
        std::vector<Scene> scenes{};
        for (const NamedLook &named: skinSetLooks()) {
            if (!skinSet.empty() && named.look.skinSet != skinSet) {
                continue;
            }
            Scene copy      = scene;
            copy.name       = scene.name + "-" + named.suffix;
            copy.look       = named.look;
            copy.look.input = scene.look.input;
            scenes.push_back(std::move(copy));
        }
        return scenes;
    }
}
