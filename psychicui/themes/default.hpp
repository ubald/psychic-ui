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

        // region MenuBar
        manager->style("menubar")
               ->set(direction, "row")
               ->set(backgroundColor, BASE_01);

        manager->style("menubar button")
               ->set(paddingHorizontal, 12)
               ->set(cornerRadius, 0)
               ->set(borderRight, 1)
               ->set(borderColor, BASE_00);
        // endregion

        // region Buttons
        manager->style("button")
               ->set(direction, "row")
               ->set(alignItems, "center")
               ->set(cornerRadius, 6)
               ->set(backgroundColor, BASE_02)
               ->set(color, BASE_07)
               ->set(fontSize, 12)
               ->set(lineHeight, 12);

        manager->style("button:hover")
               ->set(color, BASE_00)
               ->set(backgroundColor, 0xFF00FFEC);
        // endregion
    }
};