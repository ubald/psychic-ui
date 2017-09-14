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

        manager->style("buttons button")
               ->set(margin, 12);

        manager->style("labels label")
               ->set(margin, 12);

        manager->style("ranges range")
               ->set(margin, 12);

    }
};
