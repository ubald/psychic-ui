#pragma once

#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/StyleSheet.hpp>
#include <psychic-ui/skins/SliderRangeSkin.hpp>
#include <psychic-ui/skins/DefaultButtonSkin.hpp>
#include <psychic-ui/skins/DefaultScrollBarSkin.hpp>
#include <psychic-ui/skins/DefaultCheckBoxSkin.hpp>
#include <psychic-ui/skins/DefaultMenuButtonSkin.hpp>
#include <psychic-ui/skins/TitleBarButtonSkin.hpp>
#include <psychic-color/spaces/RGB.hpp>
#include <psychic-color/spaces/HSB.hpp>
#include <psychic-ui/skins/DefaultBasicButtonSkin.hpp>

using namespace psychic_ui;
using namespace psychic_color;

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
 * Psychic UI Style
 */
class PsychicUIStyleSheet : public StyleSheet {
protected:
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
        int smallText     = 10;
        int text          = 12;
        int mediumText    = 14;
        int largeText     = 18;
        int bigText       = 24;
        int hugeText      = 36;
        int radius        = 7;
        int scrollBarSize = 12;

        manager->loadFont("stan0755", "../res/fonts/stan0755.ttf");
        manager->loadFont("Ubuntu Light", "../res/fonts/Ubuntu/Ubuntu-Light.ttf");
        manager->loadFont("Ubuntu Regular", "../res/fonts/Ubuntu/Ubuntu-Regular.ttf");

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
               ->set(overflow, "visible")
               ->set(fontFamily, "Ubuntu Regular")
               ->set(fontSize, text)
               ->set(lineHeight, 16)
               ->set(color, themeForegroundColor.getColorAlpha())
               ->set(antiAlias, true)
               ->set(textAntiAlias, true)
               ->set(multiline, false);

        manager->style("Component .defaultSkinChrome")
               ->set(borderRadius, radius)
               ->set(color, themeForegroundColor.getColorAlpha())
               ->set(borderColor, themeLowContrastColor.getColorAlpha()) // TODO: Should be contrasting with alpha
               ->set(backgroundColor, themeMediumContrastColor.getColorAlpha())
               ->set(contentBackgroundColor, themeBackgroundColor.getColorAlpha());

        manager->style("Component:hover .defaultSkinChrome")
               ->set(backgroundColor, themeHighContrastColor.getColorAlpha());

        manager->style("Component:active .defaultSkinChrome")
               ->set(color, BASE_00)
               ->set(backgroundColor, themeHighlightColor.getColorAlpha());

        manager->style("Component:disabled .defaultSkinChrome")
               ->set(backgroundColor, themeLowContrastColor.getColorAlpha());

        manager->style("Component:disabled:hover .defaultSkinChrome")
               ->set(backgroundColor, themeLowContrastColor.getColorAlpha());

        manager->style("Component:disabled:active .defaultSkinChrome")
               ->set(color, BASE_00)
               ->set(backgroundColor, themeHighlightColor.getColorAlpha()); // TODO: Fade

        // endregion

        // region Typography

        manager->style(".h1")
               ->set(fontFamily, "Ubuntu Light")
               ->set(fontSize, hugeText)
               ->set(lineHeight, hugeText);

        manager->style(".h2")
               ->set(fontFamily, "Ubuntu Light")
               ->set(fontSize, bigText)
               ->set(lineHeight, bigText);

        manager->style(".h3")
               ->set(fontFamily, "Ubuntu Light")
               ->set(fontSize, largeText)
               ->set(lineHeight, largeText);

        manager->style(".h4")
               ->set(fontFamily, "Ubuntu Regular")
               ->set(fontSize, mediumText)
               ->set(lineHeight, mediumText);

        manager->style(".h5")
               ->set(fontFamily, "Ubuntu Regular")
               ->set(fontSize, text)
               ->set(lineHeight, text);

        manager->style(".h6")
               ->set(fontFamily, "Ubuntu Regular")
               ->set(fontSize, smallText)
               ->set(lineHeight, smallText);

        // endregion

        // region Window
        manager->style("Window")
               ->set(color, themeForegroundColor.getColorAlpha())
               ->set(backgroundColor, themeBackgroundColor.getColorAlpha())
               ->set(border, 1)
               ->set(borderColor, themeSeparatorColor.getColorAlpha());
        // endregion

        // region Title Bar
        manager->style("TitleBar")
               ->set(alignItems, "center")
               ->set(height, 48)
               ->set(padding, 12)
               ->set(backgroundColor, themeBaseColor.getColorAlpha())
               ->set(borderBottom, 1)
               ->set(borderColor, themeSeparatorColor.getColorAlpha());

        manager->style("TitleBar Label.title")
               ->set(fontFamily, "Ubuntu Light")
               ->set(fontSize, largeText)
               ->set(marginLeft, 12)
               ->set(grow, 1);

