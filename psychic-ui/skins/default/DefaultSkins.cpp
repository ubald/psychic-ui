#include <string>
#include <utility>
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/themes/one-dark.hpp"
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
        /**
         * Where the default chrome goes: a component and, inside it, the part carrying the chrome
         */
        constexpr std::pair<const char *, const char *> chromes[]{
            {"Button", "DefaultButtonSkin"},
            {"CheckBox", ".box"},
            {"Range", "DefaultSliderSkin"},
            {"TextInput", "DefaultTextInputSkin"},
            {"TextArea", "DefaultTextAreaSkin"},
        };
    }

    std::string_view DefaultSkins::name() const {
        return "Default";
    }

    const Theme &DefaultSkins::defaultTheme() const {
        return themes::oneDark;
    }

    void DefaultSkins::registerSkins(SkinRegistry &skins) const {
        skins.add<ButtonSkin, DefaultButtonSkin>()
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
        const Sizes      &s     = manager->sizes();

        // Inter (Pivotal's) and Ubuntu are always there, so any style can name them; then the theme's own
        for (const Typography &type: {Typography{}, themes::ubuntuType, t}) {
            for (const Font &font: {type.regular, type.medium, type.semiBold}) {
                manager->loadFont(std::string{font.name}, std::string{font.path});
            }
        }

        // region Defaults
        manager->style("*")
               .overflow(Overflow::Visible)
               .fontFamily(t.regular.name)
               .fontSize(t.sm.size)
               .lineHeight(t.sm.leading)
               .color(p.text)
               .selectionColor(p.onAccent)
               .selectionBackgroundColor(p.accent)
               .antiAlias(true)
               .textAntiAlias(true);

        // The default chrome (see DefaultChrome.hpp), on the skins that draw it and on a check box's box.
        // Its border thickens when focused, eating its margin, so the content doesn't move.
        for (const auto &[component, part]: chromes) {
            const auto rule = [&](const std::string &state) -> Style & {
                return manager->style(std::string{component} + state + " " + part);
            };

            rule("")
                .borderRadius(m.radius.control)
                .border(m.border)
                .margin(m.focusRing - m.border)
                .borderColor(p.raised.withAlpha(0x40))
                .backgroundColor(p.raised)
                .contentBackgroundColor(p.base);

            rule(":hover")
                .borderColor(p.raised.withAlpha(0x80))
                .backgroundColor(p.hover);

            rule(":active")
                .borderColor(p.raised.withAlpha(0xC0))
                .backgroundColor(p.accent);

            rule(":focus")
                .border(m.focusRing)
                .margin(0)
                .borderColor(p.accent);

            rule(":disabled")
                .backgroundColor(p.raised);

            rule(":disabled:hover")
                .backgroundColor(p.raised);

            rule(":disabled:active")
                .backgroundColor(p.accent); // TODO: Fade
        }

        // endregion

        // Fingers reach the controls from around them (none with a pointer); they look the same
        for (const char *control: {"Button", "CheckBox", "Range", "ScrollBar", "TextInput", "TextArea", "MenuButton"}) {
            manager->style(control).hitSlop(s.hitSlop);
        }

        // region Typography

        manager->style(".h1")
               .fontFamily(t.regular.name)
               .fontSize(t.xxl.size)
               .lineHeight(t.xxl.size);

        manager->style(".h2")
               .fontFamily(t.regular.name)
               .fontSize(t.xxl.size)
               .lineHeight(t.xxl.size);

        manager->style(".h3")
               .fontFamily(t.regular.name)
               .fontSize(t.xl.size)
               .lineHeight(t.xl.size);

        manager->style(".h4")
               .fontFamily(t.regular.name)
               .fontSize(t.md.size)
               .lineHeight(t.md.size);

        manager->style(".h5")
               .fontFamily(t.regular.name)
               .fontSize(t.sm.size)
               .lineHeight(t.sm.size);

        manager->style(".h6")
               .fontFamily(t.regular.name)
               .fontSize(t.xxs.size)
               .lineHeight(t.xxs.size);

        // endregion

        // region Window
        manager->style("Window")
               .color(p.text)
               .backgroundColor(p.base)
               .border(m.border)
               .borderColor(p.line);
        // endregion

        // region Title Bar
        manager->style("TitleBar")
               .alignItems(Align::Center)
               .height(48)
               .padding(m.space(3))
               .backgroundColor(p.surface)
               .borderBottom(m.border)
               .borderColor(p.line);

        manager->style("TitleBar Label.title")
               .fontFamily(t.regular.name)
               .fontSize(t.xl.size)
               .marginLeft(m.space(3))
               .grow(1);

        manager->style("TitleBar Button")
               .skin("title-bar")
               .width(18)
               .height(18)
               .margin(3)
               .padding(0)
               .border(0);

        manager->style("TitleBar Button.close")
               .backgroundColor(argb(0xffff5f57));

        manager->style("TitleBar Button.close:hover")
               .backgroundColor(argb(0xffff5f57));

        manager->style("TitleBar Button.minimize")
               .backgroundColor(argb(0xffffbd2f));

        manager->style("TitleBar Button.minimize:hover")
               .backgroundColor(argb(0xffffbd2f));

        manager->style("TitleBar Button.maximize")
               .backgroundColor(argb(0xff28c940));

        manager->style("TitleBar Button.maximize:hover")
               .backgroundColor(argb(0xff28c940));

        // endregion

        // region Text Input

        manager->style("TextInput")
               .padding(m.space(1.5f));

        manager->style("TextArea")
               .padding(m.space(1.5f));

        // endregion

        // region ScrollBars

        manager->style("ScrollBar.vertical")
               .width(m.scrollBar);

        manager->style("ScrollBar.horizontal")
               .height(m.scrollBar);

        manager->style("ScrollBar .track")
               .backgroundColor(p.base);

        manager->style("ScrollBar .thumb")
               .backgroundColor(p.raised);

        manager->style("ScrollBar.vertical .thumb")
               .marginLeft(1)
               .minHeight(s.controlSm);

        manager->style("ScrollBar.horizontal .thumb")
               .marginTop(1)
               .minWidth(s.controlSm);

        manager->style("ScrollBar:hover .thumb")
               .backgroundColor(p.raised);

        manager->style("ScrollBar:active .thumb")
               .backgroundColor(p.accent);

        manager->style("ScrollBar:disabled .thumb")
               .backgroundColor(p.surface);

        manager->style("Scroller ScrollBar.horizontal")
               .marginRight(m.scrollBar);

        // endregion

        // region Buttons
        manager->style("Button")
               .cursor(Cursor::Hand)
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .padding(m.space(3))
               .fontSize(t.md.size)
               .lineHeight(t.md.size)
               .color(p.text)
               .backgroundColor(p.raised);

        manager->style("Button:hover")
               .backgroundColor(p.hover);

        manager->style("Button:active")
               .color(p.onAccent)
               .backgroundColor(p.accent);

        manager->style("Button:disabled")
               .backgroundColor(p.raised);

        manager->style("Button:disabled:hover")
               .backgroundColor(p.raised);

        manager->style("Button:disabled:active")
               .color(p.onAccent)
               .backgroundColor(p.accent); // TODO: Fade

        // endregion

        // region CheckBox
        manager->style("CheckBox")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .padding(m.space(3))
               .color(p.text)
               .fontSize(t.md.size)
               .lineHeight(t.md.size);

        manager->style("CheckBox .box")
               .width(s.controlSm)
               .height(s.controlSm)
               .marginRight(m.space(1.5f));

        // endregion

        // region Menus
        // A translucent background rather than an opacity, which would fade the labels and highlights too
        manager->style("Menu")
               .backgroundColor(p.base.withAlpha(204))
               .minWidth(200)
               .alignItems(Align::Stretch);

        manager->style("Menu MenuButton")
               .direction(Direction::Row)
               .paddingHorizontal(m.space(1.5f))
               .paddingVertical(m.space(1.5f))
               .borderRadius(0)
               .backgroundColor(Color{}); // Transparent, over the menu's background

        manager->style("Menu MenuButton.submenu")
               .skin("submenu");

        manager->style("Menu MenuButton .label")
               .grow(1)
               .fontFamily(t.regular.name)
               .fontSize(t.xxs.size)
               .lineHeight(t.sm.size);

        manager->style("Menu MenuButton .shortcut")
               .fontSize(t.xxs.size)
               .lineHeight(t.sm.size)
               .color(p.text);

        manager->style("Menu MenuButton.submenu .arrow")
               .width(6);

        manager->style("Menu MenuButton:hover")
               .backgroundColor(p.accent);

        manager->style("Menu MenuButton:hover .label")
               .color(p.onAccent);

        manager->style("Menu MenuButton:hover .shortcut")
               .color(p.onAccent.withAlpha(0xB0));

        manager->style("Menu MenuButton.submenu:hover .arrow")
               .color(p.onAccent);

        manager->style("Menu MenuButton:active")
               .backgroundColor(p.accent);

        manager->style("Menu MenuButton:active .label")
               .color(p.onAccent);

        manager->style("Menu MenuButton:active .shortcut")
               .color(p.onAccent.withAlpha(0xB0));

        manager->style("Menu MenuButton.submenu:active .arrow")
               .color(p.onAccent);

        // endregion

        // region Ranges

        manager->style("Range DefaultSliderSkin")
               .borderRadius(m.radius.control)
               .fontFamily(t.regular.name)
               .fontSize(t.xxs.size)
               .lineHeight(t.xxs.size)
               .color(p.text);

        manager->style("Range DefaultSliderSkin.horizontal")
               .height(s.controlSm);

        manager->style("Range DefaultSliderSkin.vertical")
               .width(s.controlSm);

        manager->style("Range DefaultSliderSkin.vertical .value")
               .visible(false); // No room

        manager->style("Range:hover DefaultSliderSkin.inverted")
               .color(p.onAccent);

        manager->style("Range:hover DefaultSliderSkin.normal")
               .color(p.text);

        manager->style("Range:active DefaultSliderSkin.normal")
               .color(p.text);

        manager->style("Range:active DefaultSliderSkin.inverted")
               .color(p.onAccent);

        // endregion

        // region Tabs
        manager->style("Tabs")
               .direction(Direction::Row)
               .borderBottom(m.border)
               .borderColor(p.line)
               .backgroundColor(p.raised);

        manager->style("Tabs button")
               .skin("basic")
               .fontSize(t.xxs.size)
               .lineHeight(t.sm.size)
               .marginHorizontal(-m.border)
               .paddingHorizontal(m.space(3))
               .paddingVertical(m.space(1.5f))
               .borderRadius(0)
               .borderHorizontal(m.border)
               .borderColor(p.line);

        // endregion

        // region Tab Container

        manager->style("TabContainer")
               .backgroundColor(p.surface);

        // endregion

        // region MenuBar
        manager->style("MenuBar")
               .direction(Direction::Row)
               .backgroundColor(p.base)
               .borderBottom(m.border)
               .borderColor(p.line);

        manager->style("MenuBar Button")
               .skin("basic")
               .fontSize(t.xxs.size)
               .lineHeight(t.xxs.size)
               .paddingHorizontal(m.space(3))
               .paddingVertical(m.space(1.5f))
               .borderRadius(0)
               .backgroundColor(p.base);

        manager->style("MenuBar Button:hover")
               .color(p.accent);

        manager->style("MenuBar Button:active")
               .color(p.onAccent)
               .backgroundColor(p.accent);

        // endregion

        // region ToolBar
        manager->style("ToolBar")
               .direction(Direction::Row)
               .backgroundColor(p.raised)
               .borderBottom(m.border)
               .borderColor(p.line);

        manager->style("ToolBar Button")
               .skin("basic")
               .marginHorizontal(-m.border)
               .paddingHorizontal(m.space(4.5f))
               .borderRadius(0)
               .borderHorizontal(m.border)
               .borderColor(p.line);
        // endregion
    }
}
