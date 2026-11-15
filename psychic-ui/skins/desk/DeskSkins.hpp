#pragma once

#include "psychic-ui/skins/SkinSet.hpp"

namespace psychic_ui {
    /**
     * P2 Desk: Pivotal as a dense desktop tool that is friendly with touch screens, with Desk Dark and Desk Light
     * (`themes/desk.hpp`). Flush panes on hairlines, 4 px corners, 26 px rows, right-aligned inspector labels,
     * value-field sliders, square swatches. Its sizes follow the input mode (the style manager's density): the board's
     * desk sizes for a pointer, 40 px rows and controls by touch, and hit slop taking every target to 44 px.
     *
     * Like Float, it falls back on `DefaultSkins` for every skin and rule, and only overrides the rules whose look
     * differs (the slider's value field too: rules over `DefaultSliderSkin`'s parts).
     *
     *     window.skinSet<DeskSkins>();
     */
    class DeskSkins : public SkinSet {
    public:
        std::string_view name() const override;
        const Theme &defaultTheme() const override; // Desk Dark, when the system doesn't say
        const Theme &lightTheme() const override;
        const Theme &darkTheme() const override;
        std::unique_ptr<SkinSet> fallback() const override;
        void registerSkins(SkinRegistry &skins) const override;
        void load(StyleManager *manager) override;
    };
}
