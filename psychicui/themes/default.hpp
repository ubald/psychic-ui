#pragma once

#include <psychicui/style/StyleManager.hpp>
#include <psychicui/style/StyleSheet.hpp>
#include <psychicui/skins/SliderRangeSkin.hpp>
#include <psychicui/skins/DefaultButtonSkin.hpp>
#include <psychicui/skins/DefaultScrollBarSkin.hpp>
#include <psychicui/skins/DefaultCheckBoxSkin.hpp>
#include <psychicui/skins/DefaultMenuButtonSkin.hpp>
#include <psychicui/skins/TitleBarButtonSkin.hpp>

using namespace psychicui;

#define BASE_00 0xFF151515
#define BASE_01 0xFF202020
#define BASE_02 0xFF303030
#define BASE_03 0xFF505050
#define BASE_04 0xFFB0B0B0
#define BASE_05 0xFFD0D0D0
#define BASE_06 0xFFE0E0E0
#define BASE_07 0xFFF5F5F5

#define BASE_RED 0xFFAC4142
#define BASE_ORANGE 0xFFD28445
#define BASE_YELLOW 0xFFF4BF75
#define BASE_GREEN 0xFF90A959
#define BASE_TURQUOISE 0xFF75B5AA
#define BASE_BLUE 0xFF6A9FB5
#define BASE_VIOLET 0xFFAA759F
#define BASE_BROWN 0xFF8F5536

#define ONE_LIGHT 0xFFABB2BF
#define ONE_MEDIUM 0xFF828997
#define ONE_DARK 0xFF5C6370
#define ONE_CYAN 0xFF56B6C2
#define ONE_BLUE 0xFF61AFEF
#define ONE_PURPLE 0xFFC678DD
#define ONE_GREEN 0xFF98C379
#define ONE_RED_FADED 0xFFE06C75
#define ONE_RED 0xFFBE5046
#define ONE_ORANGE 0xFFD19A66
#define ONE_YELLOW 0xFFE5C07B
#define ONE_FOREGROUND 0xFFABB2BF
#define ONE_BACKGROUND 0xFF282C34
#define ONE_GUTTER 0xFF636D83
#define ONE_BLUE_ACCENT 0xFF528BFF

/**
 * PsychicUI Style
 */
class PsychicUIStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {
        int   text       = 12;
        int   smallText  = 10;
        int   mediumText = 14;
        int   largeText  = 16;
        int   radius     = 6;
        Color highlight  = 0xFF00FFEC;

        manager->loadFont("stan0755", "../res/fonts/stan0755.ttf");
        manager->loadFont("Ubuntu Light", "../res/fonts/Ubuntu/Ubuntu-Light.ttf");
        manager->loadFont("Ubuntu Regular", "../res/fonts/Ubuntu/Ubuntu-Regular.ttf");

        manager->registerSkin(
            "default-button-skin",
            SkinType::make([]() { return std::make_shared<DefaultButtonSkin>(); }));
        manager->registerSkin(
            "default-checkbox-skin",
            SkinType::make([]() { return std::make_shared<DefaultCheckBoxSkin>(); }));
        manager->registerSkin(
            "default-scrollbar-skin",
            SkinType::make([]() { return std::make_shared<DefaultScrollBarSkin>(); }));
        manager->registerSkin(
            "menu-button-skin",
            SkinType::make([]() { return std::make_shared<DefaultMenuButtonSkin>(); }));
        manager->registerSkin(
            "sub-menu-button-skin",
            SkinType::make([]() { return std::make_shared<DefaultSubMenuButtonSkin>(); }));
        manager->registerSkin(
            "title-bar-button",
            SkinType::make([]() { return std::make_shared<TitleBarButtonSkin>(); }));
        manager->registerSkin(
            "slider",
            SkinType::make([]() { return std::make_shared<SliderRangeSkin>(); }));

        // region Defaults
        manager->style("*")
               ->set(overflow, "visible")
               ->set(fontFamily, "Ubuntu Light")
               ->set(fontSize, text)
               ->set(lineHeight, 16)
               ->set(color, 0xFFFFFFFF)
               ->set(antiAlias, true)
               ->set(textAntiAlias, true);
        // endregion

