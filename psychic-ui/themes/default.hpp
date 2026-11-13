#pragma once

#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/StyleSheet.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include <psychic-ui/themes/psychic.hpp>
#include <psychic-ui/skins/SliderRangeSkin.hpp>
#include <psychic-ui/skins/DefaultButtonSkin.hpp>
#include <psychic-ui/skins/DefaultScrollBarSkin.hpp>
#include <psychic-ui/skins/DefaultCheckBoxSkin.hpp>
#include <psychic-ui/skins/DefaultMenuButtonSkin.hpp>
#include <psychic-ui/skins/TitleBarButtonSkin.hpp>
#include <psychic-ui/skins/DefaultBasicButtonSkin.hpp>
#include <psychic-ui/skins/DefaultTextInputSkin.hpp>
#include <psychic-ui/skins/DefaultTextAreaSkin.hpp>

namespace psychic_ui {

/**
 * Named colors: a base16 gray scale and accents, and the One Dark palette
 */
namespace palette {
    inline constexpr Color base00 = argb(0xFF151515);
    inline constexpr Color base01 = argb(0xFF202020);
    inline constexpr Color base02 = argb(0xFF303030);
    inline constexpr Color base03 = argb(0xFF505050);
    inline constexpr Color base04 = argb(0xFFB0B0B0);
    inline constexpr Color base05 = argb(0xFFD0D0D0);
    inline constexpr Color base06 = argb(0xFFE0E0E0);
    inline constexpr Color base07 = argb(0xFFF5F5F5);

    inline constexpr Color baseRed       = argb(0xFFAC4142);
    inline constexpr Color baseOrange    = argb(0xFFD28445);
    inline constexpr Color baseYellow    = argb(0xFFF4BF75);
    inline constexpr Color baseGreen     = argb(0xFF90A959);
    inline constexpr Color baseTurquoise = argb(0xFF75B5AA);
    inline constexpr Color baseBlue      = argb(0xFF6A9FB5);
    inline constexpr Color baseViolet    = argb(0xFFAA759F);
    inline constexpr Color baseBrown     = argb(0xFF8F5536);

    inline constexpr Color oneLight      = argb(0xFFABB2BF);
    inline constexpr Color oneMedium     = argb(0xFF828997);
    inline constexpr Color oneDark       = argb(0xFF5C6370);
    inline constexpr Color oneCyan       = argb(0xFF56B6C2);
    inline constexpr Color oneBlue       = argb(0xFF61AFEF);
    inline constexpr Color onePurple     = argb(0xFFC678DD);
    inline constexpr Color oneGreen      = argb(0xFF98C379);
    inline constexpr Color oneRedFaded   = argb(0xFFE06C75);
    inline constexpr Color oneRed        = argb(0xFFBE5046);
    inline constexpr Color oneOrange     = argb(0xFFD19A66);
    inline constexpr Color oneYellow     = argb(0xFFE5C07B);
    inline constexpr Color oneForeground = argb(0xFFABB2BF);
    inline constexpr Color oneBackground = argb(0xFF282C34);
    inline constexpr Color oneGutter     = argb(0xFF636D83);
    inline constexpr Color oneBlueAccent = argb(0xFF528BFF);
}

/**
 * Psychic UI's look, from the style manager's theme
 */
class PsychicUIStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {
        const Theme      &theme = manager->theme();
        const Palette    &p     = theme.palette;
        const Typography &t     = theme.type;
        const Metrics    &m     = theme.metrics;

        manager->loadFont(std::string{t.light.name}, std::string{t.light.path});
        manager->loadFont(std::string{t.regular.name}, std::string{t.regular.path});

        manager->registerSkin("default-text-input-skin", SkinType::make([]() { return std::make_shared<DefaultTextInputSkin>(); }));
        manager->registerSkin("default-text-area-skin", SkinType::make([]() { return std::make_shared<DefaultTextAreaSkin>(); }));
        manager->registerSkin("default-basic-button-skin", SkinType::make([]() { return std::make_shared<DefaultBasicButtonSkin>(); }));
        manager->registerSkin("default-button-skin", SkinType::make([]() { return std::make_shared<DefaultButtonSkin>(); }));
        manager->registerSkin("default-checkbox-skin", SkinType::make([]() { return std::make_shared<DefaultCheckBoxSkin>(); }));
        manager->registerSkin("default-scrollbar-skin", SkinType::make([]() { return std::make_shared<DefaultScrollBarSkin>(); }));
        manager->registerSkin("menu-button-skin", SkinType::make([]() { return std::make_shared<DefaultMenuButtonSkin>(); }));
        manager->registerSkin("sub-menu-button-skin", SkinType::make([]() { return std::make_shared<DefaultSubMenuButtonSkin>(); }));
        manager->registerSkin("title-bar-button", SkinType::make([]() { return std::make_shared<TitleBarButtonSkin>(); }));
        manager->registerSkin("slider", SkinType::make([]() { return std::make_shared<SliderRangeSkin>(); }));

