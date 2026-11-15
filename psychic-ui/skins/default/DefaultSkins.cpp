#include <string>
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/themes/one-dark.hpp"
#include "psychic-ui/themes/one-light.hpp"
#include "psychic-ui/themes/pivotal.hpp"
#include "psychic-ui/themes/psychic.hpp"
#include "psychic-ui/themes/ubuntu-type.hpp"
#include "DefaultSkins.hpp"
#include "DefaultBasicButtonSkin.hpp"
#include "DefaultButtonSkin.hpp"
#include "DefaultAvatarSkin.hpp"
#include "DefaultCheckBoxSkin.hpp"
#include "DefaultInputSkins.hpp"
#include "DefaultMenuButtonSkin.hpp"
#include "DefaultScrollBarSkin.hpp"
#include "DefaultSliderSkin.hpp"
#include "DefaultTextAreaSkin.hpp"
#include "DefaultTextInputSkin.hpp"
#include "TitleBarButtonSkin.hpp"

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
        Style &focusRing(Style &style, const Palette &p, const Metrics &m) {
            return style.outlineColor(p.accent)
                        .outlineWidth(m.focusRing)
                        .outlineOffset(m.focusRingOffset);
        }
    }

    std::string_view DefaultSkins::name() const {
        return "Default";
    }

    const Theme &DefaultSkins::defaultTheme() const {
        return themes::pivotalDark;
    }

    const Theme &DefaultSkins::lightTheme() const {
        return themes::pivotalLight;
    }

    std::vector<const Theme *> DefaultSkins::themes() const {
        return {&themes::pivotalDark, &themes::pivotalLight, &themes::oneDark, &themes::oneLight, &themes::psychic};
    }

    void DefaultSkins::registerSkins(SkinRegistry &skins) const {
        skins.add<ButtonSkin, DefaultButtonSkin>()
             .add<ButtonSkin, DefaultButtonSkin>("secondary")
             .add<ButtonSkin, DefaultPrimaryButtonSkin>("primary")
             .add<ButtonSkin, DefaultGhostButtonSkin>("ghost")
             .add<ButtonSkin, DefaultQuietButtonSkin>("quiet")
             .add<ButtonSkin, DefaultDangerButtonSkin>("danger")
             .add<ButtonSkin, DefaultTabButtonSkin>("tab")
             .add<ButtonSkin, DefaultBasicButtonSkin>("basic")
             .add<ButtonSkin, TitleBarButtonSkin>("title-bar")
             .add<CheckBoxSkin, DefaultCheckBoxSkin>()
             .add<RangeSkin, DefaultSliderSkin>()
             .add<ScrollBarSkin, DefaultScrollBarSkin>()
             .add<TextInputSkin, DefaultTextInputSkin>()
             .add<TextAreaSkin, DefaultTextAreaSkin>()
             .add<MenuButtonSkin, DefaultMenuButtonSkin>()
             .add<MenuButtonSkin, DefaultSubMenuButtonSkin>("submenu");

        // Lists
        skins.add<AvatarSkin, DefaultAvatarSkin>();

        // Inputs
        skins.add<SwitchSkin, DefaultSwitchSkin>()
             .add<SegmentedControlSkin, DefaultSegmentedControlSkin>()
             .add<ColorSwatchesSkin, DefaultColorSwatchesSkin>()
             .add<NumberStepperSkin, DefaultNumberStepperSkin>()
             .add<SearchFieldSkin, DefaultSearchFieldSkin>();
    }

    void DefaultSkins::load(StyleManager *manager) {
        const Theme      &theme = manager->theme();
        const Palette    &p     = theme.palette;
        const Typography &t     = theme.type;
        const Metrics    &m     = theme.metrics;
        const Radii      &r     = m.radius;
        const Sizes      &s     = manager->sizes();
        const Motion     &mo    = m.motion;

        // Motion, Pivotal's: colors blend quickly on hover, press, focus and selection; the focus ring fades in (and
        // goes at once: its width snaps to 0); a switch's thumb springs. What paints the colors transitions them: a
        // component's skin (which inherits the component's colors), rows, parts with colors of their own.
        using P = Property;
        const Timing quick = mo.quick();
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
            .color(p.text)
            .selectionColor(p.text)
            .selectionBackgroundColor(p.accentSoft)
            .iconSize(m.iconMd)
            .iconStrokeWidth(m.iconStroke)
            .antiAlias(true)
            .textAntiAlias(true)
            .transition(P::outlineColor, quick); // Every focus ring fades in

        // Three levels of text, by descending importance (TextRole)
        rule(".text-primary").color(p.text);
        rule(".text-secondary").color(p.textSecondary);
        rule(".text-tertiary").color(p.textMuted);

        // Headings: Pivotal's titles are medium weight, small; only the largest is semibold
        text(rule(".h1"), t.xxl, t.semiBold, s).letterSpacing(-0.2f);
        text(rule(".h2"), t.xl, t.medium, s);
        text(rule(".h3"), t.md, t.medium, s);
        text(rule(".h4"), t.base, t.medium, s);
        text(rule(".h5"), t.sm, t.medium, s);
        text(rule(".h6"), t.xs, t.medium, s).color(p.textMuted);

        // Fingers reach the controls from around them (none with a pointer); they look the same
        for (const char *control: {"Button", "CheckBox", "Range", "ScrollBar", "TextInput", "TextArea", "MenuButton"}) {
            rule(control).hitSlop(s.hitSlop);
        }

        // Skins paint their components: they blend the colors they inherit
        rule("Skin").transition(colors, quick);

        // endregion

        // region Window

        rule("Window")
            .color(p.text)
            .backgroundColor(p.base)
            .border(m.border)
            .borderColor(p.line);

        // endregion

        // region Title bar and window controls

        rule("TitleBar")
            .alignItems(Align::Center)
            .gap(m.space(2.5f))
            .height(m.barHeader)
            .paddingLeft(m.space(3.5f))
            .paddingRight(m.space(2.5f))
            .backgroundColor(p.frame)
            .borderBottom(m.border)
            .borderColor(p.line);

        text(rule("TitleBar Label.title"), t.md, t.semiBold, s)
            .letterSpacing(-0.14f) // -0.01em
            .color(p.text)
            .grow(1);

        // The three dots: neutral at rest, all three in the theme's semantic colors while the pointer is over the group
        rule("WindowControls").gap(7);

        rule("WindowControls Button")
            .skin("title-bar")
            .width(11)
            .height(11)
            .paddingHorizontal(0) // Over the button's
            .border(0)
            .backgroundColor(p.lineStrong)
            .transition({P::backgroundColor, P::outlineColor}, quick); // Its skin paints the component's color

        rule("WindowControls:hover Button.close").backgroundColor(p.danger);
        rule("WindowControls:hover Button.minimize").backgroundColor(p.warning);
        rule("WindowControls:hover Button.maximize").backgroundColor(p.success);

        // endregion

        // region Buttons

        // The secondary button, Pivotal's default: a raised pill in a hairline
        text(rule("Button"), t.sm, t.medium, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .gap(m.space(1.5f))
            .height(s.controlMd)
            .paddingHorizontal(m.space(2.5f))
            .border(m.border)
            .borderRadius(r.button)
            .color(p.text)
            .backgroundColor(p.raised)
            .borderColor(p.line)
            .iconSize(m.iconSm);

        rule("Button:hover").borderColor(p.lineStrong);
        // Pressed, or selected (a toggle): held as a hover, filled with the strong hairline's color in its outline
        rule("Button:active")
            .borderColor(p.lineStrong)
            .backgroundColor(p.lineStrong);
        rule("Button:disabled").opacity(0.45f);
        focusRing(rule("Button:focus-visible"), p, m);

        // Smaller, for tool bars and fields' sides (Pivotal's sm)
        text(rule("Button.small"), t.xs, t.medium, s)
            .height(s.controlSm)
            .paddingHorizontal(m.space(2));

        // An icon alone, on a square
        rule("Button.icon-button")
            .width(s.controlSm)
            .height(s.controlSm)
            .paddingHorizontal(0) // Over the button's
            .iconSize(m.iconMd)
            .color(p.textMuted)
            .backgroundColor(transparent)
            .borderColor(transparent);
        rule("Button.icon-button:hover").color(p.text).backgroundColor(p.raised);
        rule("Button.icon-button:active").color(p.text).backgroundColor(p.raised);

        // The variants, on the skin (`button.skin("primary")`)
        rule("Button DefaultButtonSkin.primary")
            .color(p.onAccent)
            .backgroundColor(p.accent)
            .borderColor(transparent);
        rule("Button:hover DefaultButtonSkin.primary").backgroundColor(p.accentHover);
        rule("Button:active DefaultButtonSkin.primary").backgroundColor(p.accentHover);

        rule("Button DefaultButtonSkin.ghost")
            .color(p.textSecondary)
            .backgroundColor(transparent)
            .borderColor(transparent);
        rule("Button:hover DefaultButtonSkin.ghost").backgroundColor(p.raised);
        rule("Button:active DefaultButtonSkin.ghost").color(p.text).backgroundColor(p.raised);

        rule("Button DefaultButtonSkin.quiet")
            .color(p.textMuted)
            .backgroundColor(transparent)
            .borderColor(transparent);
        rule("Button:hover DefaultButtonSkin.quiet").color(p.text).backgroundColor(p.raised);
        rule("Button:active DefaultButtonSkin.quiet").color(p.text).backgroundColor(p.raised);

        rule("Button DefaultButtonSkin.danger")
            .color(p.dangerText)
            .backgroundColor(transparent)
            .borderColor(p.danger.withAlpha(0x4D)); // 30%
        rule("Button:hover DefaultButtonSkin.danger").backgroundColor(p.danger.withAlpha(0x14)); // 8%
        rule("Button:active DefaultButtonSkin.danger").backgroundColor(p.danger.withAlpha(0x14));

        // endregion

        // region Check box

        text(rule("CheckBox"), t.sm, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(2))
            .color(p.text);

        rule("CheckBox .box")
            .width(14)
            .height(14)
            .shrink(0)
            .border(m.border)
            .borderRadius(r.check)
            .borderColor(p.lineStrong)
            .backgroundColor(p.page)
            .color(p.onAccent) // The check mark
            .iconStrokeWidth(1.8f)
            .transition(colors, quick);

        rule("CheckBox:hover .box").borderColor(p.textSecondary);
        rule("CheckBox.checked .box")
            .borderColor(p.accent)
            .backgroundColor(p.accent);
        rule("CheckBox:disabled").opacity(0.5f);
        focusRing(rule("CheckBox:focus-visible .box"), p, m);

        // endregion

        // region Slider

        // A thin track filled with the accent, a white thumb, the value after it
        rule("Range DefaultSliderSkin.horizontal")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(2.5f))
            .height(s.controlSm);

        rule("Range DefaultSliderSkin.vertical")
            .direction(Direction::Column)
            .alignItems(Align::Center)
            .width(s.controlSm);

        rule("Range .track")
            .grow(1)
            .borderRadius(r.button)
            .backgroundColor(p.lineStrong);
        rule("Range DefaultSliderSkin.horizontal .track").height(4);
        rule("Range DefaultSliderSkin.vertical .track").width(4);

        rule("Range .fill")
            .borderRadius(r.button)
            .backgroundColor(p.accent);

        // A step by the keyboard glides; the thumb follows a drag (or a press) at once
        rule("Range .fill").transition({P::width, P::height}, mo.arrive());
        rule("Range .thumb-anchor").transition({P::left, P::bottom}, mo.arrive());
        rule("Range:active .fill").transition(Transition{});
        rule("Range:active .thumb-anchor").transition(Transition{});

        shadow(rule("Range .thumb"), theme.elevation.thumb)
            .width(14)
            .height(14)
            .borderRadius(r.button)
            .backgroundColor(p.onAccent);
        focusRing(rule("Range:focus-visible .thumb"), p, m);

        text(rule("Range .value"), t.xs, t.regular, s)
            .width(34)
            .shrink(0)
            .textAlign(TextAlign::End)
            .color(p.text);
        rule("Range DefaultSliderSkin.vertical .value").display(Display::None); // No room

        rule("Range:disabled").opacity(0.5f);

        // endregion

        // region Text fields

        // A raised field in a hairline; focus shows as the accent's border, not a ring
        for (const char *field: {"TextInput", "TextArea"}) {
            text(rule(field), t.base, t.regular, s)
                .paddingHorizontal(m.space(2.5f))
                .border(m.border)
                .borderRadius(r.control)
                .borderColor(p.line)
                .backgroundColor(p.raised)
                .color(p.text)
                .cursor(Cursor::IBeam)
                .overflow(Overflow::Hidden);
        }
        rule("TextInput")
            .height(s.controlMd)
            .justify(Justify::Center);
        rule("TextArea")
            .minHeight(s.controlMd * 2)
            .paddingVertical(m.space(1.5f));

        rule("TextInput:focus").borderColor(p.accent);
        rule("TextArea:focus").borderColor(p.accent);
        rule("TextInput:disabled").opacity(0.5f);
        rule("TextArea:disabled").opacity(0.5f);

        // endregion

        // region Inputs

        // Fingers reach them from around them (none with a pointer), like the other controls
        for (const char *control: {"Switch", "SegmentedControl", "ColorSwatches", "NumberStepper", "SearchField"}) {
            rule(control).hitSlop(s.hitSlop);
        }

        rule("TextInput .placeholder").color(p.textMuted);

        // Search field: a text input's field, a muted search icon before the text, a clear button after it
        text(rule("SearchField"), t.base, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(2))
            .height(s.controlMd)
            .paddingLeft(m.space(2.5f))
            .paddingRight(m.space(1))
            .border(m.border)
            .borderRadius(r.control)
            .borderColor(p.line)
            .backgroundColor(p.raised)
            .color(p.text)
            .cursor(Cursor::IBeam);
        rule("SearchField:focus").borderColor(p.accent);
        rule("SearchField:disabled").opacity(0.5f);

        rule("SearchField .icon")
            .shrink(0)
            .color(p.textMuted);

        // The text input inside, bare: the field draws it
        rule("SearchField .input")
            .grow(1)
            .shrink(1)
            .alignSelf(Align::Stretch)
            .height(Auto)
            .paddingHorizontal(0)
            .border(0)
            .borderRadius(0)
            .backgroundColor(transparent);

        rule("SearchField .clear")
            .width(s.controlXs)
            .height(s.controlXs)
            .shrink(0)
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .borderRadius(r.small)
            .iconSize(m.iconSm)
            .color(p.textMuted);
        rule("SearchField .clear:hover")
            .color(p.text)
            .backgroundColor(p.line);

        // Switch: a pill track in a strong hairline, the accent when on, a white thumb sliding 2 px inside it
        text(rule("Switch"), t.sm, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(2))
            .color(p.textSecondary);

        rule("Switch .track")
            .width(30)
            .height(18)
            .paddingHorizontal(2)
            .border(m.border)
            .borderRadius(r.button)
            .borderColor(p.lineStrong)
            .backgroundColor(p.switchOff)
            .transition(colors, mo.moderate());
        rule("Switch.checked .track")
            .borderColor(p.accent)
            .backgroundColor(p.accent);
        focusRing(rule("Switch:focus-visible .track"), p, m);

        shadow(rule("Switch .thumb"), theme.elevation.thumb)
            .width(12)
            .height(12)
            .borderRadius(r.button)
            .backgroundColor(p.onAccent);

        // The thumb springs to its side, keeping its speed when flipped again half-way; it follows a drag at once
        rule("Switch .thumb-space").transition(P::grow, Timing{mo.spring});
        rule("Switch.dragging .thumb-space").transition(Transition{});

        rule("Switch:disabled").opacity(0.45f);

        // Segmented control: a pill on the surface in a hairline; the selected segment raised, nested 1 px in
        text(rule("SegmentedControl"), t.xs, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Stretch)
            .height(s.controlMd)
            .padding(1)
            .border(m.border)
            .borderRadius(r.button)
            .borderColor(p.line)
            .backgroundColor(p.surface)
            .color(p.textMuted)
            .iconSize(m.iconSm);
        rule("SegmentedControl.small").height(s.controlSm);
        rule("SegmentedControl:disabled").opacity(0.45f);

        rule("SegmentedControl .segment")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(1.5f))
            .paddingHorizontal(m.space(2))
            .borderRadius(r.button - 1)
            .transition(colors, quick);
        rule("SegmentedControl.small .segment").paddingHorizontal(m.space(1.5f));
        rule("SegmentedControl .segment:hover").color(p.text);
        text(rule("SegmentedControl .segment.selected"), t.xs, t.medium, s)
            .color(p.text)
            .backgroundColor(p.raised);
        // Inside the segment, like a sidebar item's: outside it would cross the group's edge
        focusRing(rule("SegmentedControl:focus-visible .segment.selected"), p, m).outlineOffset(-m.focusRing);

        // Number stepper: a raised field four controls wide, − and + at its ends behind hairlines
        text(rule("NumberStepper"), t.base, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Stretch)
            .width(s.controlMd * 4)
            .height(s.controlMd)
            .border(m.border)
            .borderRadius(r.control)
            .borderColor(p.line)
            .backgroundColor(p.raised)
            .color(p.text);
        text(rule("NumberStepper.small"), t.sm, t.regular, s)
            .width(s.controlSm * 4)
            .height(s.controlSm);
        rule("NumberStepper:focus").borderColor(p.accent);
        rule("NumberStepper:disabled").opacity(0.5f);

        // The text input inside, bare and centered
        rule("NumberStepper .value")
            .grow(1)
            .shrink(1)
            .height(Auto)
            .minWidth(0)
            .paddingHorizontal(m.space(1))
            .border(0)
            .borderRadius(0)
            .backgroundColor(transparent)
            .textAlign(TextAlign::Center);
        rule("NumberStepper .value:disabled").opacity(1.0f); // The stepper fades already

        for (const char *button: {"NumberStepper .decrement", "NumberStepper .increment"}) {
            rule(button)
                .width(s.controlMd)
                .shrink(0)
                .alignItems(Align::Center)
                .justify(Justify::Center)
                .borderColor(p.line)
                .iconSize(m.iconSm)
                .color(p.textSecondary);
        }
        rule("NumberStepper.small .decrement").width(s.controlSm);
        rule("NumberStepper.small .increment").width(s.controlSm);
        rule("NumberStepper .decrement")
            .borderRight(m.border)
            .borderRadiusLeft(r.control - m.border);
        rule("NumberStepper .increment")
            .borderLeft(m.border)
            .borderRadiusRight(r.control - m.border);
        for (const char *hover: {"NumberStepper .decrement:hover", "NumberStepper .increment:hover"}) {
            rule(hover)
                .color(p.text)
                .backgroundColor(p.line);
        }
        // At a bound: faded, and no hover
        for (const char *bound: {"NumberStepper .decrement:disabled", "NumberStepper .increment:disabled"}) {
            rule(bound)
                .color(p.textMuted)
                .backgroundColor(transparent);
        }

        // Color swatches: 20 px chips 4 px apart, wrapping; the selected one ringed in the accent, 2 px out; the one
        // the keyboard is on ringed in a neutral grey while the focus is visible (the accent when it is the selected)
        rule("ColorSwatches")
            .direction(Direction::Row)
            .wrap(Wrap::Wrap)
            .gap(m.space(1));

        rule("ColorSwatches .swatch")
            .width(s.controlXs)
            .height(s.controlXs)
            .shrink(0)
            .borderRadius(r.button)
            .transition(P::outlineColor, quick);
        rule("ColorSwatches .swatch.selected")
            .outlineColor(p.accent)
            .outlineWidth(2)
            .outlineOffset(2);
        rule("ColorSwatches:focus-visible .swatch.focused")
            .outlineColor(p.textMuted)
            .outlineWidth(m.focusRing)
            .outlineOffset(2);
        rule("ColorSwatches:focus-visible .swatch.focused.selected")
            .outlineColor(p.accent);
        rule("ColorSwatches:disabled").opacity(0.45f);

        // endregion

        // region Scroll bars

        // Thin and self-effacing: a translucent pill, inset in a transparent track
        rule("ScrollBar.vertical").width(m.scrollBar);
        rule("ScrollBar.horizontal").height(m.scrollBar);

        rule("ScrollBar .thumb")
            .borderRadius(r.button)
            .backgroundColor(p.scrollThumb)
            .transition(P::backgroundColor, quick);
        rule("ScrollBar.vertical .thumb")
            .marginHorizontal(m.scrollThumbInset)
            .minHeight(s.controlSm);
        rule("ScrollBar.horizontal .thumb")
            .marginVertical(m.scrollThumbInset)
            .minWidth(s.controlSm);

        rule("ScrollBar:hover .thumb").backgroundColor(p.scrollThumbHover);
        rule("ScrollBar:active .thumb").backgroundColor(p.scrollThumbHover);
        rule("ScrollBar:disabled .thumb").visible(false);

        rule("Scroller ScrollBar.horizontal").marginRight(m.scrollBar);

        // endregion

        // region Menus

        // The floating surface: overlay color, strong hairline, the popup radius, the one shadow
        floating(rule("Menu"), theme.elevation)
            .backgroundColor(p.overlay)
            .border(m.border)
            .borderColor(p.lineStrong)
            .borderRadius(r.popup)
            .padding(m.space(1))
            .minWidth(m.menuWidth)
            .alignItems(Align::Stretch)
            .color(p.text);

        // Rows: 32 px, 13 px text, raised grey while highlighted
        text(rule("Menu MenuButton"), t.base, t.regular, s)
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(2))
            .minHeight(s.rowSm)
            .paddingVertical(m.space(1.5f))
            .paddingHorizontal(m.space(2))
            .borderRadius(r.row)
            .color(p.textSecondary)
            .backgroundColor(transparent);

        rule("Menu MenuButton.submenu").skin("submenu");
        rule("Menu MenuButton .icon").color(p.textMuted).transition(P::color, quick);
        rule("Menu MenuButton .label").grow(1);

        text(rule("Menu MenuButton .shortcut"), t.xs, t.regular, s)
            .paddingLeft(m.space(4))
            .color(p.textMuted);

        rule("Menu MenuButton .arrow")
            .iconSize(m.iconSm)
            .marginLeft(m.space(2))
            .color(p.textMuted);

        // Highlighted (the menu's one row, by the mouse or the keyboard)
        rule("Menu MenuButton:active")
            .color(p.text)
            .backgroundColor(p.raised);
        rule("Menu MenuButton:active .icon").color(p.text);

        // Check marks keep their column, in the primary color; a select's after the label
        rule("Menu MenuButton .check")
            .shrink(0)
            .color(p.text);
        rule("Menu MenuButton .indicator")
            .shrink(0)
            .marginLeft(m.space(2))
            .color(p.text);
        rule("Menu.listbox MenuButton.checked").color(p.text);

        rule("Menu MenuButton:disabled").opacity(0.45f);

        // Actions that destroy, in the danger color, softly tinted while highlighted
        rule("Menu MenuButton.danger").color(p.dangerText);
        rule("Menu MenuButton.danger .icon").color(p.dangerText);
        rule("Menu MenuButton.danger:active")
            .color(p.dangerText)
            .backgroundColor(p.danger.withAlpha(0x14)); // 8%
        rule("Menu MenuButton.danger:active .icon").color(p.dangerText);

        // A hairline across the menu, its padding included
        rule("Menu .separator")
            .height(m.border)
            .shrink(0)
            .marginVertical(m.space(1))
            .marginHorizontal(-m.space(1))
            .backgroundColor(p.line);

        text(rule("Menu .group-label"), t.xs, t.medium, s)
            .paddingTop(m.space(1.5f))
            .paddingHorizontal(m.space(2))
            .paddingBottom(m.space(1))
            .color(p.textMuted);

        // endregion

        // region Select

        // A closed field like a text input: the value, the chevron at the end; the accent's border while open
        text(rule("Select"), t.base, t.regular, s)
            .gap(m.space(2))
            .height(s.controlMd)
            .paddingLeft(m.space(2.5f))
            .paddingRight(m.space(2))
            .border(m.border)
            .borderRadius(r.control)
            .borderColor(p.line)
            .backgroundColor(p.raised)
            .color(p.text)
            .hitSlop(s.hitSlop)
            .transition(colors, quick);

        rule("Select:hover").borderColor(p.lineStrong);
        rule("Select:focus-visible").borderColor(p.accent);
        rule("Select.open").borderColor(p.accent);
        rule("Select:disabled").opacity(0.5f);
        rule("Select .value.placeholder").color(p.textMuted);
        rule("Select .chevron").color(p.textMuted);

        // endregion

        // region Tooltips

        // A small label on the overlay color, in a strong hairline, with an arrow
        floating(text(rule("Tooltip"), t.xs, t.regular, s), theme.elevation)
            .paddingVertical(m.space(1))
            .paddingHorizontal(m.space(2))
            .maxWidth(m.popoverWidth)
            .border(m.border)
            .borderColor(p.lineStrong)
            .borderRadius(r.row)
            .backgroundColor(p.overlay)
            .color(p.text);

        // endregion

        // region Dialogs

        // The scrim over the page, the panel on it: the floating surface, a header, a body, a footer after a hairline
        rule("Dialog").backgroundColor(p.scrim);

        floating(rule("Dialog .panel"), theme.elevation)
            .width(m.dialogSm)
            .maxWidth(100_pct)
            .maxHeight(80_pct)
            .border(m.border)
            .borderColor(p.lineStrong)
            .borderRadius(r.popup)
            .backgroundColor(p.overlay)
            .color(p.text)
            .overflow(Overflow::Hidden);
        rule("Dialog.wide .panel").width(m.dialogMd);

        rule("Dialog .header")
            .gap(m.space(1.5f))
            .paddingTop(m.space(3))
            .paddingHorizontal(m.gutterCard);
        text(rule("Dialog .header .title"), t.sm, t.regular, s).color(p.text);
        rule("Dialog .header .icon").color(p.textSecondary);

        rule("Dialog .body")
            .gap(m.space(3))
            .padding(m.gutterCard);

        text(rule("Dialog .description"), t.sm, t.regular, s)
            .lineHeight(t.sm.size * 1.625f + s.textStep)
            .color(p.textMuted);

        rule("Dialog .footer")
            .gap(m.space(1.5f))
            .paddingVertical(m.space(3))
            .paddingHorizontal(m.gutterCard)
            .borderTop(m.border)
            .borderColor(p.line);

        // endregion

        // region Toasts

        // Stacked in the bottom right corner, the newest at the bottom
        rule("ToastStack")
            .right(m.viewportMargin)
            .bottom(m.viewportMargin)
            .width(m.toastWidth)
            .gap(m.space(2));

        // The floating surface: an icon on a tile in the variant's color, the title and description, the actions
        floating(text(rule("Toast"), t.sm, t.regular, s), theme.elevation)
            .gap(m.space(2.5f))
            .padding(m.space(3))
            .border(m.border)
            .borderColor(p.lineStrong)
            .borderRadius(r.popup)
            .backgroundColor(p.overlay)
            .color(p.text);

        rule("Toast .icon-tile")
            .width(s.controlSm)
            .height(s.controlSm)
            .borderRadius(r.row)
            .color(p.accentText)
            .backgroundColor(p.accentSoft);
        rule("Toast.success .icon-tile").color(p.success).backgroundColor(p.success.withAlpha(0x1A)); // 10%
        rule("Toast.warning .icon-tile").color(p.warningText).backgroundColor(p.warning.withAlpha(0x1A));
        rule("Toast.danger .icon-tile").color(p.dangerText).backgroundColor(p.danger.withAlpha(0x1A));

        rule("Toast .text").gap(m.space(0.5f));
        text(rule("Toast .title"), t.sm, t.medium, s).color(p.text);
        rule("Toast .description").color(p.textSecondary);

        rule("Toast .actions")
            .gap(m.space(1.5f))
            .marginTop(m.space(1.5f));
        text(rule("Toast .actions Button"), t.xs, t.medium, s)
            .height(s.controlXs)
            .paddingHorizontal(m.space(2));

        rule("Toast Button.close")
            .width(s.controlXs)
            .height(s.controlXs)
            .marginTop(-m.space(0.5f))
            .marginRight(-m.space(0.5f));

        // endregion

        // region Entering and leaving

        // Pivotal's floating motion: what floats fades and scales from 97% about its anchor (`Popup` puts its
        // transform origin there) as it opens, and back as it closes, a little quicker. Popups have the `entering`
        // class for their first style and `exiting` while they leave, so these rules are where they come from and go.
        const Timing enter  = mo.enter();
        const Timing leave  = mo.leave();
        const auto   motion = {P::opacity, P::scale, P::translateX, P::translateY};

        rule("Menu").transition(motion, enter).transition(P::outlineColor, quick);
        rule("Menu.entering").opacity(0).scale(0.97f);
        rule("Menu.exiting").opacity(0).scale(0.97f).transition(motion, leave);

        // A tooltip fades in from a few pixels toward its anchor, and out the same way
        const float nudge = m.space(1);
        rule("Tooltip").transition(motion, enter);
        rule("Tooltip.exiting").opacity(0).transition(motion, leave);
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

        // A toast slides in from under the stack's edge, and back out fading; one pushed out by a new one (the
        // stack's `limit`) only fades, where it is
        rule("Toast").transition(motion, Timing{mo.slow, mo.out});
        rule("Toast.entering").opacity(0).translateY(150_pct);
        rule("Toast.exiting").opacity(0).translateY(150_pct).transition(motion, Timing{mo.medium, mo.out});
        rule("Toast.exiting.limited").translateY(0);

        // endregion

        // region Menu bar

        // Its menus' triggers: ghost pills
        rule("MenuBar")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(0.5f));

        text(rule("MenuBar Button"), t.sm, t.medium, s)
            .skin("ghost")
            .height(s.controlSm)
            .paddingHorizontal(m.space(2));

        // endregion

        // region Tool bar

        // A strip under a header: Pivotal's toolbar band, with quiet small buttons
        rule("ToolBar")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(1.5f))
            .height(m.barToolbar)
            .paddingLeft(m.gutter)
            .paddingRight(m.space(3))
            .borderBottom(m.border)
            .borderColor(p.line);

        text(rule("ToolBar Button"), t.xs, t.medium, s)
            .skin("quiet")
            .height(s.controlSm)
            .paddingHorizontal(m.space(2));

        // The current tool (a toggled button), like the primary action, in the accent
        rule("ToolBar Button:active DefaultButtonSkin.quiet")
            .color(p.onAccent)
            .backgroundColor(p.accent);

        // endregion

        // region Tabs

        // Tabs on a hairline: the selected one underlined in the text color, never the accent
        rule("Tabs")
            .direction(Direction::Row)
            .alignItems(Align::End)
            .gap(m.space(1))
            .minHeight(m.barToolbar)
            .paddingHorizontal(m.gutter)
            .borderBottom(m.border)
            .borderColor(p.line);

        text(rule("Tabs Button"), t.base, t.medium, s)
            .skin("tab")
            .height(s.rowMd)
            .marginBottom(-m.border) // Over the line, so the underline covers it
            .paddingHorizontal(m.space(2.5f))
            .border(0)
            .borderRadius(0)
            .color(p.textMuted)
            .backgroundColor(transparent)
            .borderColor(transparent);

        rule("Tabs Button:hover")
            .color(p.text)
            .borderColor(p.lineStrong);

        rule("Tabs Button:active")
            .color(p.text)
            .borderColor(p.text);

        rule("TabContainer").backgroundColor(p.page);

        // endregion

        // region App shell

        // The frame: the sidebar's color around the page panel, inset in it
        rule("AppShell").backgroundColor(p.frame);

        rule("AppShell .main")
            .paddingTop(m.frameInset)
            .paddingRight(m.frameInset)
            .paddingBottom(m.frameInset);

        rule("AppShell .page")
            .backgroundColor(p.page)
            .borderRadius(r.page)
            .overflow(Overflow::Hidden);

        rule("Sidebar")
            .width(m.sidebarWidth)
            .backgroundColor(p.frame);

        // The header: the window controls, then the app's mark and name, lit like the rows
        rule("SidebarHeader")
            .gap(m.space(2.5f))
            .height(m.barHeader)
            .paddingLeft(m.space(3.5f))
            .paddingRight(m.space(2.5f));

        rule("SidebarHeader .brand")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(2))
            .grow(1)
            .shrink(1)
            .height(s.controlMd)
            .paddingHorizontal(m.space(1.5f))
            .borderRadius(r.row);

        text(rule("SidebarHeader .mark"), t.xs, t.semiBold, s)
            .width(20)
            .height(20)
            .shrink(0)
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .borderRadius(r.small)
            .backgroundColor(p.accent)
            .color(p.onAccent);

        text(rule("SidebarHeader .title"), t.md, t.semiBold, s)
            .letterSpacing(-0.14f) // -0.01em
            .shrink(1)
            .color(p.text);

        rule("SidebarHeader .chevron")
            .iconSize(m.iconSm)
            .color(p.textMuted);

        // Sections, set apart by a hairline
        rule("SidebarSection")
            .gap(m.space(0.5f))
            .padding(m.space(2))
            .paddingTop(m.space(1.5f))
            .borderTop(m.border)
            .borderColor(p.line);

        rule("SidebarSection:first-child")
            .paddingTop(m.space(2))
            .borderTop(0);

        text(rule("SidebarSection .heading"), t.xs, t.medium, s)
            .paddingTop(m.space(1))
            .paddingHorizontal(m.space(2))
            .paddingBottom(m.space(0.5f))
            .color(p.textMuted);

        // Rows: 32 px, 13 px text in the secondary color, the current one raised with an accent icon
        text(rule("SidebarItem"), t.base, t.regular, s)
            .gap(m.space(2))
            .height(s.rowSm)
            .paddingHorizontal(m.space(2))
            .borderRadius(r.row)
            .color(p.textSecondary)
            .transition(colors, quick);

        rule("SidebarItem .icon").color(p.textMuted).transition(P::color, quick);
        rule("SidebarItem .label").grow(1).shrink(1);
        text(rule("SidebarItem .count"), t.xs, t.regular, s).color(p.textMuted);

        rule("SidebarItem:hover")
            .color(p.text)
            .backgroundColor(p.hover);

        rule("SidebarItem.current")
            .color(p.text)
            .backgroundColor(p.raised);
        rule("SidebarItem.current .icon").color(p.accent);
        focusRing(rule("SidebarItem:focus-visible"), p, m).outlineOffset(-m.focusRing);

        // A page's title bar: the sidebar header's height, so that their lines meet
        rule("PageHeader")
            .gap(m.space(2))
            .height(m.barHeader)
            .paddingLeft(m.gutter)
            .paddingRight(m.space(3))
            .borderBottom(m.border)
            .borderColor(p.line);

        rule("PageHeader .icon").color(p.textMuted);
        text(rule("PageHeader .title"), t.md, t.medium, s).color(p.text);

        // endregion

        // region Lists

        // Shared by the rows of lists and grids: raised under the mouse, the selection raised with an accent bar at
        // its start (a 2 px left border, transparent until then, so that nothing moves), the focus ring inside
        const auto selectableRow = [&](const std::string &row) {
            rule(row.c_str())
                .paddingLeft(m.gutter - 2)
                .paddingRight(m.gutter)
                .borderLeft(2)
                .borderBottom(m.border)
                .borderColor(p.line)
                .borderLeftColor(transparent)
                .color(p.text)
                .transition({P::color, P::backgroundColor, P::borderLeftColor, P::outlineColor}, quick);
            rule((row + ":last-child").c_str()).borderBottom(0);
            rule((row + ":hover").c_str()).backgroundColor(p.raised);
            rule((row + ".selected").c_str())
                .backgroundColor(p.raised)
                .borderLeftColor(p.accent);
            rule((row + ":disabled").c_str()).opacity(0.45f);
            focusRing(rule((row + ":focus-visible").c_str()), p, m).outlineOffset(-m.focusRing);
        };

        // Rows edge to edge on the page's gutter, between hairlines: 48 px (Pivotal's default), `.md` 40, `.sm` 32
        rule("List")
            .borderTop(m.border)
            .borderColor(p.line);

        selectableRow("ListRow");
        text(rule("ListRow"), t.sm, t.regular, s)
            .gap(m.space(3))
            .minHeight(s.rowMd + m.space(2))
            .paddingVertical(m.space(2));
        rule("List.md ListRow")
            .minHeight(s.rowMd)
            .paddingVertical(m.space(1.5f));
        rule("List.sm ListRow")
            .minHeight(s.rowSm)
            .paddingVertical(m.space(1));

        rule("ListRow .leading")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .shrink(0)
            .color(p.textMuted);
        rule("ListRow .text").gap(m.space(1));
        rule("ListRow .title").color(p.text);
        text(rule("ListRow .secondary"), t.xs, t.regular, s).color(p.textMuted);
        rule("ListRow .trailing")
            .direction(Direction::Row)
            .alignItems(Align::Center)
            .gap(m.space(1.5f))
            .shrink(0);
        text(rule("ListRow .meta"), t.xs, t.regular, s).color(p.textMuted);

        // A data grid: a header row a tool bar's height in small muted text, then rows of cells 16 px apart
        text(rule("DataGrid .header"), t.xs, t.regular, s)
            .gap(m.space(4))
            .height(m.barToolbar)
            .paddingHorizontal(m.gutter)
            .borderBottom(m.border)
            .borderColor(p.line)
            .color(p.textMuted);

        rule("DataGrid .header .sort").gap(m.space(1));
        rule("DataGrid .header .sort")
            .paddingHorizontal(m.space(1)) // Room for the focus ring, taken back from the cell
            .marginHorizontal(-m.space(1))
            .cursor(Cursor::Hand)
            .borderRadius(r.small);
        rule("DataGrid .header .sort:hover").color(p.textSecondary);
        rule("DataGrid .header .sorted").color(p.text);
        rule("DataGrid .header .sorted:hover").color(p.text);
        focusRing(rule("DataGrid .header .sort:focus-visible"), p, m);

        // The arrow: hidden, faint (the next order) under the mouse or the keyboard's focus, plain when sorted
        rule("DataGrid .sort-icon")
            .iconSize(m.iconSm)
            .opacity(0);
        rule("DataGrid .sort:hover .sort-icon").opacity(0.6f);
        rule("DataGrid .sort:focus-visible .sort-icon").opacity(0.6f);
        rule("DataGrid .sorted .sort-icon").opacity(1);

        selectableRow("DataGridRow");
        text(rule("DataGridRow"), t.sm, t.regular, s)
            .gap(m.space(4))
            .minHeight(s.rowMd)
            .paddingVertical(m.space(1.5f));
        rule("DataGrid.sm DataGridRow")
            .minHeight(s.rowSm)
            .paddingVertical(m.space(1));
        rule("DataGridRow .cell").gap(m.space(1.5f));
        rule("DataGridRow .cell.end").color(p.textSecondary);

        rule("DataGrid ScrollBar.vertical").marginVertical(m.space(0.5f));

        // A tree: the sidebar's rows (32 px, 13 px text), indented a step per level; the selection raised in a
        // strong hairline, as Pivotal's
        rule("Tree").gap(1);

        text(rule("TreeRow"), t.base, t.regular, s)
            .gap(m.space(1.5f))
            .height(s.rowSm)
            .paddingLeft(m.space(1))
            .paddingRight(m.space(2))
            .border(m.border)
            .borderRadius(r.row)
            .borderColor(transparent)
            .color(p.textSecondary)
            .transition(colors, quick);

        rule("TreeRow:hover")
            .color(p.text)
            .backgroundColor(p.hover);
        rule("TreeRow.selected")
            .color(p.text)
            .backgroundColor(p.raised)
            .borderColor(p.lineStrong);
        rule("TreeRow:disabled").opacity(0.45f);
        focusRing(rule("TreeRow:focus-visible"), p, m).outlineOffset(-m.focusRing);

        // The chevron's column is an icon wide, kept on leaves
        rule("TreeRow .disclosure")
            .width(m.iconMd)
            .iconSize(m.iconSm)
            .color(p.textMuted);
        rule("TreeRow .icon").color(p.textMuted);
        rule("TreeRow.selected .icon").color(p.text);
        text(rule("TreeRow .trailing"), t.xs, t.regular, s).color(p.textMuted);

        // An inspector: a panel on the surface layer, sections between hairlines, fields in two columns
        text(rule("Inspector"), t.sm, t.regular, s)
            .backgroundColor(p.surface)
            .borderLeft(m.border)
            .borderColor(p.line);

        rule("Inspector .heading")
            .gap(m.space(2.5f))
            .paddingVertical(m.space(3))
            .paddingHorizontal(m.gutter)
            .borderBottom(m.border)
            .borderColor(p.line);
        rule("Inspector .media")
            .width(32)
            .height(32)
            .borderRadius(r.control)
            .backgroundColor(p.raised)
            .color(p.textSecondary)
            .iconSize(16);
        rule("Inspector .titles").gap(m.space(0.5f));
        text(rule("Inspector .heading .title"), t.md, t.medium, s).color(p.text);
        text(rule("Inspector .heading .subtitle"), t.sm, t.regular, s).color(p.textMuted);

        rule("InspectorSection")
            .borderBottom(m.border)
            .borderColor(p.line);
        rule("InspectorSection:last-child").borderBottom(0);

        rule("InspectorSection .header")
            .gap(m.space(2))
            .minHeight(s.controlMd)
            .paddingTop(m.space(3))
            .paddingHorizontal(m.gutter)
            .paddingBottom(m.space(1));
        text(rule("InspectorSection .header .title"), t.sm, t.medium, s).color(p.textSecondary);

        rule("InspectorSection .body")
            .gap(m.space(2))
            .paddingTop(m.space(1))
            .paddingHorizontal(m.gutter)
            .paddingBottom(m.space(4));

        rule("InspectorField").gap(m.space(3));
        rule("InspectorField .name")
            .width(104)
            .color(p.textMuted);
        rule("InspectorField .value").gap(m.space(1.5f));
        rule("InspectorField .text").color(p.text);

        // endregion

        // region Badges, status dots, chips

        // Pills outlined in their tone, the border its color at a third or so
        const Color accentBorder  = p.accent.withAlpha(0x73);  // 45%
        const Color successBorder = p.success.withAlpha(0x59); // 35%
        const Color warningBorder = p.warning.withAlpha(0x59); // 35%
        const Color dangerBorder  = p.danger.withAlpha(0x4D);  // 30%

        text(rule("Badge"), t.xs, t.regular, s)
            .gap(m.space(1))
            .height(s.controlXs)
            .paddingHorizontal(m.space(2))
            .border(m.border)
            .borderRadius(r.button)
            .color(p.textSecondary)
            .borderColor(p.lineStrong);
        text(rule("Badge.small"), t.xxs, t.regular, s)
            .height(16)
            .paddingHorizontal(m.space(1.5f));
        rule("Badge.accent").color(p.accentText).borderColor(accentBorder);
        rule("Badge.success").color(p.success).borderColor(successBorder);
        rule("Badge.warning").color(p.warningText).borderColor(warningBorder);
        rule("Badge.danger").color(p.dangerText).borderColor(dangerBorder);

        rule("StatusDot")
            .width(6)
            .height(6)
            .borderRadius(r.button)
            .backgroundColor(p.textMuted);
        rule("StatusDot.accent").backgroundColor(p.accent);
        rule("StatusDot.success").backgroundColor(p.success);
        rule("StatusDot.warning").backgroundColor(p.warning);
        rule("StatusDot.danger").backgroundColor(p.danger);

        // Quiet pills: secondary text in a hairline, raised under the mouse or when selected
        text(rule("Chip"), t.sm, t.regular, s)
            .gap(m.space(1.5f))
            .height(s.controlMd)
            .paddingHorizontal(m.space(2.5f))
            .border(m.border)
            .borderRadius(r.button)
            .borderColor(p.line)
            .color(p.textSecondary)
            .hitSlop(s.hitSlop)
            .transition(colors, quick);
        rule("Chip.removable").paddingRight(m.space(1.5f));
        for (const char *state: {"Chip:hover", "Chip.selected"}) {
            rule(state)
                .color(p.text)
                .backgroundColor(p.raised)
                .borderColor(p.lineStrong);
        }
        rule("Chip:disabled").opacity(0.45f);
        focusRing(rule("Chip:focus-visible"), p, m);
        rule("Chip .icon").iconSize(m.iconSm);
        rule("Chip .remove")
            .iconSize(m.iconSm)
            .color(p.textMuted)
            .cursor(Cursor::Hand);
        rule("Chip .remove:hover").color(p.text);

        // Toned chips: their text and outline tinted, softly filled when chosen or pointed at
        rule("Chip.accent").color(p.accentText).borderColor(accentBorder);
        rule("Chip.success").color(p.success).borderColor(successBorder);
        rule("Chip.warning").color(p.warningText).borderColor(warningBorder);
        rule("Chip.danger").color(p.dangerText).borderColor(dangerBorder);
        for (const char *state: {"Chip.accent:hover", "Chip.accent.selected"}) {
            rule(state).backgroundColor(p.accentSoft);
        }
        for (const char *state: {"Chip.success:hover", "Chip.success.selected"}) {
            rule(state).backgroundColor(p.success.withAlpha(0x1A)); // 10%
        }
        for (const char *state: {"Chip.warning:hover", "Chip.warning.selected"}) {
            rule(state).backgroundColor(p.warning.withAlpha(0x14)); // 8%
        }
        for (const char *state: {"Chip.danger:hover", "Chip.danger.selected"}) {
            rule(state).backgroundColor(p.danger.withAlpha(0x14)); // 8%
        }

        // endregion

        // region Avatars, keyboard hints, empty states

        // Initials on the raised layer in a hairline, medium weight; 16, 20, 24 (the default) or 32 px
        rule("Avatar")
            .alignItems(Align::Center)
            .justify(Justify::Center)
            .border(m.border)
            .borderRadius(r.button)
            .borderColor(p.line)
            .backgroundColor(p.raised)
            .color(p.textSecondary);
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
            .borderColor(p.page);
        rule("Avatar.lg .status")
            .width(10)
            .height(10);

        // Key caps: tiny text on the surface layer in a strong hairline
        rule("Kbd").gap(m.space(0.5f));
        text(rule("Kbd .key"), t.xxs, t.regular, s)
            .minWidth(18)
            .height(18)
            .lineHeight(16 + s.textStep) // The cap's inside, so that the key sits in its middle
            .paddingHorizontal(m.space(1))
            .border(m.border)
            .borderRadius(r.small)
            .borderColor(p.lineStrong)
            .backgroundColor(p.surface)
            .color(p.textSecondary)
            .textAlign(TextAlign::Center);

        // Nothing to show: centered, an icon tile, a medium title, muted text, the way forward
        rule("EmptyState")
            .gap(m.space(1.5f))
            .paddingVertical(80)
            .paddingHorizontal(m.space(6));
        rule("EmptyState .icon-tile")
            .width(40)
            .height(40)
            .marginBottom(m.space(1.5f))
            .border(m.border)
            .borderRadius(r.control)
            .borderColor(p.line)
            .backgroundColor(p.raised)
            .color(p.textSecondary)
            .iconSize(20);
        text(rule("EmptyState .title"), t.md, t.medium, s).color(p.text);
        text(rule("EmptyState .description"), t.sm, t.regular, s)
            .lineHeight(19.5f + s.textStep) // 1.625
            .maxWidth(384)
            .textAlign(TextAlign::Center)
            .color(p.textMuted);
        rule("EmptyState .action").marginTop(m.space(2.5f));

        // endregion
    }
}
