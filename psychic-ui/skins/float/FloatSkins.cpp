#include <string>
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/themes/float.hpp"
#include "psychic-ui/skins/default/DefaultSkins.hpp"
#include "FloatSkins.hpp"

namespace psychic_ui {
    namespace {
        constexpr Color transparent{};

        /**
         * A panel floating over the canvas: the surface, rounded, lifted by the theme's two-layer panel shadow
         */
        Style &floatingPanel(Style &style, const Theme &theme) {
            const Shadow &soft = theme.elevation.panel;
            const Shadow &ring = theme.elevation.panelContact;
            return style.backgroundColor(theme.palette.surface)
                        .borderRadius(theme.metrics.radius.surface)
                        .shadowColor(soft.color)
                        .shadowBlur(soft.blur)
                        .shadowOffsetY(soft.offsetY)
                        .shadowSpread(soft.spread)
                        .shadow2Color(ring.color)
                        .shadow2Blur(ring.blur)
                        .shadow2OffsetY(ring.offsetY)
                        .shadow2Spread(ring.spread);
        }
    }

    std::string_view FloatSkins::name() const {
        return "Float";
    }

    const Theme &FloatSkins::defaultTheme() const {
        return themes::floatLight;
    }

    const Theme &FloatSkins::lightTheme() const {
        return themes::floatLight;
    }

    const Theme &FloatSkins::darkTheme() const {
        return themes::floatDark;
    }

    // Everything Float doesn't override comes from the default set: its skins, and its rules, loaded first
    std::unique_ptr<SkinSet> FloatSkins::fallback() const {
        return std::make_unique<DefaultSkins>();
    }

    // Float draws nothing differently: every default skin paints from the computed style, so rules are enough.
    // A set whose drawing differs registers its own skins here, over its fallback's, for instance:
    //     skins.add<SwitchSkin, FloatSwitchSkin>();
    void FloatSkins::registerSkins(SkinRegistry & /*skins*/) const {}

    // Only what differs from the default set's rules, which loaded just before these: on equal weight, ours win
    void FloatSkins::load(StyleManager *manager) {
        const Theme   &theme = manager->theme();
        const Palette &p     = theme.palette;
        const Metrics &m     = theme.metrics;
        const Radii   &r     = m.radius;

        const auto rule = [manager](const char *selector) -> Style & {
            return manager->style(selector);
        };

        // region The canvas and its floating panels

        // The frame is a canvas: its grey, dotted every 16 px like a design tool's
        rule("AppShell")
            .backgroundColor(p.frame)
            .dotGridColor(p.canvasDot)
            .dotGridSpacing(16)
            .dotGridRadius(1);

        // The sidebar and the page float over it, the canvas showing around them
        floatingPanel(rule("Sidebar"), theme)
            .margin(m.frameInset)
            .marginRight(0);
        rule("AppShell .main").paddingLeft(m.frameInset);
        floatingPanel(rule("AppShell .page"), theme)
            .backgroundColor(p.page)
            .borderRadius(r.page);

        // Sections set apart by their headings, not by hairlines
        rule("SidebarSection").borderTop(0);

        // The current item: the selection's soft violet, its text and icon in the accent
        rule("SidebarItem.current")
            .color(p.accentText)
            .backgroundColor(p.accentSoft);

        // endregion

        // region Controls

        // Switches stay round (the button radius isn't a pill here): a 28 × 16 track without a border
        rule("Switch .track")
            .width(28)
            .height(16)
            .border(0)
            .borderRadius(999);
        rule("Switch .thumb").borderRadius(999);

        // A white thumb on a white panel: a hairline ring under its drop shadow keeps it visible
        rule("Range .thumb")
            .shadow2Color(p.lineStrong)
            .shadow2Spread(1);

        // Round avatars and color chips, whatever the radii
        rule("Avatar").borderRadius(999);
        rule("ColorSwatches .swatch").borderRadius(999);

        // Badges: small rounded rectangles in medium weight
        rule("Badge")
            .borderRadius(r.small)
            .fontFamily(theme.type.medium.name);

        // endregion

        // region Lists

        // No hairlines between rows; the selection is soft violet, without the default set's accent bar
        for (const char *row: {"ListRow", "DataGridRow"}) {
            const std::string name{row};
            rule(row).borderBottom(0);
            rule((name + ":hover").c_str()).backgroundColor(p.hover);
            rule((name + ".selected").c_str())
                .borderLeftColor(transparent)
                .backgroundColor(p.accentSoft);
            rule((name + ".sliding.selected").c_str()).backgroundColor(p.accentSoft.over(p.page)); // Opaque, as it slides
        }
        rule("ListRow.selected .title").color(p.accentText);
        rule("DataGridRow.selected").color(p.accentText);

        rule("TreeRow.selected")
            .color(p.accentText)
            .borderColor(transparent)
            .backgroundColor(p.accentSoft);
        rule("TreeRow.selected .icon").color(p.accent);

        // endregion
    }
}
