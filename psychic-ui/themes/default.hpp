#pragma once

#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/StyleSheet.hpp>
#include <psychic-ui/skins/SliderRangeSkin.hpp>
#include <psychic-ui/skins/DefaultButtonSkin.hpp>
#include <psychic-ui/skins/DefaultScrollBarSkin.hpp>
#include <psychic-ui/skins/DefaultCheckBoxSkin.hpp>
#include <psychic-ui/skins/DefaultMenuButtonSkin.hpp>
#include <psychic-ui/skins/TitleBarButtonSkin.hpp>
#include <psychic-ui/skins/DefaultBasicButtonSkin.hpp>
#include <psychic-ui/skins/DefaultTextInputSkin.hpp>
#include <psychic-ui/skins/DefaultTextAreaSkin.hpp>
#include <psychic-color/spaces/RGB.hpp>
#include <psychic-color/spaces/HSB.hpp>

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
 * Psychic UI Style
 */
class PsychicUIStyleSheet : public StyleSheet {
protected:
    using HSB = psychic_color::HSB;

    HSB themeSeparatorColor{}; // Deepest level
    HSB themeBackgroundColor{}; // Inputs & content backgrounds
    HSB themeBaseColor{}; // Base color, panel backgrounds
    HSB themeLowContrastColor{};
    HSB themeMediumContrastColor{};
    HSB themeHighContrastColor{};
    HSB themeForegroundColor{}; // Base foreground color
    HSB themeHighlightColor{}; // Color used for active statuses
    HSB themeCyanColor{};
    HSB themeBlueColor{};
    HSB themePurpleColor{};
    HSB themeGreenColor{};
    HSB themeRedColor{};
    HSB themeRedOrangeColor{};
    HSB themeOrangeColor{};
    HSB themeYellowColor{};
public:
    PsychicUIStyleSheet() = default;

