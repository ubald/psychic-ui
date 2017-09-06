#pragma once

#include <psychicui/style/StyleManager.hpp>
#include <psychicui/style/StyleSheet.hpp>

using namespace psychicui;

/**
 * Demo Style
 * This style is loaded on top of the default PsychicUI stylesheet
 */
class DemoStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {

        manager->style(".demo-panel")
               ->set(margin, 24);

        manager->style("buttons button")
               ->set(margin, 12);

        manager->style("labels label")
               ->set(margin, 12);

        manager->style("ranges range")
               ->set(margin, 12);

    }
};