        // region Window
        manager->style("Window")
               ->set(color, 0xFFE0E0E0)
               ->set(backgroundColor, BASE_00)
               ->set(border, 1)
               ->set(borderColor, BASE_00);
        // endregion

        // region Title Bar
        manager->style("TitleBar")
               ->set(alignItems, "center")
               ->set(height, 48)
               ->set(padding, 12)
               ->set(backgroundColor, BASE_02)
               ->set(borderBottom, 1)
               ->set(borderColor, BASE_00);

        manager->style("TitleBar Label.title")
               ->set(fontSize, largeText)
               ->set(grow, 1);

        manager->style("TitleBar Button")
               ->set(skin, "title-bar-button")
               ->set(width, 18)
               ->set(height, 18)
               ->set(margin, 3)
               ->set(padding, 0)
               ->set(border, 0);

        manager->style("TitleBar Button.close")
               ->set(backgroundColor, ONE_RED);

        manager->style("TitleBar Button.close:hover")
               ->set(backgroundColor, ONE_RED_FADED);

        manager->style("TitleBar Button.minimize")
               ->set(backgroundColor, ONE_ORANGE);

        manager->style("TitleBar Button.minimize:hover")
               ->set(backgroundColor, ONE_YELLOW);

        manager->style("TitleBar Button.maximize")
               ->set(backgroundColor, BASE_GREEN);

        manager->style("TitleBar Button.maximize:hover")
               ->set(backgroundColor, ONE_GREEN);

        // endregion

        // region Labels

        // endregion

        // region ScrollBars

        manager->style("ScrollBar")
               ->set(skin, "default-scrollbar-skin")
               ->set(width, 12);

        manager->style("ScrollBar .track")
               ->set(backgroundColor, BASE_00);

        manager->style("ScrollBar .thumb")
               ->set(backgroundColor, BASE_02)
               ->set(marginLeft, 1);


        manager->style("ScrollBar:hover .thumb")
               ->set(backgroundColor, BASE_03);


        manager->style("ScrollBar:active .thumb")
               ->set(backgroundColor, highlight);

        // endregion

        // region Buttons
        manager->style("Button")
               ->set(skin, "default-button-skin")
               ->set(cursor, Cursor::Hand)
               ->set(direction, "row")
               ->set(alignItems, "center")
               ->set(padding, 12)
               ->set(borderRadius, radius)
               ->set(backgroundColor, BASE_02)
               ->set(color, BASE_07)
               ->set(fontSize, mediumText)
               ->set(lineHeight, mediumText);

        manager->style("Button:hover")
               ->set(backgroundColor, BASE_03);

        manager->style("Button:active")
               ->set(color, BASE_00)
               ->set(backgroundColor, highlight);
        // endregion

        // region CheckBox
        manager->style("CheckBox")
               ->set(skin, "default-checkbox-skin")
               ->set(direction, "row")
               ->set(alignItems, "center")
               ->set(padding, 12)
               ->set(borderRadius, radius)
               ->set(color, BASE_07)
               ->set(fontSize, mediumText)
               ->set(lineHeight, mediumText);

        manager->style("CheckBox Shape")
               ->set(width, 24)
               ->set(height, 24)
               ->set(marginRight, 6)
               ->set(borderRadius, radius)
               ->set(borderColor, BASE_02)
               ->set(backgroundColor, BASE_00);

        manager->style("CheckBox:hover Shape")
               ->set(color, BASE_03);

        manager->style("CheckBox:active Shape")
               ->set(color, highlight);
        // endregion

        // region Menus
        manager->style("Menu")
               ->set(backgroundColor, 0xFF000000)
               ->set(opacity, 0.8f)
               ->set(minWidth, 200)
               ->set(alignItems, "stretch");

        manager->style("Menu MenuButton")
               ->set(direction, "row")
               ->set(paddingHorizontal, 6)
               ->set(paddingVertical, 6)
               ->set(borderRadius, 0)
               ->set(backgroundColor, 0x00000000);

        manager->style("Menu MenuButton.simple")
               ->set(skin, "menu-button-skin");

        manager->style("Menu MenuButton.submenu")
               ->set(skin, "sub-menu-button-skin");

        manager->style("Menu MenuButton .label")
               ->set(grow, 1)
               ->set(fontFamily, "Ubuntu Regular")
               ->set(fontSize, smallText)
               ->set(lineHeight, text);