    void load(StyleManager *manager) override {
        float smallText     = 10.0;
        float text          = 12.0;
        float mediumText    = 14.0;
        float largeText     = 18.0;
        float bigText       = 24.0;
        float hugeText      = 36.0;
        float radius        = 7.0;
        float scrollBarSize = 12.0;

        manager->loadFont("Ubuntu Light", "../res/fonts/Ubuntu/Ubuntu-Light.ttf");
        manager->loadFont("Ubuntu Regular", "../res/fonts/Ubuntu/Ubuntu-Regular.ttf");

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
               .fontFamily("Ubuntu Regular")
               .fontSize(text)
               .lineHeight(16)
               .color(themeForegroundColor)
               .selectionColor(palette::base00)
               .selectionBackgroundColor(themeHighlightColor)
               .antiAlias(true)
               .textAntiAlias(true);

        manager->style("Component .defaultSkinChrome")
               .borderRadius(radius)
               .border(1)
               .margin(1)
                   //.color(themeForegroundColor)
               .borderColor(Color(themeLowContrastColor).withAlpha(0x40)) // TODO: Should be contrasting with alpha
               .backgroundColor(themeMediumContrastColor)
               .contentBackgroundColor(themeBackgroundColor);

        manager->style("Component:hover .defaultSkinChrome")
               .borderColor(Color(themeLowContrastColor).withAlpha(0x80)) // TODO: Should be contrasting with alpha
               .backgroundColor(themeHighContrastColor);

        manager->style("Component:active .defaultSkinChrome")
               .borderColor(Color(themeLowContrastColor).withAlpha(0xC0)) // TODO: Should be contrasting with alpha
               .backgroundColor(themeHighlightColor);

        manager->style("Component:focus .defaultSkinChrome")
               .border(2)
               .margin(0)
               .borderColor(themeHighlightColor);

        //manager->style("Component:focus .defaultSkinChrome")
        //        .border(2)
        //        .borderColor(themeHighlightColor);

        manager->style("Component:disabled .defaultSkinChrome")
               .backgroundColor(themeLowContrastColor);

        manager->style("Component:disabled:hover .defaultSkinChrome")
               .backgroundColor(themeLowContrastColor);

        manager->style("Component:disabled:active .defaultSkinChrome")
               .backgroundColor(themeHighlightColor); // TODO: Fade

        // endregion

        // region Typography

        manager->style(".h1")
               .fontFamily("Ubuntu Light")
               .fontSize(hugeText)
               .lineHeight(hugeText);

        manager->style(".h2")
               .fontFamily("Ubuntu Light")
               .fontSize(bigText)
               .lineHeight(bigText);

        manager->style(".h3")
               .fontFamily("Ubuntu Light")
               .fontSize(largeText)
               .lineHeight(largeText);

        manager->style(".h4")
               .fontFamily("Ubuntu Regular")
               .fontSize(mediumText)
               .lineHeight(mediumText);

        manager->style(".h5")
               .fontFamily("Ubuntu Regular")
               .fontSize(text)
               .lineHeight(text);

        manager->style(".h6")
               .fontFamily("Ubuntu Regular")
               .fontSize(smallText)
               .lineHeight(smallText);

        // endregion

        // region Window
        manager->style("Window")
               .color(themeForegroundColor)
               .backgroundColor(themeBackgroundColor)
               .border(1)
               .borderColor(themeSeparatorColor);
        // endregion

        // region Title Bar
        manager->style("TitleBar")
               .alignItems(Align::Center)
               .height(48)
               .padding(12)
               .backgroundColor(themeBaseColor)
               .borderBottom(1)
               .borderColor(themeSeparatorColor);

        manager->style("TitleBar Label.title")
               .fontFamily("Ubuntu Light")
               .fontSize(largeText)
               .marginLeft(12)
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
               .padding(6);

        manager->style("TextArea")
               .skin("default-text-area-skin")
               .padding(6);

        // endregion

        // region ScrollBars

        manager->style("ScrollBar")
               .skin("default-scrollbar-skin");

        manager->style("ScrollBar.vertical")
               .width(scrollBarSize);

        manager->style("ScrollBar.horizontal")
               .height(scrollBarSize);

        manager->style("ScrollBar .track")
               .backgroundColor(themeBackgroundColor);

        manager->style("ScrollBar .thumb")
               .backgroundColor(themeLowContrastColor);

        manager->style("ScrollBar.vertical .thumb")
               .marginLeft(1)
               .minHeight(24);

        manager->style("ScrollBar.horizontal .thumb")
               .marginTop(1)
               .minWidth(24);

        manager->style("ScrollBar:hover .thumb")
               .backgroundColor(themeMediumContrastColor);

        manager->style("ScrollBar:active .thumb")
               .backgroundColor(themeHighlightColor);

        manager->style("ScrollBar:disabled .thumb")
               .backgroundColor(themeBaseColor);

        manager->style("Scroller ScrollBar.horizontal")
               .marginRight(scrollBarSize);

        // endregion

        // region Buttons
        manager->style("Button")
               .skin("default-button-skin")
               .cursor(Cursor::Hand)
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .padding(12)
               .fontSize(mediumText)
               .lineHeight(mediumText)
               .color(themeForegroundColor)
               .backgroundColor(themeMediumContrastColor);

        manager->style("Button:hover")
               .backgroundColor(themeHighContrastColor);

        manager->style("Button:active")
               .color(palette::base00)
               .backgroundColor(themeHighlightColor);

        manager->style("Button:disabled")
               .backgroundColor(themeLowContrastColor);

        manager->style("Button:disabled:hover")
               .backgroundColor(themeLowContrastColor);

        manager->style("Button:disabled:active")
               .color(palette::base00)
               .backgroundColor(themeHighlightColor); // TODO: Fade

        // endregion

        // region CheckBox
        manager->style("CheckBox")
               .skin("default-checkbox-skin")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .padding(12)
               .color(themeForegroundColor)
               .fontSize(mediumText)
               .lineHeight(mediumText);

        manager->style("CheckBox Shape")
               .width(24)
               .height(24)
               .marginRight(6);

        // endregion

        // region Menus
        manager->style("Menu")
               .backgroundColor(themeBackgroundColor)
               .opacity(0.8f)
               .minWidth(200)
               .alignItems(Align::Stretch);

        manager->style("Menu MenuButton")
               .direction(Direction::Row)
               .paddingHorizontal(6)
               .paddingVertical(6)
               .borderRadius(0)
               .backgroundColor(themeBackgroundColor);

        manager->style("Menu MenuButton.simple")
               .skin("menu-button-skin");

        manager->style("Menu MenuButton.submenu")
               .skin("sub-menu-button-skin");

        manager->style("Menu MenuButton .label")
               .grow(1)
               .fontFamily("Ubuntu Regular")
               .fontSize(smallText)
               .lineHeight(text);

        manager->style("Menu MenuButton .shortcut")
               .fontSize(smallText)
               .lineHeight(text)
               .color(themeForegroundColor);

        manager->style("Menu MenuButton.submenu .arrow")
               .width(6);

        manager->style("Menu MenuButton:hover")
               .backgroundColor(themeHighlightColor);

        manager->style("Menu MenuButton:hover .label")
               .color(palette::base00); // TODO: Contrasting text

        manager->style("Menu MenuButton:hover .shortcut")
               .color(palette::base03); // TODO: Contrasting text

        manager->style("Menu MenuButton.submenu:hover .arrow")
               .color(palette::base00); // TODO: Contrasting text

        manager->style("Menu MenuButton:active")
               .backgroundColor(themeHighlightColor);

        manager->style("Menu MenuButton:active .label")
               .color(palette::base00); // TODO: Contrasting text

        manager->style("Menu MenuButton:active .shortcut")
               .color(palette::base03); // TODO: Contrasting text

        manager->style("Menu MenuButton.submenu:active .arrow")
               .color(palette::base00); // TODO: Contrasting text

        // endregion

        // region Ranges

        manager->style("Range")
               .skin("slider");

        manager->style("Range Slider")
               .borderRadius(radius)
               .fontFamily("Ubuntu Regular")
               .fontSize(smallText)
               .lineHeight(smallText)
               .color(themeForegroundColor);

        manager->style("Range Slider.horizontal")
               .height(24);

        manager->style("Range Slider.vertical")
               .width(24);

        manager->style("Range:hover Slider.inverted")
               .color(palette::base00); // TODO: Contrasting text

        manager->style("Range:hover Slider.normal")
               .color(themeForegroundColor); // TODO: Contrasting text

        manager->style("Range:active Slider.normal")
               .color(themeForegroundColor); // TODO: Contrasting text

        manager->style("Range:active Slider.inverted")
               .color(palette::base00); // TODO: Contrasting text

        // endregion

        // region Tabs
        manager->style("Tabs")
               .direction(Direction::Row)
               .borderBottom(1)
               .borderColor(themeSeparatorColor)
               .backgroundColor(themeLowContrastColor);

        manager->style("Tabs button")
               .skin("default-basic-button-skin")
               .fontSize(smallText)
               .lineHeight(text)
               .marginHorizontal(-1)
               .paddingHorizontal(12)
               .paddingVertical(6)
               .borderRadius(0)
               .borderHorizontal(1)
               .borderColor(themeSeparatorColor);

//        manager->style("tabs button:firstChild")
//               .borderRadiusLeft(radius);
//
//        manager->style("tabs button:lastChild")
//               .borderRadiusRight(radius);

        // endregion

        // region Tab Container

        manager->style("TabContainer")
               .backgroundColor(themeBaseColor);

        // endregion

        // region MenuBar
        manager->style("MenuBar")
               .direction(Direction::Row)
               .backgroundColor(themeBackgroundColor)
               .borderBottom(1)
               .borderColor(themeSeparatorColor);

        manager->style("MenuBar Button")
               .skin("default-basic-button-skin")
               .fontSize(smallText)
               .lineHeight(smallText)
               .paddingHorizontal(12)
               .paddingVertical(6)
               .borderRadius(0)
               .backgroundColor(themeBackgroundColor);

        manager->style("MenuBar Button:hover")
               .color(themeHighlightColor);

        manager->style("MenuBar Button:active")
               .color(palette::base00) // TODO: Contrasting text
               .backgroundColor(themeHighlightColor);

        // endregion

        // region ToolBar
        manager->style("ToolBar")
               .direction(Direction::Row)
               .backgroundColor(themeLowContrastColor)
               .borderBottom(1)
               .borderColor(themeSeparatorColor);

        manager->style("ToolBar Button")
               .skin("default-basic-button-skin")
               .marginHorizontal(-1)
               .paddingHorizontal(18)
               .borderRadius(0)
               .borderHorizontal(1)
               .borderColor(themeSeparatorColor);
        // endregion
    }
};

