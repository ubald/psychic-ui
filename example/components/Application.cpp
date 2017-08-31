#include <psychicui/Label.hpp>
#include <psychicui/Button.hpp>
#include <psychicui/themes/default.hpp>
#include "Application.hpp"
#include "MenuBar.hpp"
#include "../demo-stylesheet.hpp"

namespace psychicui {

    Application::Application() :
            Window("Demo Application") {
            loadStyleSheet<PsychicUIStyleSheet>();
            loadStyleSheet<DemoStyleSheet>();

            addChild(std::make_shared<MenuBar>());

        _children[0]->children()[0]->computedStyle()->trace();

            addChild(std::make_shared<Label>("Label"));
            addChild(std::make_shared<Button>("Button"));
        }
}