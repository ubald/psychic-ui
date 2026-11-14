#include <string>
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/themes/pivotal.hpp"
#include "psychic-ui/themes/ubuntu-type.hpp"
#include "DefaultSkins.hpp"
#include "DefaultBasicButtonSkin.hpp"
#include "DefaultButtonSkin.hpp"
#include "DefaultCheckBoxSkin.hpp"
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
    }

    void DefaultSkins::load(StyleManager *manager) {
        const Theme      &theme = manager->theme();
        const Palette    &p     = theme.palette;
        const Typography &t     = theme.type;
        const Metrics    &m     = theme.metrics;
        const Radii      &r     = m.radius;
        const Sizes      &s     = manager->sizes();

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
            .textAntiAlias(true);

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

        // The three dots, in the theme's semantic colors
        rule("WindowControls").gap(7);

        rule("WindowControls Button")
            .skin("title-bar")
            .width(11)
            .height(11)
            .padding(0)
            .border(0);

        rule("WindowControls Button.close").backgroundColor(p.danger);
        rule("WindowControls Button.minimize").backgroundColor(p.warning);
        rule("WindowControls Button.maximize").backgroundColor(p.success);

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
        rule("Button:active").borderColor(p.textMuted); // Pressed, or selected (a toggle)
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
            .padding(0)
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
            .iconStrokeWidth(1.8f);

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

        // region Scroll bars

        // Thin and self-effacing: a translucent pill, inset in a transparent track
        rule("ScrollBar.vertical").width(m.scrollBar);
        rule("ScrollBar.horizontal").height(m.scrollBar);

        rule("ScrollBar .thumb")
            .borderRadius(r.button)
            .backgroundColor(p.scrollThumb);
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
        rule("Menu MenuButton .icon").color(p.textMuted);
        rule("Menu MenuButton .label").grow(1);

        text(rule("Menu MenuButton .shortcut"), t.xs, t.regular, s)
            .paddingLeft(m.space(4))
            .color(p.textMuted);

        rule("Menu MenuButton .arrow")
            .iconSize(m.iconSm)
            .marginLeft(m.space(2))
            .color(p.textMuted);

        for (const char *state: {"Menu MenuButton:hover", "Menu MenuButton:active"}) {
            rule(state)
                .color(p.text)
                .backgroundColor(p.raised);
        }
        rule("Menu MenuButton:hover .icon").color(p.text);
        rule("Menu MenuButton:active .icon").color(p.text);

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
    }
}