        manager->style("TitleBar Button")
               ->set(skin, "title-bar-button")
               ->set(width, 18)
               ->set(height, 18)
               ->set(margin, 3)
               ->set(padding, 0)
               ->set(border, 0);

        manager->style("TitleBar Button.close")
               ->set(backgroundColor, 0xffff5f57);

        manager->style("TitleBar Button.close:hover")
               ->set(backgroundColor, 0xffff5f57);

        manager->style("TitleBar Button.minimize")
               ->set(backgroundColor, 0xffffbd2f);

        manager->style("TitleBar Button.minimize:hover")
               ->set(backgroundColor, 0xffffbd2f);

        manager->style("TitleBar Button.maximize")
               ->set(backgroundColor, 0xff28c940);

        manager->style("TitleBar Button.maximize:hover")
               ->set(backgroundColor, 0xff28c940);

        // endregion

        // region Boxes

        // endregion

        // region Labels

        // endregion

        // region ScrollBars

        manager->style("ScrollBar")
               ->set(skin, "default-scrollbar-skin");

        manager->style("ScrollBar.vertical")
               ->set(width, scrollBarSize);

        manager->style("ScrollBar.horizontal")
               ->set(height, scrollBarSize);

        manager->style("ScrollBar .track")
               ->set(backgroundColor, themeBackgroundColor.getColorAlpha());

        manager->style("ScrollBar .thumb")
               ->set(backgroundColor, themeLowContrastColor.getColorAlpha());

        manager->style("ScrollBar.vertical .thumb")
               ->set(marginLeft, 1)
               ->set(minHeight, 24);

        manager->style("ScrollBar.horizontal .thumb")
               ->set(marginTop, 1)
               ->set(minWidth, 24);

        manager->style("ScrollBar:hover .thumb")
               ->set(backgroundColor, themeMediumContrastColor.getColorAlpha());

        manager->style("ScrollBar:active .thumb")
               ->set(backgroundColor, themeHighlightColor.getColorAlpha());

        manager->style("ScrollBar:disabled .thumb")
               ->set(backgroundColor, themeBaseColor.getColorAlpha());

        manager->style("Scroller ScrollBar.horizontal")
               ->set(marginRight, scrollBarSize);

        // endregion

        // region Buttons
        manager->style("Button")
               ->set(skin, "default-button-skin")
               ->set(cursor, Cursor::Hand)
               ->set(flexDirection, "row")
               ->set(alignItems, "center")
               ->set(padding, 12)
               ->set(fontSize, mediumText)
               ->set(lineHeight, mediumText)
               ->set(color, themeForegroundColor.getColorAlpha())
               ->set(backgroundColor, themeMediumContrastColor.getColorAlpha());

        manager->style("Button:hover")
               ->set(backgroundColor, themeHighContrastColor.getColorAlpha());

        manager->style("Button:active")
               ->set(color, BASE_00)
               ->set(backgroundColor, themeHighlightColor.getColorAlpha());

        manager->style("Button:disabled")
               ->set(backgroundColor, themeLowContrastColor.getColorAlpha());

        manager->style("Button:disabled:hover")
               ->set(backgroundColor, themeLowContrastColor.getColorAlpha());

        manager->style("Button:disabled:active")
               ->set(color, BASE_00)
               ->set(backgroundColor, themeHighlightColor.getColorAlpha()); // TODO: Fade

        // endregion

        // region CheckBox
        manager->style("CheckBox")
               ->set(skin, "default-checkbox-skin")
               ->set(flexDirection, "row")
               ->set(alignItems, "center")
               ->set(padding, 12)
               ->set(color, themeForegroundColor.getColorAlpha())
               ->set(fontSize, mediumText)
               ->set(lineHeight, mediumText);

        manager->style("CheckBox Shape")
               ->set(width, 24)
               ->set(height, 24)
               ->set(marginRight, 6);

        // endregion

        // region Menus
        manager->style("Menu")
               ->set(backgroundColor, themeBackgroundColor.getColorAlpha())
               ->set(opacity, 0.8f)
               ->set(minWidth, 200)
               ->set(alignItems, "stretch");

        manager->style("Menu MenuButton")
               ->set(flexDirection, "row")
               ->set(paddingHorizontal, 6)
               ->set(paddingVertical, 6)
               ->set(borderRadius, 0)
               ->set(backgroundColor, themeBackgroundColor.getColorAlpha());

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
               ->set(color, themeForegroundColor.getColorAlpha());

        manager->style("Menu MenuButton.submenu .arrow")
               ->set(width, 6);

        manager->style("Menu MenuButton:hover")
               ->set(backgroundColor, themeHighlightColor.getColorAlpha());

        manager->style("Menu MenuButton:hover .label")
               ->set(color, BASE_00); // TODO: Contrasting text

