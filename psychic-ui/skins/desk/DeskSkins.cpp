#include <algorithm>
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/skins/default/DefaultSkins.hpp"
#include "psychic-ui/themes/desk.hpp"
#include "DeskSkins.hpp"

namespace psychic_ui {
    namespace {
        constexpr Color transparent{};

        /**
         * What a finger needs to hit: targets smaller than this get hit slop up to it (WCAG 2.5.5, Apple's 44 pt)
         */
        constexpr float touchTarget = 44;

        /**
         * Desk's own part sizes, beside the theme's scale: for a pointer the board's "P2 Desk as it was", by touch
         * its approach B
         */
        struct Parts {
            float check;         // A check box's box
            float switchWidth;   // A switch's track
            float switchHeight;
            float swatch;        // A color swatch, square
            float field;         // A slider's value field
            float stepperButton; // A number stepper's − and +
            float menuRow;       // A menu's rows
            float gridHeader;    // A data grid's header row, an inspector section's band
            float grabTab;       // A slider's grab tab's length across the field (0: no tab)
            float scrollBar;     // A scroll bar's width
            float sidebarExtra;  // Added to the sidebar's width: room for the larger brand and counts
        };

        constexpr Parts pointerParts{
            .check = 14, .switchWidth = 24, .switchHeight = 14, .swatch = 16, .field = 18, .stepperButton = 22,
            .menuRow = 24, .gridHeader = 26, .grabTab = 0, .scrollBar = 10, .sidebarExtra = 0,
        };

        constexpr Parts touchParts{
            .check = 20, .switchWidth = 36, .switchHeight = 22, .swatch = 26, .field = 32, .stepperButton = 36,
            .menuRow = 40, .gridHeader = 28, .grabTab = 28, .scrollBar = 6, .sidebarExtra = 16,
        };

        /**
         * A step of the type scale on a style, in a font of the theme
         */
        Style &text(Style &style, const TextStep &step, const Font &font, const Sizes &sizes) {
            return style.fontFamily(font.name)
                        .fontSize(step.size + sizes.textStep)
                        .lineHeight(step.leading + sizes.textStep);
        }
    }

    std::string_view DeskSkins::name() const {
        return "Desk";
    }

    const Theme &DeskSkins::defaultTheme() const {
        return themes::deskDark;
    }

    const Theme &DeskSkins::lightTheme() const {
        return themes::deskLight;
    }

    const Theme &DeskSkins::darkTheme() const {
        return themes::deskDark;
    }

    // Everything Desk doesn't override comes from the default set: its skins, and its rules, loaded first
    std::unique_ptr<SkinSet> DeskSkins::fallback() const {
        return std::make_unique<DefaultSkins>();
    }

    // Desk draws nothing differently: the default skins paint it from these rules (its slider too: a value field is
    // the default slider's parts laid out differently)
    void DeskSkins::registerSkins(SkinRegistry & /*skins*/) const {}