        // region Defaults
        manager->style("*")
               .overflow(Overflow::Visible)
               .fontFamily(t.regular.name)
               .fontSize(t.text)
               .lineHeight(t.lineHeight)
               .color(p.text)
               .selectionColor(p.onAccent)
               .selectionBackgroundColor(p.accent)
               .antiAlias(true)
               .textAntiAlias(true);

        manager->style("Component .defaultSkinChrome")
               .borderRadius(m.radius)
               .border(m.border)
               .margin(m.focusWidth - m.border) // Keeps the size when focused
                   //.color(p.text)
               .borderColor(p.muted.withAlpha(0x40))
               .backgroundColor(p.control)
               .contentBackgroundColor(p.background);

        manager->style("Component:hover .defaultSkinChrome")
               .borderColor(p.muted.withAlpha(0x80))
               .backgroundColor(p.controlHover);

        manager->style("Component:active .defaultSkinChrome")
               .borderColor(p.muted.withAlpha(0xC0))
               .backgroundColor(p.accent);

        manager->style("Component:focus .defaultSkinChrome")
               .border(m.focusWidth)
               .margin(0)
               .borderColor(p.accent);

        //manager->style("Component:focus .defaultSkinChrome")
        //        .border(m.focusWidth)
        //        .borderColor(p.accent);

        manager->style("Component:disabled .defaultSkinChrome")
               .backgroundColor(p.muted);

        manager->style("Component:disabled:hover .defaultSkinChrome")
               .backgroundColor(p.muted);

        manager->style("Component:disabled:active .defaultSkinChrome")
               .backgroundColor(p.accent); // TODO: Fade

        // endregion

        // region Typography

        manager->style(".h1")
               .fontFamily(t.light.name)
               .fontSize(t.huge)
               .lineHeight(t.huge);

        manager->style(".h2")
               .fontFamily(t.light.name)
               .fontSize(t.big)
               .lineHeight(t.big);

        manager->style(".h3")
               .fontFamily(t.light.name)
               .fontSize(t.large)
               .lineHeight(t.large);

        manager->style(".h4")
               .fontFamily(t.regular.name)
               .fontSize(t.medium)
               .lineHeight(t.medium);

        manager->style(".h5")
               .fontFamily(t.regular.name)
               .fontSize(t.text)
               .lineHeight(t.text);

        manager->style(".h6")
               .fontFamily(t.regular.name)
               .fontSize(t.small)
               .lineHeight(t.small);

        // endregion

        // region Window
        manager->style("Window")
               .color(p.text)
               .backgroundColor(p.background)
               .border(m.border)
               .borderColor(p.separator);
        // endregion

        // region Title Bar
        manager->style("TitleBar")
               .alignItems(Align::Center)
               .height(48)
               .padding(m.spaceMd)
               .backgroundColor(p.surface)
               .borderBottom(m.border)
               .borderColor(p.separator);

        manager->style("TitleBar Label.title")
               .fontFamily(t.light.name)
               .fontSize(t.large)
               .marginLeft(m.spaceMd)
               .grow(1);

        manager->style("TitleBar Button")
               .skin("title-bar-button")
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

        // region Boxes

        // endregion

        // region Typography

        // endregion

        // region Text Input

        manager->style("TextInput")
               .skin("default-text-input-skin")
               .padding(m.spaceSm);

        manager->style("TextArea")
               .skin("default-text-area-skin")
               .padding(m.spaceSm);

        // endregion

        // region ScrollBars

        manager->style("ScrollBar")
               .skin("default-scrollbar-skin");

        manager->style("ScrollBar.vertical")
               .width(m.scrollBar);

        manager->style("ScrollBar.horizontal")
               .height(m.scrollBar);

        manager->style("ScrollBar .track")
               .backgroundColor(p.background);

        manager->style("ScrollBar .thumb")
               .backgroundColor(p.muted);

        manager->style("ScrollBar.vertical .thumb")
               .marginLeft(1)
               .minHeight(m.control);

        manager->style("ScrollBar.horizontal .thumb")
               .marginTop(1)
               .minWidth(m.control);

        manager->style("ScrollBar:hover .thumb")
               .backgroundColor(p.control);

        manager->style("ScrollBar:active .thumb")
               .backgroundColor(p.accent);

        manager->style("ScrollBar:disabled .thumb")
               .backgroundColor(p.surface);

        manager->style("Scroller ScrollBar.horizontal")
               .marginRight(m.scrollBar);

        // endregion

        // region Buttons
        manager->style("Button")
               .skin("default-button-skin")
               .cursor(Cursor::Hand)
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .padding(m.spaceMd)
               .fontSize(t.medium)
               .lineHeight(t.medium)
               .color(p.text)
               .backgroundColor(p.control);

        manager->style("Button:hover")
               .backgroundColor(p.controlHover);

        manager->style("Button:active")
               .color(p.onAccent)
               .backgroundColor(p.accent);

        manager->style("Button:disabled")
               .backgroundColor(p.muted);

        manager->style("Button:disabled:hover")
               .backgroundColor(p.muted);