        manager->style("Menu MenuButton:hover .shortcut")
               ->set(color, BASE_03); // TODO: Contrasting text

        manager->style("Menu MenuButton.submenu:hover .arrow")
               ->set(color, BASE_00); // TODO: Contrasting text

        manager->style("Menu MenuButton:active")
               ->set(backgroundColor, themeHighlightColor.getColorAlpha());

        manager->style("Menu MenuButton:active .label")
               ->set(color, BASE_00); // TODO: Contrasting text

        manager->style("Menu MenuButton:active .shortcut")
               ->set(color, BASE_03); // TODO: Contrasting text

        manager->style("Menu MenuButton.submenu:active .arrow")
               ->set(color, BASE_00); // TODO: Contrasting text

        // endregion

        // region Ranges

        manager->style("Range")
               ->set(skin, "slider");

        manager->style("Range Slider")
               ->set(borderRadius, radius)
               ->set(fontFamily, "Ubuntu Regular")
               ->set(fontSize, smallText)
               ->set(lineHeight, smallText)
               ->set(color, themeForegroundColor.getColorAlpha());

        manager->style("Range Slider.horizontal")
               ->set(height, 24);

        manager->style("Range Slider.vertical")
               ->set(width, 24);

        manager->style("Range:hover Slider.inverted")
               ->set(color, BASE_00); // TODO: Contrasting text

        manager->style("Range:hover Slider.normal")
               ->set(color, themeForegroundColor.getColorAlpha()); // TODO: Contrasting text

        manager->style("Range:active Slider.normal")
               ->set(color, themeForegroundColor.getColorAlpha()); // TODO: Contrasting text

        manager->style("Range:active Slider.inverted")
               ->set(color, BASE_00); // TODO: Contrasting text

        // endregion

        // region Tabs
        manager->style("Tabs")
               ->set(flexDirection, "row")
               ->set(borderBottom, 1)
               ->set(borderColor, themeSeparatorColor.getColorAlpha())
               ->set(backgroundColor, themeLowContrastColor.getColorAlpha());

        manager->style("Tabs button")
               ->set(skin, "default-basic-button-skin")
               ->set(fontSize, smallText)
               ->set(lineHeight, text)
               ->set(marginHorizontal, -1)
               ->set(paddingHorizontal, 12)
               ->set(paddingVertical, 6)
               ->set(borderRadius, 0)
               ->set(borderHorizontal, 1)
               ->set(borderColor, themeSeparatorColor.getColorAlpha());

//        manager->style("tabs button:firstChild")
//               ->set(borderRadiusLeft, radius);
//
//        manager->style("tabs button:lastChild")
//               ->set(borderRadiusRight, radius);

        // endregion

        // region Tab Container

        manager->style("TabContainer")
               ->set(backgroundColor, themeBaseColor.getColorAlpha());

        // endregion

        // region MenuBar
        manager->style("MenuBar")
               ->set(flexDirection, "row")
               ->set(backgroundColor, themeBackgroundColor.getColorAlpha())
               ->set(borderBottom, 1)
               ->set(borderColor, themeSeparatorColor.getColorAlpha());

        manager->style("MenuBar Button")
               ->set(skin, "default-basic-button-skin")
               ->set(fontSize, smallText)
               ->set(lineHeight, smallText)
               ->set(paddingHorizontal, 12)
               ->set(paddingVertical, 6)
               ->set(borderRadius, 0)
               ->set(backgroundColor, themeBackgroundColor.getColorAlpha());

        manager->style("MenuBar Button:hover")
               ->set(color, themeHighlightColor.getColorAlpha());

        manager->style("MenuBar Button:active")
               ->set(color, BASE_00) // TODO: Contrasting text
               ->set(backgroundColor, themeHighlightColor.getColorAlpha());

        // endregion

        // region ToolBar
        manager->style("ToolBar")
               ->set(flexDirection, "row")
               ->set(backgroundColor, themeLowContrastColor.getColorAlpha())
               ->set(borderBottom, 1)
               ->set(borderColor, themeSeparatorColor.getColorAlpha());

        manager->style("ToolBar Button")
               ->set(skin, "default-basic-button-skin")
               ->set(marginHorizontal, -1)
               ->set(paddingHorizontal, 18)
               ->set(borderRadius, 0)
               ->set(borderHorizontal, 1)
               ->set(borderColor, themeSeparatorColor.getColorAlpha());
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
        themeSeparatorColor      = HSB{0x181A1F};
        themeBackgroundColor     = HSB{0x1B1D23};
        themeBaseColor           = HSB{0x21252B};
        themeLowContrastColor    = HSB{0x282C34};
        themeMediumContrastColor = HSB{0x383D48};
        themeHighContrastColor   = HSB{0x3C414F};
        themeForegroundColor     = HSB{0x9DA5B5};
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
