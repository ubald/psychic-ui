#pragma once

#include <iostream>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>

using namespace psychic_ui;

namespace psychic_ui_playground {

    class PlaygroundWindow : public Window {

    public:
        PlaygroundWindow();
    };

    inline PlaygroundWindow::PlaygroundWindow() :
        Window("Playground Application") {
        _decorated = false;

        skinSet<PivotalSkins>();
        //loadStyleSheet<PlaygroundStyleSheet>();

        Div::debugLayout = true;
    }
}
