#pragma once

#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/StyleSheet.hpp>
#include <psychic-ui/themes/palette.hpp>

using namespace psychic_ui;

/**
 * Demo Style
 * This style is loaded on top of the default Psychic UI stylesheet
 */
class DemoStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {

        manager->loadFont("stan0755", "fonts/stan0755.ttf");

        manager->style(".demo-panel")
               .padding(24);

        manager->style("Divs .demo-div")
            .margin(12)
            .width(128.0f)
            .height(64.0f)
            .backgroundColor(palette::base03)
            .borderColor(palette::base06);

        manager->style("Buttons Button")
               .margin(12);

        manager->style("TextInputs TextArea")
               .height(120);

        manager->style("Ranges Range")
               .margin(12);

        manager->style("Colors .swatch")
            .height(32.0f);

        manager->style("Colors .swatches")
            .direction(Direction::Row);

        manager->style("Colors .color-schemes .title")
            .marginTop(12);

        manager->style("Colors .swatches .swatch")
            .grow(1);

        manager->style("Colors .column Range")
            .shrink(0);
    }
};
