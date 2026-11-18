#include <string>
#include <vector>
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/themes/one-dark.hpp"
#include "psychic-ui/themes/one-light.hpp"
#include "psychic-ui/themes/pivotal.hpp"
#include "psychic-ui/themes/psychic.hpp"
#include "psychic-ui/themes/ubuntu-type.hpp"
#include "psychic-ui/skins/base/ColorSwatchesSkinBase.hpp"
#include "psychic-ui/skins/base/CheckBoxSkinBase.hpp"
#include "psychic-ui/skins/base/MenuButtonSkinBase.hpp"
#include "psychic-ui/skins/base/NumberStepperSkinBase.hpp"
#include "psychic-ui/skins/base/RangeSkinBase.hpp"
#include "psychic-ui/skins/base/ScrollBarSkinBase.hpp"
#include "psychic-ui/skins/base/SearchFieldSkinBase.hpp"
#include "psychic-ui/skins/base/SegmentedControlSkinBase.hpp"
#include "psychic-ui/skins/base/SwitchSkinBase.hpp"
#include "psychic-ui/skins/base/TextAreaSkinBase.hpp"
#include "psychic-ui/skins/base/TextInputSkinBase.hpp"
#include "PivotalSkins.hpp"
#include "PivotalAvatarSkin.hpp"
#include "TabButtonSkin.hpp"
#include "TitleBarButtonSkin.hpp"
#include "psychic-ui/animation/Timing.hpp"
#include "psychic-ui/components/Avatar.hpp"
#include "psychic-ui/components/Button.hpp"
#include "psychic-ui/components/CheckBox.hpp"
#include "psychic-ui/components/ColorSwatches.hpp"
#include "psychic-ui/components/MenuButton.hpp"
#include "psychic-ui/components/NumberStepper.hpp"
#include "psychic-ui/components/Range.hpp"
#include "psychic-ui/components/ScrollBar.hpp"
#include "psychic-ui/components/SearchField.hpp"
#include "psychic-ui/components/SegmentedControl.hpp"
#include "psychic-ui/components/Switch.hpp"
#include "psychic-ui/components/TextArea.hpp"
#include "psychic-ui/components/TextInput.hpp"
#include "psychic-ui/skins/SkinSet.hpp"
#include "psychic-ui/skins/base/ButtonSkinBase.hpp"
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
         * A step of the type scale on a style, in a font of the theme
         */
        Style &text(Style &style, const TextStep &step, const Font &font, const Sizes &sizes) {
            return style.fontFamily(font.name)
                        .fontSize(step.size + sizes.textStep)
                        .lineHeight(step.leading + sizes.textStep);
        }

        /**
         * A shadow as the first layer of a style
         */
        Style &shadow(Style &style, const Shadow &shadow) {
            return style.shadowColor(shadow.color)
                        .shadowBlur(shadow.blur)
                        .shadowOffsetX(shadow.offsetX)
                        .shadowOffsetY(shadow.offsetY)
                        .shadowSpread(shadow.spread);
        }

        /**
         * Pivotal's one elevation, for everything that floats: two layers
         */
        Style &floating(Style &style, const Elevation &elevation) {
            const Shadow &contact = elevation.overlayContact;
            return shadow(style, elevation.overlay)
                .shadow2Color(contact.color)
                .shadow2Blur(contact.blur)
                .shadow2OffsetX(contact.offsetX)
                .shadow2OffsetY(contact.offsetY)
                .shadow2Spread(contact.spread);
        }

        /**
         * Pivotal's focus ring: one for every focusable thing, shown for the keyboard
         */
        Style &focusRing(Style &style, const Palette &palette, const Metrics &metrics) {
            return style.outlineColor(palette.accent)
                        .outlineWidth(metrics.focusRing)
                        .outlineOffset(metrics.focusRingOffset);
        }
    }

    std::string_view PivotalSkins::name() const {
        return "Default";
    }

    const Theme &PivotalSkins::defaultTheme() const {
        return themes::pivotalDark;
    }

    const Theme &PivotalSkins::lightTheme() const {
        return themes::pivotalLight;
    }

    std::vector<const Theme *> PivotalSkins::themes() const {
        return {&themes::pivotalDark, &themes::pivotalLight, &themes::oneDark, &themes::oneLight, &themes::psychic};
    }

    void PivotalSkins::registerSkins(SkinRegistry &skins) const {
        // The bases draw from the rules; the variants are classes on them (`Button ButtonSkinBase.primary`)
        skins.add<ButtonSkin, ButtonSkinBase>()
             .add<ButtonSkin, ButtonSkinBase>("primary")
             .add<ButtonSkin, ButtonSkinBase>("ghost")
             .add<ButtonSkin, ButtonSkinBase>("quiet")
             .add<ButtonSkin, ButtonSkinBase>("danger")
             .add<ButtonSkin, TabButtonSkin>("tab")
             .add<ButtonSkin, TitleBarButtonSkin>("title-bar")
             .add<CheckBoxSkin, CheckBoxSkinBase>()
             .add<RangeSkin, RangeSkinBase>()
             .add<ScrollBarSkin, ScrollBarSkinBase>()
             .add<TextInputSkin, TextInputSkinBase>()
             .add<TextInputSkin, TextInputSkinBase>("bare")
             .add<TextAreaSkin, TextAreaSkinBase>()
             .add<MenuButtonSkin, MenuButtonSkinBase>();

        // Lists
        skins.add<AvatarSkin, PivotalAvatarSkin>();

        // Inputs
        skins.add<SwitchSkin, SwitchSkinBase>()
             .add<SegmentedControlSkin, SegmentedControlSkinBase>()
             .add<ColorSwatchesSkin, ColorSwatchesSkinBase>()
             .add<NumberStepperSkin, NumberStepperSkinBase>()
             .add<SearchFieldSkin, SearchFieldSkinBase>();
    }

    void PivotalSkins::load(StyleManager *manager) {
        const Theme      &theme   = manager->theme();
        const Palette    &palette = theme.palette;
        const Typography &t       = theme.type;
        const Metrics    &metrics = theme.metrics;
        const Radii      &r       = metrics.radius;
        const Sizes      &s       = manager->sizes();
        const Motion     &motion  = metrics.motion;

        // Icons, a step larger where the density says so (`Sizes::iconStep`)
        const float iconSm = metrics.iconSm + s.iconStep;
        const float iconMd = metrics.iconMd + s.iconStep;

        // Motion, Pivotal's: colors blend quickly on hover, press, focus and selection; the focus ring fades in (and
        // goes at once: its width snaps to 0); a switch's thumb springs. What paints the colors transitions them: a
        // component's skin (which inherits the component's colors), rows, parts with colors of their own.
        using P = Property;
        const Timing quick = motion.quick();
        const auto   colors = {P::color, P::backgroundColor, P::borderColor, P::outlineColor};

        // Inter (Pivotal's) and Ubuntu are always there, so any style can name them; then the theme's own
        for (const Typography &type: {Typography{}, themes::ubuntuType, t}) {
            for (const Font &font: {type.regular, type.medium, type.semiBold}) {
                manager->loadFont(std::string{font.name}, std::string{font.path});
            }
        }

        const auto rule = [manager](const char *selector) -> Style & {
            return manager->style(selector);
        };

        // region Defaults

        // Body text: 12 px on 18, primary text; the selection is the accent, softly
        text(rule("*"), t.sm, t.regular, s)
            .overflow(Overflow::Visible)
            .color(palette.text)
            .selectionColor(palette.text)
            .selectionBackgroundColor(palette.accentSoft)
            .iconSize(iconMd)
            .iconStrokeWidth(metrics.iconStroke)
            .antiAlias(true)
            .transition(P::outlineColor, quick); // Every focus ring fades in

        // Three levels of text, by descending importance: titles, values and what was chosen; body text, labels and
        // controls at rest; metadata and placeholders. Classes, as the headings are
        rule(".text-primary").color(palette.text);
        rule(".text-secondary").color(palette.textSecondary);
        rule(".text-tertiary").color(palette.textMuted);

        // Headings: Pivotal's titles are medium weight, small; only the largest is semibold
        text(rule(".h1"), t.xxl, t.semiBold, s).letterSpacing(-0.2f);
        text(rule(".h2"), t.xl, t.medium, s);
        text(rule(".h3"), t.md, t.medium, s);
        text(rule(".h4"), t.base, t.medium, s);
        text(rule(".h5"), t.sm, t.medium, s);
        text(rule(".h6"), t.xs, t.medium, s).color(palette.textMuted);

        // Fingers reach the controls from around them (none with a pointer); they look the same
        for (const char *control: {"Button", "CheckBox", "Range", "ScrollBar", "TextInput", "TextArea", "MenuButton"}) {
            rule(control).hitSlop(s.hitSlop);
        }

        // Skins paint their components: they blend the colors they inherit
        rule("Skin").transition(colors, quick);

        // endregion

        // region Window

        rule("Window")
            .color(palette.text)
            .backgroundColor(palette.base)
            .border(metrics.border)
            .borderColor(palette.line);

        // endregion

        // region Title bar and window controls

        rule("TitleBar")
            .alignItems(Align::Center)
            .gap(metrics.space(2.5f))
            .height(metrics.barHeader)
            .paddingLeft(metrics.space(3.5f))
            .paddingRight(metrics.space(2.5f))
            .backgroundColor(palette.frame)
            .borderBottom(metrics.border)
            .borderColor(palette.line);

        text(rule("TitleBar Label.title"), t.md, t.semiBold, s)
            .letterSpacing(-0.14f) // -0.01em
            .color(palette.text)
            .grow(1);

        // The three dots: neutral at rest, all three in the theme's semantic colors while the pointer is over the group
        rule("WindowControls").gap(7);

        rule("WindowControls Button")
            .skin("title-bar")
            .width(11)
            .height(11)
            .paddingHorizontal(0) // Over the button's
            .border(0)
            .backgroundColor(palette.lineStrong)
            .transition({P::backgroundColor, P::outlineColor}, quick); // Its skin paints the component's color

        rule("WindowControls:hover Button.close").backgroundColor(palette.danger);
        rule("WindowControls:hover Button.minimize").backgroundColor(palette.warning);
        rule("WindowControls:hover Button.maximize").backgroundColor(palette.success);

        // endregion

        // region Buttons

        // The secondary button, Pivotal's default: a raised pill in a hairline
        text(rule("Button"), t.sm, t.medium, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .gap(metrics.space(1.5f))
            .height(s.controlMd)
            .paddingHorizontal(metrics.space(2.5f))
            .border(metrics.border)
            .borderRadius(r.button)
            .color(palette.text)
            .backgroundColor(palette.raised)
            .borderColor(palette.line)
            .iconSize(iconSm);

        rule("Button:hover").borderColor(palette.lineStrong);
        // Pressed, or selected (a toggle): held as a hover, filled with the strong hairline's color in its outline
        rule("Button:active")
            .borderColor(palette.lineStrong)
            .backgroundColor(palette.lineStrong);
        rule("Button:disabled").opacity(0.45f);
        focusRing(rule("Button:focus-visible"), palette, metrics);

        // Smaller, for tool bars and fields' sides (Pivotal's sm)
        text(rule("Button.small"), t.xs, t.medium, s)
            .height(s.controlSm)
            .paddingHorizontal(metrics.space(2));

        // An icon alone, on a square
        rule("Button.icon-button")
            .width(s.controlSm)
            .height(s.controlSm)
            .paddingHorizontal(0) // Over the button's
            .iconSize(iconMd)
            .color(palette.textMuted)
            .backgroundColor(transparent)
            .borderColor(transparent);
        rule("Button.icon-button:hover").color(palette.text).backgroundColor(palette.raised);
        rule("Button.icon-button:active").color(palette.text).backgroundColor(palette.raised);

        // The variants, on the skin (`button.skin("primary")`)
        rule("Button ButtonSkinBase.primary")
            .color(palette.onAccent)
            .backgroundColor(palette.accent)
            .borderColor(transparent);
        rule("Button:hover ButtonSkinBase.primary").backgroundColor(palette.accentHover);
        rule("Button:active ButtonSkinBase.primary").backgroundColor(palette.accentHover);

        rule("Button ButtonSkinBase.ghost")
            .color(palette.textSecondary)
            .backgroundColor(transparent)
            .borderColor(transparent);
        rule("Button:hover ButtonSkinBase.ghost").backgroundColor(palette.raised);
        rule("Button:active ButtonSkinBase.ghost").color(palette.text).backgroundColor(palette.raised);

        rule("Button ButtonSkinBase.quiet")
            .color(palette.textMuted)
            .backgroundColor(transparent)
            .borderColor(transparent);
        rule("Button:hover ButtonSkinBase.quiet").color(palette.text).backgroundColor(palette.raised);
        rule("Button:active ButtonSkinBase.quiet").color(palette.text).backgroundColor(palette.raised);

        rule("Button ButtonSkinBase.danger")
            .color(palette.dangerText)
            .backgroundColor(transparent)
            .borderColor(palette.danger.withAlpha(0x4D)); // 30%
        rule("Button:hover ButtonSkinBase.danger").backgroundColor(palette.danger.withAlpha(0x14)); // 8%
        rule("Button:active ButtonSkinBase.danger").backgroundColor(palette.danger.withAlpha(0x14));

        // endregion

        // region Check box

        text(rule("CheckBox"), t.sm, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(2))
            .color(palette.text);

        rule("CheckBox .box")
            .width(14)
            .height(14)
            .shrink(0)
            .border(metrics.border)
            .borderRadius(r.check)
            .borderColor(palette.lineStrong)
            .backgroundColor(palette.page)
            .color(palette.onAccent) // The check mark
            .iconStrokeWidth(1.8f)
            .transition(colors, quick);

        rule("CheckBox:hover .box").borderColor(palette.textSecondary);
        rule("CheckBox.checked .box")
            .borderColor(palette.accent)
            .backgroundColor(palette.accent);
        rule("CheckBox:disabled").opacity(0.5f);
        focusRing(rule("CheckBox:focus-visible .box"), palette, metrics);

        // endregion

        // region Slider

        // A thin track filled with the accent, a white thumb, the value after it
        rule("Range RangeSkinBase.horizontal")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(2.5f))
            .height(s.controlSm);

        rule("Range RangeSkinBase.vertical")
            .direction(Direction::Column)
            .alignItems(Align::Center)
            .width(s.controlSm);

        rule("Range .track")
            .grow(1)
            .borderRadius(r.button)
            .backgroundColor(palette.lineStrong);
        rule("Range RangeSkinBase.horizontal .track").height(4);
        rule("Range RangeSkinBase.vertical .track").width(4);

        rule("Range .fill")
            .borderRadius(r.button)
            .backgroundColor(palette.accent);

        // A step by the keyboard glides; the thumb follows a drag (or a press) at once
        rule("Range .fill").transition({P::width, P::height}, motion.arrive());
        rule("Range .thumb-anchor").transition({P::left, P::bottom}, motion.arrive());
        rule("Range:active .fill").transition(Transition{});
        rule("Range:active .thumb-anchor").transition(Transition{});

        shadow(rule("Range .thumb"), theme.elevation.thumb)
            .width(14)
            .height(14)
            .borderRadius(r.button)
            .backgroundColor(palette.onAccent);
        focusRing(rule("Range:focus-visible .thumb"), palette, metrics);

        text(rule("Range .value"), t.xs, t.regular, s)
            .width(34)
            .shrink(0)
            .textAlign(TextAlign::End)
            .color(palette.text);
        rule("Range RangeSkinBase.vertical .value").display(Display::None); // No room

        rule("Range:disabled").opacity(0.5f);

        // endregion

        // region Text fields

        // A raised field in a hairline; focus shows as the accent's border, not a ring
        for (const char *field: {"TextInput", "TextArea"}) {
            text(rule(field), t.base, t.regular, s)
                .paddingHorizontal(metrics.space(2.5f))
                .border(metrics.border)
                .borderRadius(r.control)
                .borderColor(palette.line)
                .backgroundColor(palette.raised)
                .color(palette.text)
                .cursor(Cursor::IBeam)
                .overflow(Overflow::Hidden);
        }
        rule("TextInput")
            .height(s.controlMd)
            .justify(Justify::Center);
        rule("TextArea")
            .minHeight(s.controlMd * 2)
            .paddingVertical(metrics.space(1.5f));

        rule("TextInput:focus").borderColor(palette.accent);
        rule("TextArea:focus").borderColor(palette.accent);
        rule("TextInput:disabled").opacity(0.5f);
        rule("TextArea:disabled").opacity(0.5f);

        // The selection only shows while its field has the focus (its text inherits the colors)
        for (const char *field: {"TextInput", "TextArea", "SearchField", "NumberStepper"}) {
            rule(field).selectionBackgroundColor(transparent);
        }
        for (const char *focused: {"TextInput:focus", "TextArea:focus", "SearchField:focus", "NumberStepper:focus"}) {
            rule(focused).selectionBackgroundColor(palette.accentSoft);
        }

        // Bare (`skin("bare")`): no chrome and no padding, for read-only text one can select
        rule("TextInputSkinBase.bare")
            .padding(0)
            .border(0)
            .borderRadius(0)
            .backgroundColor(transparent);

        // endregion

        // region Inputs

        // Fingers reach them from around them (none with a pointer), like the other controls
        for (const char *control: {"Switch", "SegmentedControl", "ColorSwatches", "NumberStepper", "SearchField"}) {
            rule(control).hitSlop(s.hitSlop);
        }

        rule("TextInput .placeholder").color(palette.textMuted);

        // Search field: a text input's field, a muted search icon before the text, a clear button after it
        text(rule("SearchField"), t.base, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(2))
            .height(s.controlMd)
            .paddingLeft(metrics.space(2.5f))
            .paddingRight(metrics.space(1))
            .border(metrics.border)
            .borderRadius(r.control)
            .borderColor(palette.line)
            .backgroundColor(palette.raised)
            .color(palette.text)
            .cursor(Cursor::IBeam);
        rule("SearchField:focus").borderColor(palette.accent);
        rule("SearchField:disabled").opacity(0.5f);

        rule("SearchField .icon")
            .shrink(0)
            .color(palette.textMuted);

        // The text, and its placeholder at its start, across the room the icon and the clear button leave
        rule("SearchField .field")
            .grow(1)
            .shrink(1)
            .alignSelf(Align::Stretch)
            .justify(Justify::Center)
            .overflow(Overflow::Hidden);
        rule("SearchField .placeholder").color(palette.textMuted);

        rule("SearchField .clear")
            .cursor(Cursor::Arrow)
            .width(s.controlXs)
            .height(s.controlXs)
            .shrink(0)
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .borderRadius(r.small)
            .iconSize(iconSm)
            .color(palette.textMuted);
        rule("SearchField .clear:hover")
            .color(palette.text)
            .backgroundColor(palette.line);

        // Switch: a pill track in a strong hairline, the accent when on, a white thumb sliding 2 px inside it
        text(rule("Switch"), t.sm, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(2))
            .color(palette.textSecondary);

        rule("Switch .track")
            .width(30)
            .height(18)
            .paddingHorizontal(2)
            .border(metrics.border)
            .borderRadius(r.button)
            .borderColor(palette.lineStrong)
            .backgroundColor(palette.switchOff)
            .transition(colors, motion.moderate());
        rule("Switch.checked .track")
            .borderColor(palette.accent)
            .backgroundColor(palette.accent);
        focusRing(rule("Switch:focus-visible .track"), palette, metrics);

        shadow(rule("Switch .thumb"), theme.elevation.thumb)
            .width(12)
            .height(12)
            .borderRadius(r.button)
            .backgroundColor(palette.onAccent);

        // The thumb springs to its side, keeping its speed when flipped again half-way; it follows a drag at once
        rule("Switch .thumb-space").transition(P::grow, Timing{motion.spring});
        rule("Switch.dragging .thumb-space").transition(Transition{});

        rule("Switch:disabled").opacity(0.45f);

        // Segmented control: a pill on the surface in a hairline; the selected segment raised, nested 1 px in
        text(rule("SegmentedControl"), t.xs, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Stretch)
            .height(s.controlMd)
            .padding(1)
            .border(metrics.border)
            .borderRadius(r.button)
            .borderColor(palette.line)
            .backgroundColor(palette.surface)
            .color(palette.textMuted)
            .iconSize(iconSm);
        rule("SegmentedControl.small").height(s.controlSm);
        rule("SegmentedControl:disabled").opacity(0.45f);

        rule("SegmentedControl .segment")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(1.5f))
            .paddingHorizontal(metrics.space(2))
            .borderRadius(r.button - 1)
            .transition(colors, quick);
        rule("SegmentedControl.small .segment").paddingHorizontal(metrics.space(1.5f));
        rule("SegmentedControl .segment:hover").color(palette.text);
        text(rule("SegmentedControl .segment.selected"), t.xs, t.medium, s)
            .color(palette.text)
            .backgroundColor(palette.raised);
        // Inside the segment, like a sidebar item's: outside it would cross the group's edge
        focusRing(rule("SegmentedControl:focus-visible .segment.selected"), palette, metrics).outlineOffset(-metrics.focusRing);

        // Number stepper: a raised field four controls wide, − and + at its ends behind hairlines
        text(rule("NumberStepper"), t.base, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Stretch)
            .width(s.controlMd * 4)
            .height(s.controlMd)
            .border(metrics.border)
            .borderRadius(r.control)
            .borderColor(palette.line)
            .backgroundColor(palette.raised)
            .color(palette.text);
        text(rule("NumberStepper.small"), t.sm, t.regular, s)
            .width(s.controlSm * 4)
            .height(s.controlSm);
        rule("NumberStepper:focus").borderColor(palette.accent);
        rule("NumberStepper:disabled").opacity(0.5f);

        // The text between the buttons, centered
        rule("NumberStepper .value")
            .grow(1)
            .shrink(1)
            .minWidth(0)
            .alignSelf(Align::Center)
            .paddingHorizontal(metrics.space(1))
            .textAlign(TextAlign::Center)
            .cursor(Cursor::IBeam);

        for (const char *button: {"NumberStepper .decrement", "NumberStepper .increment"}) {
            rule(button)
                .cursor(Cursor::Arrow)
                .width(s.controlMd)
                .shrink(0)
                .alignItems(Align::Center)
                .justify(Justify::Center)
                .borderColor(palette.line)
                .iconSize(iconSm)
                .color(palette.textSecondary);
        }
        rule("NumberStepper.small .decrement").width(s.controlSm);
        rule("NumberStepper.small .increment").width(s.controlSm);
        rule("NumberStepper .decrement")
            .borderRight(metrics.border)
            .borderRadiusLeft(r.control - metrics.border);
        rule("NumberStepper .increment")
            .borderLeft(metrics.border)
            .borderRadiusRight(r.control - metrics.border);
        for (const char *hover: {"NumberStepper .decrement:hover", "NumberStepper .increment:hover"}) {
            rule(hover)
                .color(palette.text)
                .backgroundColor(palette.line);
        }
        // At a bound: faded, and no hover
        for (const char *bound: {"NumberStepper .decrement:disabled", "NumberStepper .increment:disabled"}) {
            rule(bound)
                .color(palette.textMuted)
                .backgroundColor(transparent);
        }

        // Color swatches: 20 px chips 4 px apart, wrapping; the selected one ringed in the accent, 2 px out; the one
        // the keyboard is on ringed in a neutral grey while the focus is visible (the accent when it is the selected)
        rule("ColorSwatches")
            .direction(Direction::Row)
            .wrap(Wrap::Wrap)
            .gap(metrics.space(1));

        rule("ColorSwatches .swatch")
            .width(s.controlXs)
            .height(s.controlXs)
            .shrink(0)
            .borderRadius(r.button)
            .transition(P::outlineColor, quick);
        rule("ColorSwatches .swatch.selected")
            .outlineColor(palette.accent)
            .outlineWidth(2)
            .outlineOffset(2);
        rule("ColorSwatches:focus-visible .swatch.focused")
            .outlineColor(palette.textMuted)
            .outlineWidth(metrics.focusRing)
            .outlineOffset(2);
        rule("ColorSwatches:focus-visible .swatch.focused.selected")
            .outlineColor(palette.accent);
        rule("ColorSwatches:disabled").opacity(0.45f);

        // endregion

        // region Scroll bars

        // Thin and self-effacing: a translucent pill, inset in a transparent track
        rule("ScrollBar.vertical").width(metrics.scrollBar);
        rule("ScrollBar.horizontal").height(metrics.scrollBar);

        rule("ScrollBar .thumb")
            .borderRadius(r.button)
            .backgroundColor(palette.scrollThumb)
            .transition(P::backgroundColor, quick);
        rule("ScrollBar.vertical .thumb")
            .marginHorizontal(metrics.scrollThumbInset)
            .minHeight(s.controlSm);
        rule("ScrollBar.horizontal .thumb")
            .marginVertical(metrics.scrollThumbInset)
            .minWidth(s.controlSm);

        rule("ScrollBar:hover .thumb").backgroundColor(palette.scrollThumbHover);
        rule("ScrollBar:active .thumb").backgroundColor(palette.scrollThumbHover);
        rule("ScrollBar:disabled .thumb").visible(false);

        rule("Scroller ScrollBar.horizontal").marginRight(metrics.scrollBar);

        // endregion

        // region Menus

        // The floating surface: overlay color, strong hairline, the popup radius, the one shadow
        floating(rule("Menu"), theme.elevation)
            .backgroundColor(palette.overlay)
            .border(metrics.border)
            .borderColor(palette.lineStrong)
            .borderRadius(r.popup)
            .padding(metrics.space(1))
            .minWidth(metrics.menuWidth)
            .alignItems(Align::Stretch)
            .color(palette.text);

        // Rows: 32 px, 13 px text, raised grey while highlighted
        text(rule("Menu MenuButton"), t.base, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(2))
            .minHeight(s.rowSm)
            .paddingVertical(metrics.space(1.5f))
            .paddingHorizontal(metrics.space(2))
            .borderRadius(r.row)
            .color(palette.textSecondary)
            .backgroundColor(transparent);

        rule("Menu MenuButton .icon").color(palette.textMuted).transition(P::color, quick);
        rule("Menu MenuButton .label").grow(1);

        text(rule("Menu MenuButton .shortcut"), t.xs, t.regular, s)
            .paddingLeft(metrics.space(4))
            .color(palette.textMuted);

        rule("Menu MenuButton .arrow")
            .iconSize(iconSm)
            .marginLeft(metrics.space(2))
            .color(palette.textMuted);

        // Highlighted (the menu's one row, by the mouse or the keyboard)
        rule("Menu MenuButton:active")
            .color(palette.text)
            .backgroundColor(palette.raised);
        rule("Menu MenuButton:active .icon").color(palette.text);

        // Check marks keep their column, in the primary color; a select's after the label
        rule("Menu MenuButton .check")
            .shrink(0)
            .color(palette.text);
        rule("Menu MenuButton .indicator")
            .shrink(0)
            .marginLeft(metrics.space(2))
            .color(palette.text);
        rule("Menu.listbox MenuButton.checked").color(palette.text);

        rule("Menu MenuButton:disabled").opacity(0.45f);

        // Actions that destroy, in the danger color, softly tinted while highlighted
        rule("Menu MenuButton.danger").color(palette.dangerText);
        rule("Menu MenuButton.danger .icon").color(palette.dangerText);
        rule("Menu MenuButton.danger:active")
            .color(palette.dangerText)
            .backgroundColor(palette.danger.withAlpha(0x14)); // 8%
        rule("Menu MenuButton.danger:active .icon").color(palette.dangerText);

        // A hairline across the menu, its padding included
        rule("Menu .separator")
            .height(metrics.border)
            .shrink(0)
            .marginVertical(metrics.space(1))
            .marginHorizontal(-metrics.space(1))
            .backgroundColor(palette.line);

        text(rule("Menu .group-label"), t.xs, t.medium, s)
            .paddingTop(metrics.space(1.5f))
            .paddingHorizontal(metrics.space(2))
            .paddingBottom(metrics.space(1))
            .color(palette.textMuted);

        // endregion

        // region Select

        // A closed field like a text input: the value, the chevron at the end; the accent's border while open
        text(rule("Select"), t.base, t.regular, s)
            .gap(metrics.space(2))
            .height(s.controlMd)
            .paddingLeft(metrics.space(2.5f))
            .paddingRight(metrics.space(2))
            .border(metrics.border)
            .borderRadius(r.control)
            .borderColor(palette.line)
            .backgroundColor(palette.raised)
            .color(palette.text)
            .hitSlop(s.hitSlop)
            .transition(colors, quick);

        rule("Select:hover").borderColor(palette.lineStrong);
        rule("Select:focus-visible").borderColor(palette.accent);
        rule("Select.open").borderColor(palette.accent);
        rule("Select:disabled").opacity(0.5f);
        rule("Select .value.placeholder").color(palette.textMuted);
        rule("Select .chevron").color(palette.textMuted);

        // endregion

        // region Tooltips

        // A small label on the overlay color, in a strong hairline, with an arrow
        floating(text(rule("Tooltip"), t.xs, t.regular, s), theme.elevation)
            .paddingVertical(metrics.space(1))
            .paddingHorizontal(metrics.space(2))
            .maxWidth(metrics.popoverWidth)
            .border(metrics.border)
            .borderColor(palette.lineStrong)
            .borderRadius(r.row)
            .backgroundColor(palette.overlay)
            .color(palette.text);

        // endregion

        // region Dialogs

        // The scrim over the page, the panel on it: the floating surface, a header, a body, a footer after a hairline
        rule("Dialog").backgroundColor(palette.scrim);

        floating(rule("Dialog .panel"), theme.elevation)
            .width(metrics.dialogSm)
            .maxWidth(100_pct)
            .maxHeight(80_pct)
            .border(metrics.border)
            .borderColor(palette.lineStrong)
            .borderRadius(r.popup)
            .backgroundColor(palette.overlay)
            .color(palette.text)
            .overflow(Overflow::Hidden);
        rule("Dialog.wide .panel").width(metrics.dialogMd);

        rule("Dialog .header")
            .gap(metrics.space(1.5f))
            .paddingTop(metrics.space(3))
            .paddingHorizontal(metrics.gutterCard);
        text(rule("Dialog .header .title"), t.sm, t.regular, s).color(palette.text);
        rule("Dialog .header .icon").color(palette.textSecondary);

        rule("Dialog .body")
            .gap(metrics.space(3))
            .padding(metrics.gutterCard);

        text(rule("Dialog .description"), t.sm, t.regular, s)
            .lineHeight((t.sm.size * 1.625f) + s.textStep)
            .color(palette.textMuted);

        rule("Dialog .footer")
            .gap(metrics.space(1.5f))
            .paddingVertical(metrics.space(3))
            .paddingHorizontal(metrics.gutterCard)
            .borderTop(metrics.border)
            .borderColor(palette.line);

        // endregion

        // region Toasts

        // Stacked in the bottom right corner, the newest at the bottom
        rule("ToastStack")
            .right(metrics.viewportMargin)
            .bottom(metrics.viewportMargin)
            .width(metrics.toastWidth)
            .gap(metrics.space(2));

        // The floating surface: an icon on a tile in the variant's color, the title and description, the actions
        floating(text(rule("Toast"), t.sm, t.regular, s), theme.elevation)
            .gap(metrics.space(2.5f))
            .padding(metrics.space(3))
            .border(metrics.border)
            .borderColor(palette.lineStrong)
            .borderRadius(r.popup)
            .backgroundColor(palette.overlay)
            .color(palette.text);

        rule("Toast .icon-tile")
            .width(s.controlSm)
            .height(s.controlSm)
            .borderRadius(r.row)
            .color(palette.accentText)
            .backgroundColor(palette.accentSoft);
        rule("Toast.success .icon-tile").color(palette.success).backgroundColor(palette.success.withAlpha(0x1A)); // 10%
        rule("Toast.warning .icon-tile").color(palette.warningText).backgroundColor(palette.warning.withAlpha(0x1A));
        rule("Toast.danger .icon-tile").color(palette.dangerText).backgroundColor(palette.danger.withAlpha(0x1A));

        rule("Toast .text").gap(metrics.space(0.5f));
        text(rule("Toast .title"), t.sm, t.medium, s).color(palette.text);
        rule("Toast .description").color(palette.textSecondary);

        rule("Toast .actions")
            .gap(metrics.space(1.5f))
            .marginTop(metrics.space(1.5f));
        text(rule("Toast .actions Button"), t.xs, t.medium, s)
            .height(s.controlXs)
            .paddingHorizontal(metrics.space(2));

        rule("Toast Button.close")
            .width(s.controlXs)
            .height(s.controlXs)
            .marginTop(-metrics.space(0.5f))
            .marginRight(-metrics.space(0.5f));

        // endregion

        // region Entering and leaving

        // Pivotal's floating motion: what floats fades and scales from 97% about its anchor (`Popup` puts its
        // transform origin there) as it opens, and back as it closes, a little quicker. Popups have the `entering`
        // class for their first style and `exiting` while they leave, so these rules are where they come from and go.
        const Timing enter    = motion.enter();
        const Timing leave    = motion.leave();
        const auto   floating = {P::opacity, P::scale, P::translateX, P::translateY};

        rule("Menu").transition(floating, enter).transition(P::outlineColor, quick);
        rule("Menu.entering").opacity(0).scale(0.97f);
        rule("Menu.exiting").opacity(0).scale(0.97f).transition(floating, leave);

        // A tooltip fades in from a few pixels toward its anchor, and out the same way
        const float nudge = metrics.space(1);
        rule("Tooltip").transition(floating, enter);
        rule("Tooltip.exiting").opacity(0).transition(floating, leave);
        rule("Tooltip.entering").opacity(0);
        rule("Tooltip.side-top.entering").translateY(nudge);
        rule("Tooltip.side-top.exiting").translateY(nudge);
        rule("Tooltip.side-bottom.entering").translateY(-nudge);
        rule("Tooltip.side-bottom.exiting").translateY(-nudge);
        rule("Tooltip.side-left.entering").translateX(nudge);
        rule("Tooltip.side-left.exiting").translateX(nudge);
        rule("Tooltip.side-right.entering").translateX(-nudge);
        rule("Tooltip.side-right.exiting").translateX(-nudge);

        // A dialog's scrim fades (and its panel with it), the panel scales from 97% about its middle
        rule("Dialog").transition(P::opacity, enter);
        rule("Dialog .panel").transition(P::scale, enter);
        rule("Dialog.entering").opacity(0);
        rule("Dialog.entering .panel").scale(0.97f);
        rule("Dialog.exiting").opacity(0).transition(P::opacity, leave);
        rule("Dialog.exiting .panel").scale(0.97f).transition(P::scale, leave);

        // A toast slides in from under the stack's edge, and back out fading; one with another under it (dismissed
        // from the middle, or pushed out by a new one, the stack's `limit`) only fades, where it is. The others slide
        // into its place meanwhile (it left the stack's flow).
        rule("Toast").transition(floating, enter);
        rule("Toast.entering").opacity(0).translateY(150_pct);
        rule("Toast.exiting").opacity(0).translateY(150_pct).transition(floating, Timing{motion.medium, motion.out});
        rule("Toast.exiting.in-place").translateY(0);

        // endregion

        // region Layout transitions

        // What a new layout moves slides from where it was painted (FLIP, a paint offset): the toasts as one comes or
        // goes, rows when a list or a grid is sorted, the tree's rows below a node that expands or collapses, as its
        // children fade in (they go at once), the sidebar's sections and items. A grid's rows travel furthest, across
        // the whole grid as it is sorted: at the slow step
        const Timing slide = motion.arrive();
        rule("ToastStack").layoutTransition(slide);
        rule("List").layoutTransition(slide);
        rule("DataGrid .body").layoutTransition(Timing{motion.slow, motion.out});
        rule("Tree").layoutTransition(slide);
        rule("Tree .children").layoutTransition(slide).opacity(0).transition(P::opacity, slide);
        rule("Tree .children.open").opacity(1);
        rule("Sidebar .sections").layoutTransition(slide);
        rule("SidebarSection").layoutTransition(slide);

        // endregion

        // region Menu bar

        // Its menus' triggers: ghost pills
        rule("MenuBar")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(0.5f));

        text(rule("MenuBar Button"), t.sm, t.medium, s)
            .skin("ghost")
            .height(s.controlSm)
            .paddingHorizontal(metrics.space(2));

        // endregion

        // region Tool bar

        // A strip under a header: Pivotal's toolbar band, with quiet small buttons
        rule("ToolBar")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(1.5f))
            .height(metrics.barToolbar)
            .paddingLeft(metrics.gutter)
            .paddingRight(metrics.space(3))
            .borderBottom(metrics.border)
            .borderColor(palette.line);

        text(rule("ToolBar Button"), t.xs, t.medium, s)
            .skin("quiet")
            .height(s.controlSm)
            .paddingHorizontal(metrics.space(2));

        // The current tool (a toggled button), like the primary action, in the accent
        rule("ToolBar Button:active ButtonSkinBase.quiet")
            .color(palette.onAccent)
            .backgroundColor(palette.accent);

        // endregion

        // region Tabs

        // Tabs on a hairline: the selected one underlined in the text color, never the accent
        rule("Tabs")
            .direction(Direction::Row)
            .alignItems(Align::End)
            .gap(metrics.space(1))
            .minHeight(metrics.barToolbar)
            .paddingHorizontal(metrics.gutter)
            .borderBottom(metrics.border)
            .borderColor(palette.line);

        text(rule("Tabs Button"), t.base, t.medium, s)
            .skin("tab")
            .height(s.rowMd)
            .marginBottom(-metrics.border) // Over the line, so the underline covers it
            .paddingHorizontal(metrics.space(2.5f))
            .border(0)
            .borderRadius(0)
            .color(palette.textMuted)
            .backgroundColor(transparent)
            .borderColor(transparent);

        rule("Tabs Button:hover")
            .color(palette.text)
            .borderColor(palette.lineStrong);

        rule("Tabs Button:active")
            .color(palette.text)
            .borderColor(palette.text);

        rule("TabContainer").backgroundColor(palette.page);

        // endregion

        // region App shell

        // The frame: the sidebar's color around the page panel, inset in it
        rule("AppShell").backgroundColor(palette.frame);

        rule("AppShell .main")
            .paddingTop(metrics.frameInset)
            .paddingRight(metrics.frameInset)
            .paddingBottom(metrics.frameInset);

        rule("AppShell .page")
            .backgroundColor(palette.page)
            .borderRadius(r.page)
            .overflow(Overflow::Hidden);

        rule("Sidebar")
            .width(metrics.sidebarWidth)
            .backgroundColor(palette.frame);

        // The header: the window controls, then the app's mark and name, lit like the rows
        rule("SidebarHeader")
            .gap(metrics.space(2.5f))
            .height(metrics.barHeader)
            .paddingLeft(metrics.space(3.5f))
            .paddingRight(metrics.space(2.5f));

        rule("SidebarHeader .brand")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(2))
            .grow(1)
            .shrink(1)
            .height(s.controlMd)
            .paddingHorizontal(metrics.space(1.5f))
            .borderRadius(r.row);

        text(rule("SidebarHeader .mark"), t.xs, t.semiBold, s)
            .width(20)
            .height(20)
            .shrink(0)
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .borderRadius(r.small)
            .backgroundColor(palette.accent)
            .color(palette.onAccent);

        text(rule("SidebarHeader .title"), t.md, t.semiBold, s)
            .letterSpacing(-0.14f) // -0.01em
            .shrink(1)
            .color(palette.text);

        rule("SidebarHeader .chevron")
            .iconSize(iconSm)
            .color(palette.textMuted);

        // Sections, set apart by a hairline
        rule("SidebarSection")
            .gap(metrics.space(0.5f))
            .padding(metrics.space(2))
            .paddingTop(metrics.space(1.5f))
            .borderTop(metrics.border)
            .borderColor(palette.line);

        rule("SidebarSection:first-child")
            .paddingTop(metrics.space(2))
            .borderTop(0);

        text(rule("SidebarSection .heading"), t.xs, t.medium, s)
            .paddingTop(metrics.space(1))
            .paddingHorizontal(metrics.space(2))
            .paddingBottom(metrics.space(0.5f))
            .color(palette.textMuted);

        // Rows: 32 px, 13 px text in the secondary color, the current one raised with an accent icon
        text(rule("SidebarItem"), t.base, t.regular, s)
            .gap(metrics.space(2))
            .height(s.rowSm)
            .paddingHorizontal(metrics.space(2))
            .borderRadius(r.row)
            .color(palette.textSecondary)
            .transition(colors, quick);

        rule("SidebarItem .icon").color(palette.textMuted).transition(P::color, quick);
        rule("SidebarItem .label").grow(1).shrink(1);
        text(rule("SidebarItem .count"), t.xs, t.regular, s).color(palette.textMuted);

        rule("SidebarItem:hover")
            .color(palette.text)
            .backgroundColor(palette.hover);

        rule("SidebarItem.current")
            .color(palette.text)
            .backgroundColor(palette.raised);
        rule("SidebarItem.current .icon").color(palette.accent);
        focusRing(rule("SidebarItem:focus-visible"), palette, metrics).outlineOffset(-metrics.focusRing);

        // A page's title bar: the sidebar header's height, so that their lines meet
        rule("PageHeader")
            .gap(metrics.space(2))
            .height(metrics.barHeader)
            .paddingLeft(metrics.gutter)
            .paddingRight(metrics.space(3))
            .borderBottom(metrics.border)
            .borderColor(palette.line);

        rule("PageHeader .icon").color(palette.textMuted);
        text(rule("PageHeader .title"), t.md, t.medium, s).color(palette.text);

        // endregion

        // region Lists

        // Shared by the rows of lists and grids: raised under the mouse, the selection raised with an accent bar at
        // its start (a 2 px left border, transparent until then, so that nothing moves), the focus ring inside.
        // Sliding to a new place (a sort), a row hides the rows it crosses: opaque at once (no blend) in the page's
        // color, which lists sit on, so that it looks the same; the selection keeps its fill, made opaque
        const auto selectableRow = [&](const std::string &row) {
            rule(row.c_str())
                .paddingLeft(metrics.gutter - 2)
                .paddingRight(metrics.gutter)
                .borderLeft(2)
                .borderBottom(metrics.border)
                .borderColor(palette.line)
                .borderLeftColor(transparent)
                .color(palette.text)
                .transition({P::color, P::backgroundColor, P::borderLeftColor, P::outlineColor}, quick);
            rule((row + ":last-child").c_str()).borderBottom(0);
            rule((row + ":hover").c_str()).backgroundColor(palette.raised);
            rule((row + ".sliding").c_str())
                .backgroundColor(palette.page)
                .transition({P::color, P::borderLeftColor, P::outlineColor}, quick);
            rule((row + ".selected").c_str())
                .backgroundColor(palette.raised)
                .borderLeftColor(palette.accent);
            rule((row + ".sliding.selected").c_str()).backgroundColor(palette.raised.over(palette.page));
            rule((row + ":disabled").c_str()).opacity(0.45f);
            focusRing(rule((row + ":focus-visible").c_str()), palette, metrics).outlineOffset(-metrics.focusRing);
        };

        // Rows edge to edge on the page's gutter, between hairlines: 48 px (Pivotal's default), `.md` 40, `.sm` 32
        rule("List")
            .borderTop(metrics.border)
            .borderColor(palette.line);

        selectableRow("ListRow");
        text(rule("ListRow"), t.sm, t.regular, s)
            .gap(metrics.space(3))
            .minHeight(s.rowMd + metrics.space(2))
            .paddingVertical(metrics.space(2));
        rule("List.md ListRow")
            .minHeight(s.rowMd)
            .paddingVertical(metrics.space(1.5f));
        rule("List.sm ListRow")
            .minHeight(s.rowSm)
            .paddingVertical(metrics.space(1));

        rule("ListRow .leading")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .shrink(0)
            .color(palette.textMuted);
        rule("ListRow .text").gap(metrics.space(1));
        rule("ListRow .title").color(palette.text);
        text(rule("ListRow .secondary"), t.xs, t.regular, s).color(palette.textMuted);
        rule("ListRow .trailing")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(metrics.space(1.5f))
            .shrink(0);
        text(rule("ListRow .meta"), t.xs, t.regular, s).color(palette.textMuted);

        // A data grid: a header row a tool bar's height in small muted text, then rows of cells 16 px apart
        text(rule("DataGrid .header"), t.xs, t.regular, s)
            .gap(metrics.space(4))
            .height(metrics.barToolbar)
            .paddingHorizontal(metrics.gutter)
            .borderBottom(metrics.border)
            .borderColor(palette.line)
            .color(palette.textMuted);

        rule("DataGrid .header .sort").gap(metrics.space(1));
        rule("DataGrid .header .sort")
            .paddingHorizontal(metrics.space(1)) // Room for the focus ring, taken back from the cell
            .marginHorizontal(-metrics.space(1))
            .cursor(Cursor::Hand)
            .borderRadius(r.small);
        rule("DataGrid .header .sort:hover").color(palette.textSecondary);
        rule("DataGrid .header .sorted").color(palette.text);
        rule("DataGrid .header .sorted:hover").color(palette.text);
        focusRing(rule("DataGrid .header .sort:focus-visible"), palette, metrics);

        // The arrow: hidden, faint (the next order) under the mouse or the keyboard's focus, plain when sorted
        rule("DataGrid .sort-icon")
            .iconSize(iconSm)
            .opacity(0);
        rule("DataGrid .sort:hover .sort-icon").opacity(0.6f);
        rule("DataGrid .sort:focus-visible .sort-icon").opacity(0.6f);
        rule("DataGrid .sorted .sort-icon").opacity(1);

        selectableRow("DataGridRow");
        text(rule("DataGridRow"), t.sm, t.regular, s)
            .gap(metrics.space(4))
            .minHeight(s.rowMd)
            .paddingVertical(metrics.space(1.5f));
        rule("DataGrid.sm DataGridRow")
            .minHeight(s.rowSm)
            .paddingVertical(metrics.space(1));
        rule("DataGridRow .cell").gap(metrics.space(1.5f));
        rule("DataGridRow .cell.end").color(palette.textSecondary);

        rule("DataGrid ScrollBar.vertical").marginVertical(metrics.space(0.5f));

        // A tree: the sidebar's rows (32 px, 13 px text), indented a step per level; the selection raised in a
        // strong hairline, as Pivotal's
        rule("Tree").gap(1);

        // A node sliding down or up (one above it expands or collapses) hides what it passes over, the children fading
        // in: opaque in the page's color, under its row's own fill
        rule("TreeNode.sliding").backgroundColor(palette.page);

        text(rule("TreeRow"), t.base, t.regular, s)
            .gap(metrics.space(1.5f))
            .height(s.rowSm)
            .paddingLeft(metrics.space(1))
            .paddingRight(metrics.space(2))
            .border(metrics.border)
            .borderRadius(r.row)
            .borderColor(transparent)
            .color(palette.textSecondary)
            .transition(colors, quick);

        rule("TreeRow:hover")
            .color(palette.text)
            .backgroundColor(palette.hover);
        rule("TreeRow.selected")
            .color(palette.text)
            .backgroundColor(palette.raised)
            .borderColor(palette.lineStrong);
        rule("TreeRow:disabled").opacity(0.45f);
        focusRing(rule("TreeRow:focus-visible"), palette, metrics).outlineOffset(-metrics.focusRing);

        // The chevron's column is an icon wide, kept on leaves
        rule("TreeRow .disclosure")
            .width(iconMd)
            .iconSize(iconSm)
            .color(palette.textMuted);
        rule("TreeRow .icon").color(palette.textMuted);
        rule("TreeRow.selected .icon").color(palette.text);
        text(rule("TreeRow .trailing"), t.xs, t.regular, s).color(palette.textMuted);

        // An inspector: a panel on the surface layer, sections between hairlines, fields in two columns
        text(rule("Inspector"), t.sm, t.regular, s)
            .backgroundColor(palette.surface)
            .borderLeft(metrics.border)
            .borderColor(palette.line);

        rule("Inspector .heading")
            .gap(metrics.space(2.5f))
            .paddingVertical(metrics.space(3))
            .paddingHorizontal(metrics.gutter)
            .borderBottom(metrics.border)
            .borderColor(palette.line);
        rule("Inspector .media")
            .width(32)
            .height(32)
            .borderRadius(r.control)
            .backgroundColor(palette.raised)
            .color(palette.textSecondary)
            .iconSize(16);
        rule("Inspector .titles").gap(metrics.space(0.5f));
        text(rule("Inspector .heading .title"), t.md, t.medium, s).color(palette.text);
        text(rule("Inspector .heading .subtitle"), t.sm, t.regular, s).color(palette.textMuted);

        rule("InspectorSection")
            .borderBottom(metrics.border)
            .borderColor(palette.line);
        rule("InspectorSection:last-child").borderBottom(0);

        rule("InspectorSection .header")
            .gap(metrics.space(2))
            .minHeight(s.controlMd)
            .paddingTop(metrics.space(3))
            .paddingHorizontal(metrics.gutter)
            .paddingBottom(metrics.space(1));
        text(rule("InspectorSection .header .title"), t.sm, t.medium, s).color(palette.textSecondary);

        rule("InspectorSection .body")
            .gap(metrics.space(2))
            .paddingTop(metrics.space(1))
            .paddingHorizontal(metrics.gutter)
            .paddingBottom(metrics.space(4));

        rule("InspectorField").gap(metrics.space(3));
        rule("InspectorField .name")
            .width(104)
            .color(palette.textMuted);
        rule("InspectorField .value").gap(metrics.space(1.5f));
        rule("InspectorField .text").color(palette.text);

        // endregion

        // region Badges, status dots, chips

        // Pills outlined in their tone, the border its color at a third or so
        const Color accentBorder  = palette.accent.withAlpha(0x73);  // 45%
        const Color successBorder = palette.success.withAlpha(0x59); // 35%
        const Color warningBorder = palette.warning.withAlpha(0x59); // 35%
        const Color dangerBorder  = palette.danger.withAlpha(0x4D);  // 30%

        text(rule("Badge"), t.xs, t.regular, s)
            .gap(metrics.space(1))
            .height(s.controlXs)
            .paddingHorizontal(metrics.space(2))
            .border(metrics.border)
            .borderRadius(r.button)
            .color(palette.textSecondary)
            .borderColor(palette.lineStrong);
        text(rule("Badge.small"), t.xxs, t.regular, s)
            .height(16)
            .paddingHorizontal(metrics.space(1.5f));
        rule("Badge.accent").color(palette.accentText).borderColor(accentBorder);
        rule("Badge.success").color(palette.success).borderColor(successBorder);
        rule("Badge.warning").color(palette.warningText).borderColor(warningBorder);
        rule("Badge.danger").color(palette.dangerText).borderColor(dangerBorder);

        rule("StatusDot")
            .width(6)
            .height(6)
            .borderRadius(r.button)
            .backgroundColor(palette.textMuted);
        rule("StatusDot.accent").backgroundColor(palette.accent);
        rule("StatusDot.success").backgroundColor(palette.success);
        rule("StatusDot.warning").backgroundColor(palette.warning);
        rule("StatusDot.danger").backgroundColor(palette.danger);

        // Quiet pills: secondary text in a hairline, raised under the mouse or when selected
        text(rule("Chip"), t.sm, t.regular, s)
            .gap(metrics.space(1.5f))
            .height(s.controlMd)
            .paddingHorizontal(metrics.space(2.5f))
            .border(metrics.border)
            .borderRadius(r.button)
            .borderColor(palette.line)
            .color(palette.textSecondary)
            .hitSlop(s.hitSlop)
            .transition(colors, quick);
        rule("Chip.removable").paddingRight(metrics.space(1.5f));
        for (const char *state: {"Chip:hover", "Chip.selected"}) {
            rule(state)
                .color(palette.text)
                .backgroundColor(palette.raised)
                .borderColor(palette.lineStrong);
        }
        rule("Chip:disabled").opacity(0.45f);
        focusRing(rule("Chip:focus-visible"), palette, metrics);
        rule("Chip .icon").iconSize(iconSm);
        rule("Chip .remove")
            .iconSize(iconSm)
            .color(palette.textMuted)
            .cursor(Cursor::Hand);
        rule("Chip .remove:hover").color(palette.text);

        // Toned chips: their text and outline tinted, softly filled when chosen or pointed at
        rule("Chip.accent").color(palette.accentText).borderColor(accentBorder);
        rule("Chip.success").color(palette.success).borderColor(successBorder);
        rule("Chip.warning").color(palette.warningText).borderColor(warningBorder);
        rule("Chip.danger").color(palette.dangerText).borderColor(dangerBorder);
        for (const char *state: {"Chip.accent:hover", "Chip.accent.selected"}) {
            rule(state).backgroundColor(palette.accentSoft);
        }
        for (const char *state: {"Chip.success:hover", "Chip.success.selected"}) {
            rule(state).backgroundColor(palette.success.withAlpha(0x1A)); // 10%
        }
        for (const char *state: {"Chip.warning:hover", "Chip.warning.selected"}) {
            rule(state).backgroundColor(palette.warning.withAlpha(0x14)); // 8%
        }
        for (const char *state: {"Chip.danger:hover", "Chip.danger.selected"}) {
            rule(state).backgroundColor(palette.danger.withAlpha(0x14)); // 8%
        }

        // endregion

        // region Avatars, keyboard hints, empty states

        // Initials on the raised layer in a hairline, medium weight; 16, 20, 24 (the default) or 32 px
        rule("Avatar")
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .border(metrics.border)
            .borderRadius(r.button)
            .borderColor(palette.line)
            .backgroundColor(palette.raised)
            .color(palette.textSecondary);
        rule("Avatar.xs").width(16).height(16);
        rule("Avatar.sm").width(s.controlXs).height(s.controlXs);
        rule("Avatar.md").width(s.controlSm).height(s.controlSm);
        rule("Avatar.lg").width(32).height(32);
        rule("Avatar .initials").textAlign(TextAlign::Center);
        rule("Avatar.xs .initials")
            .fontFamily(t.medium.name)
            .fontSize(8 + s.textStep)
            .lineHeight(10 + s.textStep);
        text(rule("Avatar.sm .initials"), t.xxs, t.medium, s);
        text(rule("Avatar.md .initials"), t.xs, t.medium, s);
        text(rule("Avatar.lg .initials"), t.sm, t.medium, s);

        // The status at the bottom right, ringed in the page's color so it reads over the circle
        rule("Avatar .status")
            .width(8)
            .height(8)
            .right(-1)
            .bottom(-1)
            .border(1.5f)
            .borderColor(palette.page);
        rule("Avatar.lg .status")
            .width(10)
            .height(10);

        // Key caps: tiny text on the surface layer in a strong hairline
        rule("Kbd").gap(metrics.space(0.5f));
        text(rule("Kbd .key"), t.xxs, t.regular, s)
            .minWidth(18)
            .height(18)
            .lineHeight(16 + s.textStep) // The cap's inside, so that the key sits in its middle
            .paddingHorizontal(metrics.space(1))
            .border(metrics.border)
            .borderRadius(r.small)
            .borderColor(palette.lineStrong)
            .backgroundColor(palette.surface)
            .color(palette.textSecondary)
            .textAlign(TextAlign::Center);

        // Nothing to show: centered, an icon tile, a medium title, muted text, the way forward
        rule("EmptyState")
            .gap(metrics.space(1.5f))
            .paddingVertical(80)
            .paddingHorizontal(metrics.space(6));
        rule("EmptyState .icon-tile")
            .width(40)
            .height(40)
            .marginBottom(metrics.space(1.5f))
            .border(metrics.border)
            .borderRadius(r.control)
            .borderColor(palette.line)
            .backgroundColor(palette.raised)
            .color(palette.textSecondary)
            .iconSize(20);
        text(rule("EmptyState .title"), t.md, t.medium, s).color(palette.text);
        text(rule("EmptyState .description"), t.sm, t.regular, s)
            .lineHeight(19.5f + s.textStep) // 1.625
            .maxWidth(384)
            .textAlign(TextAlign::Center)
            .color(palette.textMuted);
        rule("EmptyState .action").marginTop(metrics.space(2.5f));

        // endregion
    }
}
