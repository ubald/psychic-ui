#include <algorithm>
#include <memory>
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/skins/pivotal/PivotalSkins.hpp"
#include "psychic-ui/themes/desk.hpp"
#include "DeskSkins.hpp"
#include "psychic-ui/skins/SkinSet.hpp"
#include "psychic-ui/style/Color.hpp"
#include "psychic-ui/style/Length.hpp"
#include "psychic-ui/style/Properties.hpp"
#include "psychic-ui/style/Style.hpp"
#include "psychic-ui/style/Values.hpp"
#include "psychic-ui/themes/Theme.hpp"

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
        return std::make_unique<PivotalSkins>();
    }

    // Desk draws nothing differently: the default skins paint it from these rules (its slider too: a value field is
    // the default slider's parts laid out differently)
    void DeskSkins::registerSkins(SkinRegistry & /*skins*/) const {}

    // Only what differs from the default set's rules, which loaded just before these: on equal weight, ours win. The
    // default rules already follow the theme and the density's sizes (text, icons and hit slop steps included).
    void DeskSkins::load(StyleManager *manager) {
        const Theme      &theme   = manager->theme();
        const Palette    &palette = theme.palette;
        const Typography &t       = theme.type;
        const Metrics    &metrics = theme.metrics;
        const Radii      &r       = metrics.radius;
        const Sizes      &s       = manager->sizes();
        const bool        touch   = manager->density() == Density::Touch;
        const Parts      &parts   = touch ? touchParts : pointerParts;

        const float iconMd = metrics.iconMd + s.iconStep;

        // By touch, hit slop takes a target of this size to 44 px (never under the theme's); none with a pointer
        const auto reach = [&](const float size) {
            return touch ? std::max(s.hitSlop, (touchTarget - size) / 2.0f) : s.hitSlop;
        };

        // Bars hold their controls with room around them: the theme's height, more when the controls grow
        const auto bar = [&](const float height) {
            return std::max(height, s.controlSm + metrics.space(3));
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

        rule("TitleBar").height(bar(metrics.barHeader));

        // endregion

        // region Check box

        rule("CheckBox .box")
            .width(parts.check)
            .height(parts.check);

        // endregion

        // region Slider

        // A value field: the default slider's parts laid out as a field, the accent at 38 % inside it up to the value,
        // the value written at its end; no thumb with a pointer, a grab tab on the value by touch
        rule("Range RangeSkinBase.horizontal")
            .alignItems(Align::Stretch)
            .height(parts.field);

        rule("Range RangeSkinBase.vertical")
            .alignItems(Align::Stretch)
            .width(parts.field);

        rule("Range RangeSkinBase .track")
            .border(metrics.border)
            .borderRadius(r.control)
            .borderColor(palette.line)
            .backgroundColor(palette.raised)
            .overflow(Overflow::Hidden)
            .transition(Property::borderColor, metrics.motion.quick());
        rule("Range RangeSkinBase.horizontal .track").height(Auto); // The field's height, over the default's 4 px
        rule("Range RangeSkinBase.vertical .track").width(Auto);
        rule("Range:hover RangeSkinBase .track").borderColor(palette.lineStrong);
        rule("Range:active RangeSkinBase .track").borderColor(palette.lineStrong);
        rule("Range:focus-visible RangeSkinBase .track").borderColor(palette.accent); // As the fields: no ring

        rule("Range RangeSkinBase .fill")
            .borderRadius(0)
            .backgroundColor(palette.accent.withAlpha(0x61)); // 38%

        // By touch, a white tab across the field on the value, so the finger has something to hold
        const float tab = std::min(parts.grabTab, parts.field - (2 * metrics.border) - 2);
        rule("Range RangeSkinBase .thumb")
            .display(touch ? Display::Flex : Display::None)
            .borderRadius(r.control)
            .border(metrics.border)
            .borderColor(palette.lineStrong);
        rule("Range RangeSkinBase.horizontal .thumb").width(12).height(tab);
        rule("Range RangeSkinBase.vertical .thumb").width(tab).height(12);

        // The value inside the field, at its end
        text(rule("Range RangeSkinBase .value"), t.xs, t.regular, s)
            .position(Position::Absolute)
            .right(metrics.space(2))
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
            .borderColor(palette.line);
        rule("Switch .thumb")
            .width(parts.switchHeight - 4)
            .height(parts.switchHeight - 4)
            .borderRadius(999);

        // Number stepper: − and + square-ish at the ends
        rule("NumberStepper").width(s.controlMd + (2 * parts.stepperButton) + 40);
        rule("NumberStepper.small").width(s.controlSm + (2 * parts.stepperButton) + 32);
        for (const char *button: {"NumberStepper .decrement", "NumberStepper .increment",
                                  "NumberStepper.small .decrement", "NumberStepper.small .increment"}) {
            rule(button).width(parts.stepperButton);
        }

        // Color swatches: squares with small corners, 6 px apart, the selected one ringed closer
        rule("ColorSwatches").gap(metrics.space(1.5f));
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
            rule("ScrollBar .track").backgroundColor(palette.surface);
            rule("ScrollBar.vertical .track").borderLeft(metrics.border).borderColor(palette.line);
            rule("ScrollBar.horizontal .track").borderTop(metrics.border).borderColor(palette.line);
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
            .height(bar(metrics.barToolbar))
            .paddingRight(metrics.gutter);

        rule("Tabs").minHeight(bar(metrics.barToolbar));

        // endregion

        // region App shell

        // Flush panes (the theme's: no inset, no corners): the sidebar on the surface meets the page on a hairline
        rule("Sidebar")
            .width(metrics.sidebarWidth + parts.sidebarExtra)
            .borderRight(metrics.border)
            .borderColor(palette.line)
            .backgroundColor(palette.surface);

        rule("SidebarHeader")
            .height(bar(metrics.barHeader))
            .borderBottom(metrics.border)
            .borderColor(palette.line);

        text(rule("SidebarHeader .mark"), t.xxs, t.semiBold, s)
            .width(18 + s.iconStep)
            .height(18 + s.iconStep)
            .borderRadius(r.control);

        // Sections 6 px in, rows 1 px apart
        rule("SidebarSection")
            .gap(1)
            .padding(metrics.space(1.5f));
        rule("SidebarSection:first-child").paddingTop(metrics.space(1.5f));

        // Rows: 26 px (40 by touch), 12 px text
        text(rule("SidebarItem"), t.base, t.regular, s).height(s.rowSm);

        rule("PageHeader")
            .height(bar(metrics.barHeader))
            .paddingRight(metrics.gutter);

        // endregion

        // region Lists

        // Rows without hairlines between them (P2's rows run on: the selection and hover set them apart)
        for (const char *row: {"ListRow", "DataGridRow"}) {
            rule(row).borderBottom(0);
        }

        // List rows: 26 px with a pointer, 40 by touch; two lines grow them
        rule("ListRow")
            .gap(metrics.space(2.5f))
            .paddingVertical(metrics.space(1));
        rule("List.md ListRow").paddingVertical(metrics.space(1));
        rule("List.sm ListRow").paddingVertical(metrics.space(0.5f));
        rule("ListRow .text").gap(0);

        // A data grid: its header 26 px on the surface layer, cells 12 px apart
        rule("DataGrid .header")
            .gap(metrics.space(3))
            .height(parts.gridHeader)
            .backgroundColor(palette.surface);

        rule("DataGridRow")
            .gap(metrics.space(3))
            .paddingVertical(metrics.space(1));
        rule("DataGrid.sm DataGridRow").paddingVertical(metrics.space(0.5f));

        // A tree: the sidebar's rows; the selection raised, no outline, its icon in the accent
        rule("TreeRow.selected").borderColor(transparent);
        rule("TreeRow.selected .icon").color(palette.accent);

        // An inspector: section headers are bands between hairlines, fields have their names right-aligned beside them
        rule("Inspector .heading").paddingVertical(metrics.space(2.5f));
        rule("Inspector .media")
            .width(28 + s.iconStep)
            .height(28 + s.iconStep)
            .iconSize(iconMd);

        rule("InspectorSection .header")
            .minHeight(parts.gridHeader)
            .padding(0)
            .paddingHorizontal(metrics.space(2.5f))
            .borderBottom(metrics.border)
            .borderColor(palette.line)
            .backgroundColor(palette.raised.withAlpha(0x8C)); // 55%
        text(rule("InspectorSection .header .title"), t.xs, t.medium, s).color(palette.text);

        rule("InspectorSection .body")
            .gap(metrics.space(1))
            .paddingTop(metrics.space(2))
            .paddingBottom(metrics.space(2.5f));

        rule("InspectorField")
            .gap(metrics.space(2.5f))
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
            .gap(metrics.space(1))
            .height(s.controlSm)
            .paddingLeft(metrics.space(2))
            .paddingRight(metrics.space(1.5f));
        rule("Chip.removable").paddingRight(metrics.space(1));

        // Key caps: 16 px squares
        rule("Kbd .key")
            .minWidth(16 + s.textStep)
            .height(16 + s.textStep)
            .lineHeight(14 + s.textStep);

        // Empty states: a smaller tile, closer
        rule("EmptyState").paddingVertical(metrics.space(12));
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