    // Only what differs from the default set's rules, which loaded just before these: on equal weight, ours win. The
    // default rules already follow the theme and the density's sizes (text, icons and hit slop steps included).
    void DeskSkins::load(StyleManager *manager) {
        const Theme      &theme = manager->theme();
        const Palette    &p     = theme.palette;
        const Typography &t     = theme.type;
        const Metrics    &m     = theme.metrics;
        const Radii      &r     = m.radius;
        const Sizes      &s     = manager->sizes();
        const bool        touch = manager->density() == Density::Touch;
        const Parts      &parts = touch ? touchParts : pointerParts;

        const float iconMd = m.iconMd + s.iconStep;

        // By touch, hit slop takes a target of this size to 44 px (never under the theme's); none with a pointer
        const auto reach = [&](const float size) {
            return touch ? std::max(s.hitSlop, (touchTarget - size) / 2.0f) : s.hitSlop;
        };

        // Bars hold their controls with room around them: the theme's height, more when the controls grow
        const auto bar = [&](const float height) {
            return std::max(height, s.controlSm + m.space(3));
        };

        const auto rule = [manager](const char *selector) -> Style & {
            return manager->style(selector);
        };

        // region Hit slop

        rule("Button").hitSlop(reach(s.controlMd));
        for (const char *small: {"Button.small", "Button.icon-button", "ToolBar Button", "MenuBar Button"}) {
            rule(small).hitSlop(reach(s.controlSm));
        }
        rule("CheckBox").hitSlop(reach(parts.check));
        rule("Range").hitSlop(reach(parts.field));
        rule("TextInput").hitSlop(reach(s.controlMd));
        rule("MenuButton").hitSlop(reach(parts.menuRow));
        rule("Switch").hitSlop(reach(parts.switchHeight));
        rule("SegmentedControl").hitSlop(reach(s.controlMd));
        rule("ColorSwatches").hitSlop(reach(parts.swatch));
        rule("NumberStepper").hitSlop(reach(s.controlMd));
        rule("SearchField").hitSlop(reach(s.controlMd));
        rule("Select").hitSlop(reach(s.controlMd));
        rule("Chip").hitSlop(reach(s.controlSm));

        // endregion

        // region Title bar

        rule("TitleBar").height(bar(m.barHeader));

        // endregion

        // region Check box

        rule("CheckBox .box")
            .width(parts.check)
            .height(parts.check);

        // endregion

        // region Slider

        // A value field: the default slider's parts laid out as a field, the accent at 38 % inside it up to the value,
        // the value written at its end; no thumb with a pointer, a grab tab on the value by touch
        rule("Range DefaultSliderSkin.horizontal")
            .alignItems(Align::Stretch)
            .height(parts.field);

        rule("Range DefaultSliderSkin.vertical")
            .alignItems(Align::Stretch)
            .width(parts.field);

        rule("Range DefaultSliderSkin .track")
            .border(m.border)
            .borderRadius(r.control)
            .borderColor(p.line)
            .backgroundColor(p.raised)
            .overflow(Overflow::Hidden)
            .transition(Property::borderColor, m.motion.quick());
        rule("Range DefaultSliderSkin.horizontal .track").height(Auto); // The field's height, over the default's 4 px
        rule("Range DefaultSliderSkin.vertical .track").width(Auto);
        rule("Range:hover DefaultSliderSkin .track").borderColor(p.lineStrong);
        rule("Range:active DefaultSliderSkin .track").borderColor(p.lineStrong);
        rule("Range:focus-visible DefaultSliderSkin .track").borderColor(p.accent); // As the fields: no ring

        rule("Range DefaultSliderSkin .fill")
            .borderRadius(0)
            .backgroundColor(p.accent.withAlpha(0x61)); // 38%

        // By touch, a white tab across the field on the value, so the finger has something to hold
        const float tab = std::min(parts.grabTab, parts.field - 2 * m.border - 2);
        rule("Range DefaultSliderSkin .thumb")
            .display(touch ? Display::Flex : Display::None)
            .borderRadius(r.control)
            .border(m.border)
            .borderColor(p.lineStrong);
        rule("Range DefaultSliderSkin.horizontal .thumb").width(12).height(tab);
        rule("Range DefaultSliderSkin.vertical .thumb").width(tab).height(12);

        // The value inside the field, at its end
        text(rule("Range DefaultSliderSkin .value"), t.xs, t.regular, s)
            .position(Position::Absolute)
            .right(m.space(2))
            .top(0)
            .width(Auto)
            .height(parts.field)
            .lineHeight(parts.field);

        // endregion

        // region Inputs

        // Switch: smaller, 24 × 14 with a 10 px thumb 2 px in (36 × 22 by touch), a pill still
        rule("Switch .track")
            .width(parts.switchWidth)
            .height(parts.switchHeight)
            .paddingHorizontal(1)
            .borderRadius(999)
            .borderColor(p.line);
        rule("Switch .thumb")
            .width(parts.switchHeight - 4)
            .height(parts.switchHeight - 4)
            .borderRadius(999);

        // Number stepper: − and + square-ish at the ends
        rule("NumberStepper").width(s.controlMd + 2 * parts.stepperButton + 40);
        rule("NumberStepper.small").width(s.controlSm + 2 * parts.stepperButton + 32);
        for (const char *button: {"NumberStepper .decrement", "NumberStepper .increment",
                                  "NumberStepper.small .decrement", "NumberStepper.small .increment"}) {
            rule(button).width(parts.stepperButton);
        }

        // Color swatches: squares with small corners, 6 px apart, the selected one ringed closer
        rule("ColorSwatches").gap(m.space(1.5f));
        rule("ColorSwatches .swatch")
            .width(parts.swatch)
            .height(parts.swatch)
            .borderRadius(r.check);
        rule("ColorSwatches .swatch.selected")
            .outlineWidth(1.5f)
            .outlineOffset(1.5f);

        // endregion

        // region Scroll bars

        // With a pointer, a track on the surface behind a hairline, the thumb 2 px inside; by touch a thin indicator
        rule("ScrollBar.vertical").width(parts.scrollBar);
        rule("ScrollBar.horizontal").height(parts.scrollBar);
        if (touch) {
            rule("ScrollBar.vertical .thumb").marginHorizontal(1);
            rule("ScrollBar.horizontal .thumb").marginVertical(1);
        } else {
            rule("ScrollBar .track").backgroundColor(p.surface);
            rule("ScrollBar.vertical .track").borderLeft(m.border).borderColor(p.line);
            rule("ScrollBar.horizontal .track").borderTop(m.border).borderColor(p.line);
            rule("ScrollBar .thumb").borderRadius(r.check);
        }
        rule("Scroller ScrollBar.horizontal").marginRight(parts.scrollBar);

        // endregion

        // region Menus

        // Rows: 24 px and the body size with a pointer, 40 by touch
        text(rule("Menu MenuButton"), t.sm, t.regular, s)
            .minHeight(parts.menuRow)
            .paddingVertical(std::max(0.0f, (parts.menuRow - t.sm.leading - s.textStep) / 2.0f));

        // endregion

        // region Tool bar, tabs

        rule("ToolBar")
            .height(bar(m.barToolbar))
            .paddingRight(m.gutter);

        rule("Tabs").minHeight(bar(m.barToolbar));

        // endregion

        // region App shell

        // Flush panes (the theme's: no inset, no corners): the sidebar on the surface meets the page on a hairline
        rule("Sidebar")
            .width(m.sidebarWidth + parts.sidebarExtra)
            .borderRight(m.border)
            .borderColor(p.line)
            .backgroundColor(p.surface);

        rule("SidebarHeader")
            .height(bar(m.barHeader))
            .borderBottom(m.border)
            .borderColor(p.line);

        text(rule("SidebarHeader .mark"), t.xxs, t.semiBold, s)
            .width(18 + s.iconStep)
            .height(18 + s.iconStep)
            .borderRadius(r.control);

        // Sections 6 px in, rows 1 px apart
        rule("SidebarSection")
            .gap(1)
            .padding(m.space(1.5f));
        rule("SidebarSection:first-child").paddingTop(m.space(1.5f));

        // Rows: 26 px (40 by touch), 12 px text
        text(rule("SidebarItem"), t.base, t.regular, s).height(s.rowSm);

        rule("PageHeader")
            .height(bar(m.barHeader))
            .paddingRight(m.gutter);

        // endregion

        // region Lists

        // Rows without hairlines between them (P2's rows run on: the selection and hover set them apart)
        for (const char *row: {"ListRow", "DataGridRow"}) {
            rule(row).borderBottom(0);
        }

        // List rows: 26 px with a pointer, 40 by touch; two lines grow them
        rule("ListRow")
            .gap(m.space(2.5f))
            .paddingVertical(m.space(1));
        rule("List.md ListRow").paddingVertical(m.space(1));
        rule("List.sm ListRow").paddingVertical(m.space(0.5f));
        rule("ListRow .text").gap(0);

        // A data grid: its header 26 px on the surface layer, cells 12 px apart
        rule("DataGrid .header")
            .gap(m.space(3))
            .height(parts.gridHeader)
            .backgroundColor(p.surface);

        rule("DataGridRow")
            .gap(m.space(3))
            .paddingVertical(m.space(1));
        rule("DataGrid.sm DataGridRow").paddingVertical(m.space(0.5f));

        // A tree: the sidebar's rows; the selection raised, no outline, its icon in the accent
        rule("TreeRow.selected").borderColor(transparent);
        rule("TreeRow.selected .icon").color(p.accent);

        // An inspector: section headers are bands between hairlines, fields have their names right-aligned beside them
        rule("Inspector .heading").paddingVertical(m.space(2.5f));
        rule("Inspector .media")
            .width(28 + s.iconStep)
            .height(28 + s.iconStep)
            .iconSize(iconMd);

        rule("InspectorSection .header")
            .minHeight(parts.gridHeader)
            .padding(0)
            .paddingHorizontal(m.space(2.5f))
            .borderBottom(m.border)
            .borderColor(p.line)
            .backgroundColor(p.raised.withAlpha(0x8C)); // 55%
        text(rule("InspectorSection .header .title"), t.xs, t.medium, s).color(p.text);

        rule("InspectorSection .body")
            .gap(m.space(1))
            .paddingTop(m.space(2))
            .paddingBottom(m.space(2.5f));

        rule("InspectorField")
            .gap(m.space(2.5f))
            .alignItems(Align::Center)
            .minHeight(s.controlSm);
        rule("InspectorField .name")
            .width(84)
            .shrink(0)
            .textAlign(TextAlign::End);

        // endregion

        // region Badges, chips, avatars, key caps, empty states

        // Badges: 3 px corners, not pills; status dots, avatars and switches stay round
        rule("Badge").borderRadius(r.check);
        rule("StatusDot").borderRadius(999);
        rule("Avatar").borderRadius(999);

        // Chips: a small control, 22 px (36 by touch), small text
        text(rule("Chip"), t.xs, t.regular, s)
            .gap(m.space(1))
            .height(s.controlSm)
            .paddingLeft(m.space(2))
            .paddingRight(m.space(1.5f));
        rule("Chip.removable").paddingRight(m.space(1));

        // Key caps: 16 px squares
        rule("Kbd .key")
            .minWidth(16 + s.textStep)
            .height(16 + s.textStep)
            .lineHeight(14 + s.textStep);

        // Empty states: a smaller tile, closer
        rule("EmptyState").paddingVertical(m.space(12));
        rule("EmptyState .icon-tile")
            .width(32 + s.iconStep)
            .height(32 + s.iconStep)
            .iconSize(16 + s.iconStep);

        // endregion

        // region Popups

        // Toasts: an icon tile the desk's control radius
        rule("Toast .icon-tile").borderRadius(r.control);

        // endregion
    }
}
