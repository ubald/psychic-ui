#pragma once

#include <iostream>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/skins/default/DefaultSkins.hpp>

using namespace psychic_ui;

namespace psychic_ui_playground {

    class PlaygroundWindow : public Window {

    public:
        PlaygroundWindow();
    };

    PlaygroundWindow::PlaygroundWindow() :
        Window("Playground Application") {
        _decorated = false;

        skinSet<DefaultSkins>();
        //loadStyleSheet<PlaygroundStyleSheet>();

        Div::debugLayout = true;
    }
}
