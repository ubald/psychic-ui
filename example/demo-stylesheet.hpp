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

class DemoStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {
        manager->loadFont("stan0755", "res/fonts/stan0755.ttf");
        manager->loadFont("Ubuntu Light", "res/fonts/Ubuntu/Ubuntu-Light.ttf");

        auto global = manager->getStyle("*");
        global->setValue(fontFamily, "Ubuntu Light");
        global->setValue(fontSize, 12);
        global->setValue(lineHeight, 16);
        global->setValue(color, 0xFFFFFFFF);
        global->setValue(textAntiAlias, true);
        global->setValue(border, 0);
        global->setValue(minWidth, -1);
        global->setValue(minHeight, -1);
        global->setValue(maxWidth, -1);
        global->setValue(maxHeight, -1);
        global->setValue(position, "relative");


        auto window = manager->getStyle("window");
        window->setValue(color, 0xFFE0E0E0);
        window->setValue(backgroundColor, BASE_00);

        auto button = manager->getStyle("button");
        button->setValue(cornerRadius, 6);
        button->setValue(color, BASE_07);
        button->setValue(backgroundColor, BASE_01);

        auto buttonHover = manager->getStyle("button:hover");
        buttonHover->setValue(color, BASE_00);
        buttonHover->setValue(backgroundColor, 0xFF00FFEC);
    }
};