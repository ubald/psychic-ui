#pragma once

#include "psychic-ui/skins/SkinSet.hpp"

namespace psychic_ui {
    /**
     * S5 Float, the sample skin set: rounded panels floating over a dotted canvas, gentle two-layer shadows,
     * neutral greys and a violet accent, light first (Float Light, Float Dark).
     *
     * It shows how little a skin set needs: it falls back on the default set for every skin and rule, and only
     * overrides the rules whose look differs. Most of the change comes from its themes alone (corners, sizes, colors),
     * since the default set's rules are written from the theme.
     *
     *     window.skinSet<FloatSkins>();
     */
    class FloatSkins : public SkinSet {
    public:
        [[nodiscard]] std::string_view name() const override;
        [[nodiscard]] const Theme &defaultTheme() const override; // Float Light: light first, when the system doesn't say
        [[nodiscard]] const Theme &lightTheme() const override;
        [[nodiscard]] const Theme &darkTheme() const override;
        [[nodiscard]] std::unique_ptr<SkinSet> fallback() const override;
        void registerSkins(SkinRegistry &skins) const override;
        void load(StyleManager *manager) override;
    };
}
