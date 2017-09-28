#pragma once

#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/StyleSheet.hpp>

using namespace psychic_ui;

/**
 * Demo Style
 * This style is loaded on top of the default Psychic UI stylesheet
 */
class DemoStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {

        manager->style(".demo-panel")
               ->set(padding, 24);

        manager
            ->style("Divs .demo-div")
            ->set(margin, 12)
            ->set(width, 128.0f)
            ->set(height, 64.0f)
            ->set(backgroundColor, BASE_03)
            ->set(borderColor, BASE_06);

        manager->style("Buttons Button")
               ->set(margin, 12);

        manager->style("Ranges Range")
               ->set(margin, 12);

        manager
            ->style("Colors .swatch")
            ->set(height, 32.0f);

        manager
            ->style("Colors .swatches")
            ->set(flexDirection, "row");

        manager
            ->style("Colors .color-schemes .title")
            ->set(marginTop, 12);

        manager
            ->style("Colors .swatches .swatch")
            ->set(grow, 1);

        manager
            ->style("Colors .column Range")
            ->set(shrink, 0);
    }
};
