#pragma once

#include "psychic-ui/skins/SkinSet.hpp"

namespace psychic_ui {
    /**
     * Psychic UI's own look, with One Dark: a skin for every component and the rules laying them out.
     * Other sets fall back on it.
     *
     *     window.skinSet<DefaultSkins>();
     */
    class DefaultSkins : public SkinSet {
    public:
        std::string_view name() const override;
        const Theme &defaultTheme() const override;
        void registerSkins(SkinRegistry &skins) const override;
        void load(StyleManager *manager) override;
    };
}