/**
 * Psychic UI Style
 */
class PsychicStyleSheet : public PsychicUIStyleSheet {
public:
    PsychicStyleSheet() :
        PsychicUIStyleSheet() {
        themeSeparatorColor      = HSB{0x19181a};
        themeBackgroundColor     = HSB{0x19181a};
        themeBaseColor           = HSB{0x252426};
        themeLowContrastColor    = HSB{0x313033};
        themeMediumContrastColor = HSB{0x3d3c40};
        themeHighContrastColor   = HSB{0x595959};
        themeForegroundColor     = HSB{0xa4a1aa};
        themeHighlightColor      = HSB{0x00FFEC};
        themeCyanColor           = HSB{0x56B6C2};
        themeBlueColor           = HSB{0x61AFEF};
        themePurpleColor         = HSB{0xC678DD};
        themeGreenColor          = HSB{0x98C379};
        themeRedColor            = HSB{0xBE5046};
        themeRedOrangeColor      = HSB{0xE06C75};
        themeOrangeColor         = HSB{0xD19A66};
        themeYellowColor         = HSB{0xE5C07B};
    }
};

/**
 * One Dark UI Style
 */
class OneDarkStyleSheet : public PsychicUIStyleSheet {
public:
    OneDarkStyleSheet() :
        PsychicUIStyleSheet() {
        themeSeparatorColor      = HSB{0x181A1F};
        themeBackgroundColor     = HSB{0x1B1D23};
        themeBaseColor           = HSB{0x21252B};
        themeLowContrastColor    = HSB{0x282C34};
        themeMediumContrastColor = HSB{0x383D48};
        themeHighContrastColor   = HSB{0x3C414F};
        themeForegroundColor     = HSB{0x9DA5B5};
        themeHighlightColor      = HSB{0x0C83EB}; //{0xFF00FFEC}
        themeCyanColor           = HSB{0x56B6C2};
        themeBlueColor           = HSB{0x61AFEF};
        themePurpleColor         = HSB{0xC678DD};
        themeGreenColor          = HSB{0x98C379};
        themeRedColor            = HSB{0xBE5046};
        themeRedOrangeColor      = HSB{0xE06C75};
        themeOrangeColor         = HSB{0xD19A66};
        themeYellowColor         = HSB{0xE5C07B};
    }
};

/**
 * One Light UI Style
 */
class OneLightStyleSheet : public PsychicUIStyleSheet {
public:
    OneLightStyleSheet() :
        PsychicUIStyleSheet() {
        themeSeparatorColor      = HSB{0xc5cad3};
        themeBackgroundColor     = HSB{0xc5cad3};//0xf3f4f6};
        themeBaseColor           = HSB{0xd7dae1};
        themeLowContrastColor    = HSB{0xe8eaed};
        themeMediumContrastColor = HSB{0xebecef};
        themeHighContrastColor   = HSB{0xf6f7f8};
        themeForegroundColor     = HSB{0x2c313b};
        themeHighlightColor      = HSB{0x5a8aed};
        themeCyanColor           = HSB{0x56B6C2};
        themeBlueColor           = HSB{0x61AFEF};
        themePurpleColor         = HSB{0xC678DD};
        themeGreenColor          = HSB{0x98C379};
        themeRedColor            = HSB{0xBE5046};
        themeRedOrangeColor      = HSB{0xE06C75};
        themeOrangeColor         = HSB{0xD19A66};
        themeYellowColor         = HSB{0xE5C07B};
    }
};

}
