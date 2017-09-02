#pragma once

#include <psychicui/style/StyleManager.hpp>
#include <psychicui/style/StyleSheet.hpp>

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

/**
 * PsychicUI Style
 */
class PsychicUIStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {
        manager->loadFont("stan0755", "res/fonts/stan0755.ttf");
        manager->loadFont("Ubuntu Light", "res/fonts/Ubuntu/Ubuntu-Light.ttf");

        // region Defaults
        manager->style("*")
               ->set(fontFamily, "Ubuntu Light")
               ->set(fontSize, 12)
               ->set(lineHeight, 16)
               ->set(color, 0xFFFFFFFF)
               ->set(textAntiAlias, true);
        // endregion

        // region Window
        manager->style("window")
               ->set(color, 0xFFE0E0E0)
               ->set(backgroundColor, BASE_00);
        // endregion

        // region Buttons
        manager->style("button")
               ->set(direction, "row")
               ->set(alignItems, "center")
               ->set(padding, 12)
               ->set(cornerRadius, 6)
               ->set(backgroundColor, BASE_02)
               ->set(color, BASE_07)
               ->set(fontSize, 14)
               ->set(lineHeight, 14);

        manager->style("button:hover")
               ->set(backgroundColor, BASE_03);

       manager->style("button:active")
               ->set(color, BASE_00)
               ->set(backgroundColor, 0xFF00FFEC);
        // endregion

        // region Tabs
        manager->style("tabs")
               ->set(direction, "row")
               ->set(borderBottom, 1)
               ->set(borderColor, BASE_00);

        manager->style("tabs button")
               ->set(marginHorizontal, -1)
               ->set(paddingHorizontal, 18)
               ->set(cornerRadius, 0)
               ->set(borderHorizontal, 1)
               ->set(borderColor, BASE_00);
        // endregion

        // region MenuBar
        manager->style("menubar")
               ->set(direction, "row")
               ->set(backgroundColor, BASE_01)
               ->set(borderBottom, 1)
               ->set(borderColor, BASE_00);

        manager->style("menubar button")
               ->set(fontSize, 10)
               ->set(paddingHorizontal, 12)
               ->set(paddingVertical, 6)
               ->set(cornerRadius, 0)
               ->set(backgroundColor, 0x00000000);

        manager->style("menubar button:hover")
               ->set(color, 0xFF00FFEC);

        manager->style("menubar button:active")
               ->set(color, BASE_00)
               ->set(backgroundColor, 0xFF00FFEC);

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
               ->set(cornerRadius, 0)
               ->set(borderHorizontal, 1)
               ->set(borderColor, BASE_00);
        // endregion
    }
};