#pragma once

#include "psychic-ui/skins/SkinSet.hpp"

namespace psychic_ui {
    /**
     * Psychic UI's own look, Pivotal's design system (P1 Faithful), with Pivotal Dark and Pivotal Light: a skin for
     * every component and the rules laying them out. Other sets fall back on it.
     *
     *     window.skinSet<DefaultSkins>();
     */
    class DefaultSkins : public SkinSet {
    public:
        std::string_view name() const override;
        const Theme &defaultTheme() const override;
        const Theme &lightTheme() const override;
        std::vector<const Theme *> themes() const override; // Pivotal's two, then One Dark, One Light, Psychic
        void registerSkins(SkinRegistry &skins) const override;
        void load(StyleManager *manager) override;
    };
}
