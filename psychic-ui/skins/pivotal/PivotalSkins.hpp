#pragma once

#include "psychic-ui/skins/SkinSet.hpp"

namespace psychic_ui {
    /**
     * Psychic UI's own look, the default set ("Default"): Pivotal's design system (P1 Faithful), with Pivotal Dark
     * and Pivotal Light. A skin for every component, the bases for most, Pivotal's own for tabs, window controls and
     * avatars, and the rules laying them out. Other sets fall back on it.
     *
     *     window.skinSet<PivotalSkins>();
     */
    class PivotalSkins : public SkinSet {
    public:
        [[nodiscard]] std::string_view name() const override;
        [[nodiscard]] const Theme &defaultTheme() const override;
        [[nodiscard]] const Theme &lightTheme() const override;
        [[nodiscard]] std::vector<const Theme *> themes() const override; // Pivotal's two, then One Dark, One Light, Psychic
        void registerSkins(SkinRegistry &skins) const override;
        void load(StyleManager *manager) override;
    };
}