        manager->style("Menu MenuButton .shortcut")
               ->set(fontSize, smallText)
               ->set(lineHeight, text)
               ->set(color, BASE_04);

        manager->style("Menu MenuButton.submenu .arrow")
               ->set(width, 6);

        manager->style("Menu MenuButton:hover")
               ->set(backgroundColor, highlight);

        manager->style("Menu MenuButton:hover .label")
               ->set(color, BASE_00);

        manager->style("Menu MenuButton:hover .shortcut")
               ->set(color, BASE_03);

        manager->style("Menu MenuButton.submenu:hover .arrow")
               ->set(color, BASE_00);

        manager->style("Menu MenuButton:active")
               ->set(backgroundColor, highlight);

        manager->style("Menu MenuButton:active .label")
               ->set(color, BASE_00);

        manager->style("Menu MenuButton:active .shortcut")
               ->set(color, BASE_03);

        manager->style("Menu MenuButton.submenu:active .arrow")
               ->set(color, BASE_00);




        // endregion

        // region Ranges
        manager->style("Range")
               ->set(skin, "slider")
               ->set(borderRadius, radius)
               ->set(border, 1)
               ->set(padding, 1)
               ->set(borderColor, BASE_02)
               ->set(backgroundColor, BASE_00)
               ->set(fontFamily, "Ubuntu Regular")
               ->set(fontSize, smallText)
               ->set(lineHeight, smallText)
               ->set(color, BASE_04);

        manager->style("Range Slider.horizontal")
               ->set(height, 24);

        manager->style("Range Slider.vertical")
               ->set(width, 24);

        manager->style("Range:hover Slider.inverted")
               ->set(color, BASE_00);

        manager->style("Range:active Slider.inverted")
               ->set(color, BASE_00);

        manager->style("Range:hover Slider")
               ->set(color, BASE_05);

        manager->style("Range Slider .track")
               ->set(backgroundColor, BASE_01);

        manager->style("Range:hover Slider .track");

        manager->style("Range Slider .range")
               ->set(backgroundColor, BASE_02);

        manager->style("Range:hover Slider .range")
               ->set(backgroundColor, BASE_03);

        manager->style("Range:active Slider .range")
               ->set(backgroundColor, highlight);
        // endregion

        // region Tabs
        manager->style("tabs")
               ->set(direction, "row")
               ->set(borderBottom, 1)
               ->set(borderColor, BASE_00);

        manager->style("tabs button")
               ->set(fontSize, smallText)
               ->set(lineHeight, text)
               ->set(marginHorizontal, -1)
               ->set(paddingHorizontal, 12)
               ->set(paddingVertical, 6)
               ->set(borderRadius, 0)
               ->set(borderHorizontal, 1)
               ->set(borderColor, BASE_00);

//        manager->style("tabs button:firstChild")
//               ->set(borderRadiusLeft, radius);
//
//        manager->style("tabs button:lastChild")
//               ->set(borderRadiusRight, radius);

        // endregion

        // region Tab Container

        manager->style("tabcontainer")
               ->set(backgroundColor, BASE_01);

        // endregion

        // region MenuBar
        manager->style("MenuBar")
               ->set(direction, "row")
               ->set(backgroundColor, BASE_01)
               ->set(borderBottom, 1)
               ->set(borderColor, BASE_00);

        manager->style("MenuBar Button")
               ->set(fontSize, smallText)
               ->set(lineHeight, smallText)
               ->set(paddingHorizontal, 12)
               ->set(paddingVertical, 6)
               ->set(borderRadius, 0)
               ->set(backgroundColor, 0x00000000);

        manager->style("MenuBar Button:hover")
               ->set(color, highlight);

        manager->style("MenuBar Button:active")
               ->set(color, BASE_00)
               ->set(backgroundColor, highlight);

        // endregion

        // region ToolBar
        manager->style("toolbar")
               ->set(direction, "row")
               ->set(backgroundColor, BASE_02)
               ->set(borderBottom, 1)
               ->set(borderColor, BASE_00);

        manager->style("toolbar button")
               ->set(marginHorizontal, -1)
               ->set(paddingHorizontal, 18)
               ->set(borderRadius, 0)
               ->set(borderHorizontal, 1)
               ->set(borderColor, BASE_00);
        // endregion
    }
};
