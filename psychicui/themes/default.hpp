#pragma once

#include <psychicui/style/StyleManager.hpp>
#include <psychicui/style/StyleSheet.hpp>
#include <psychicui/skins/SliderRangeSkin.hpp>
#include <psychicui/skins/SimpleButtonSkin.hpp>
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

        manager->registerSkin(
            "simple-button-skin",
            SkinType::make([]() { return std::make_shared<SimpleButtonSkin>(); }));
        manager->registerSkin(
            "title-bar-button",
            SkinType::make([]() { return std::make_shared<TitleBarButtonSkin>(); }));
        manager->registerSkin(
            "slider",
            SkinType::make([]() { return std::make_shared<SliderRangeSkin>(); }));

        // region Defaults
        manager->style("*")
               ->set(fontFamily, "Ubuntu Light")
               ->set(fontSize, text)
               ->set(lineHeight, 16)
               ->set(color, 0xFFFFFFFF)
               ->set(textAntiAlias, true);
        // endregion

        // region Window
        manager->style("window")
               ->set(color, 0xFFE0E0E0)
               ->set(backgroundColor, BASE_00)
               ->set(border, 1)
               ->set(borderColor, BASE_00);
        // endregion

        // region Title Bar
        manager->style("titlebar")
               ->set(alignItems, "center")
               ->set(height, 48)
               ->set(padding, 12)
               ->set(backgroundColor, BASE_02)
               ->set(borderBottom, 1)
               ->set(borderColor, BASE_00);

        manager->style("titlebar label.title")
               ->set(fontSize, largeText)
               ->set(grow, 1);

        manager->style("titlebar button")
               ->set(skin, "title-bar-button")
               ->set(width, 18)
               ->set(height, 18)
               ->set(margin, 3)
               ->set(padding, 0)
               ->set(border, 0);

        manager->style("titlebar button.close")
               ->set(backgroundColor, ONE_RED);

        manager->style("titlebar button.close:hover")
               ->set(backgroundColor, ONE_RED_FADED);

        manager->style("titlebar button.minimize")
               ->set(backgroundColor, ONE_ORANGE);

        manager->style("titlebar button.minimize:hover")
               ->set(backgroundColor, ONE_YELLOW);

        manager->style("titlebar button.maximize")
               ->set(backgroundColor, BASE_GREEN);

        manager->style("titlebar button.maximize:hover")
               ->set(backgroundColor, ONE_GREEN);

        // endregion

        // region Labels

        // endregion

        // region Buttons
        manager->style("button")
               ->set(skin, "simple-button-skin")
               ->set(direction, "row")
               ->set(alignItems, "center")
               ->set(padding, 12)
               ->set(borderRadius, radius)
               ->set(backgroundColor, BASE_02)
               ->set(color, BASE_07)
               ->set(fontSize, mediumText)
               ->set(lineHeight, mediumText);

        manager->style("button:hover")
               ->set(backgroundColor, BASE_03);

        manager->style("button:active")
               ->set(color, BASE_00)
               ->set(backgroundColor, highlight);
        // endregion

        // region Menus

        manager->style("menu")
               ->set(backgroundColor, 0xFF000000)
               ->set(opacity, 0.8f)
               ->set(minWidth, 200)
               ->set(alignItems, "stretch");

        manager->style("menu button")
               ->set(fontSize, smallText)
               ->set(lineHeight, text)
               ->set(paddingHorizontal, 6)
               ->set(paddingVertical, 6)
               ->set(borderRadius, 0)
               ->set(backgroundColor, 0x00000000);

        manager->style("menu button:hover")
               ->set(color, BASE_00)
               ->set(backgroundColor, highlight);


        // endregion

        // region Ranges
        manager->style("range")
               ->set(skin, "slider")
               ->set(borderRadius, radius)
               ->set(border, 1)
               ->set(padding, 1)
               ->set(overflow, "hidden")
               ->set(borderColor, BASE_02)
               ->set(backgroundColor, BASE_00)
               ->set(fontSize, smallText)
               ->set(color, BASE_04);

        manager->style("range slider.horizontal")
               ->set(height, 24);

        manager->style("range slider.vertical")
               ->set(width, 24);

        manager->style("range:hover slider.inverted")
               ->set(color, BASE_00);

        manager->style("range:active slider.inverted")
               ->set(color, BASE_00);

        manager->style("range:hover slider")
               ->set(color, BASE_05);

        manager->style("range slider .track")
               ->set(backgroundColor, BASE_01);

        manager->style("range:hover slider .track");

        manager->style("range slider .range")
               ->set(backgroundColor, BASE_02);

        manager->style("range:hover slider .range")
               ->set(backgroundColor, BASE_03);

        manager->style("range:active slider .range")
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
        manager->style("menubar")
               ->set(direction, "row")
               ->set(backgroundColor, BASE_01)
               ->set(borderBottom, 1)
               ->set(borderColor, BASE_00);

        manager->style("menubar button")
               ->set(fontSize, smallText)
               ->set(lineHeight, smallText)
               ->set(paddingHorizontal, 12)
               ->set(paddingVertical, 6)
               ->set(borderRadius, 0)
               ->set(backgroundColor, 0x00000000);

        manager->style("menubar button:hover")
               ->set(color, highlight);

        manager->style("menubar button:active")
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