        manager->style("Button:disabled:active")
               .color(p.onAccent)
               .backgroundColor(p.accent); // TODO: Fade

        // endregion

        // region CheckBox
        manager->style("CheckBox")
               .skin("default-checkbox-skin")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .padding(m.spaceMd)
               .color(p.text)
               .fontSize(t.medium)
               .lineHeight(t.medium);

        manager->style("CheckBox Shape")
               .width(m.control)
               .height(m.control)
               .marginRight(m.spaceSm);

        // endregion

        // region Menus
        // A translucent background rather than an opacity, which would fade the labels and highlights too
        manager->style("Menu")
               .backgroundColor(p.background.withAlpha(204))
               .minWidth(200)
               .alignItems(Align::Stretch);

        manager->style("Menu MenuButton")
               .direction(Direction::Row)
               .paddingHorizontal(m.spaceSm)
               .paddingVertical(m.spaceSm)
               .borderRadius(0)
               .backgroundColor(Color{}); // Transparent, over the menu's background

        manager->style("Menu MenuButton.simple")
               .skin("menu-button-skin");

        manager->style("Menu MenuButton.submenu")
               .skin("sub-menu-button-skin");

        manager->style("Menu MenuButton .label")
               .grow(1)
               .fontFamily(t.regular.name)
               .fontSize(t.small)
               .lineHeight(t.text);

        manager->style("Menu MenuButton .shortcut")
               .fontSize(t.small)
               .lineHeight(t.text)
               .color(p.text);

        manager->style("Menu MenuButton.submenu .arrow")
               .width(6);

        manager->style("Menu MenuButton:hover")
               .backgroundColor(p.accent);

        manager->style("Menu MenuButton:hover .label")
               .color(p.onAccent);

        manager->style("Menu MenuButton:hover .shortcut")
               .color(p.onAccentMuted);

        manager->style("Menu MenuButton.submenu:hover .arrow")
               .color(p.onAccent);

        manager->style("Menu MenuButton:active")
               .backgroundColor(p.accent);

        manager->style("Menu MenuButton:active .label")
               .color(p.onAccent);

        manager->style("Menu MenuButton:active .shortcut")
               .color(p.onAccentMuted);

        manager->style("Menu MenuButton.submenu:active .arrow")
               .color(p.onAccent);

        // endregion

        // region Ranges

        manager->style("Range")
               .skin("slider");

        manager->style("Range Slider")
               .borderRadius(m.radius)
               .fontFamily(t.regular.name)
               .fontSize(t.small)
               .lineHeight(t.small)
               .color(p.text);

        manager->style("Range Slider.horizontal")
               .height(m.control);

        manager->style("Range Slider.vertical")
               .width(m.control);

        manager->style("Range:hover Slider.inverted")
               .color(p.onAccent);

        manager->style("Range:hover Slider.normal")
               .color(p.text);

        manager->style("Range:active Slider.normal")
               .color(p.text);

        manager->style("Range:active Slider.inverted")
               .color(p.onAccent);

        // endregion

        // region Tabs
        manager->style("Tabs")
               .direction(Direction::Row)
               .borderBottom(m.border)
               .borderColor(p.separator)
               .backgroundColor(p.muted);

        manager->style("Tabs button")
               .skin("default-basic-button-skin")
               .fontSize(t.small)
               .lineHeight(t.text)
               .marginHorizontal(-m.border)
               .paddingHorizontal(m.spaceMd)
               .paddingVertical(m.spaceSm)
               .borderRadius(0)
               .borderHorizontal(m.border)
               .borderColor(p.separator);

//        manager->style("tabs button:firstChild")
//               .borderRadiusLeft(m.radius);
//
//        manager->style("tabs button:lastChild")
//               .borderRadiusRight(m.radius);

        // endregion

        // region Tab Container

        manager->style("TabContainer")
               .backgroundColor(p.surface);

        // endregion

        // region MenuBar
        manager->style("MenuBar")
               .direction(Direction::Row)
               .backgroundColor(p.background)
               .borderBottom(m.border)
               .borderColor(p.separator);

        manager->style("MenuBar Button")
               .skin("default-basic-button-skin")
               .fontSize(t.small)
               .lineHeight(t.small)
               .paddingHorizontal(m.spaceMd)
               .paddingVertical(m.spaceSm)
               .borderRadius(0)
               .backgroundColor(p.background);

        manager->style("MenuBar Button:hover")
               .color(p.accent);

        manager->style("MenuBar Button:active")
               .color(p.onAccent) // TODO: Contrasting text
               .backgroundColor(p.accent);

        // endregion

        // region ToolBar
        manager->style("ToolBar")
               .direction(Direction::Row)
               .backgroundColor(p.muted)
               .borderBottom(m.border)
               .borderColor(p.separator);

        manager->style("ToolBar Button")
               .skin("default-basic-button-skin")
               .marginHorizontal(-m.border)
               .paddingHorizontal(m.spaceLg)
               .borderRadius(0)
               .borderHorizontal(m.border)
               .borderColor(p.separator);
        // endregion
    }
};

}
