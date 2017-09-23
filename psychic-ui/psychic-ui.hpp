#pragma once

#include <SkColor.h>
#include <yoga/Yoga.h>
//#include <rxcpp/rx.hpp>

// Configuration
#define DEBUG_STYLES
#define DEBUG_LAYOUT

// Define command key for windows/mac/linux
#ifdef __APPLE__
#define SYSTEM_COMMAND_MOD GLFW_MOD_SUPER
#else
#define SYSTEM_COMMAND_MOD GLFW_MOD_CONTROL
#endif

namespace psychic_ui {

    /**
     * Since we support either SDL2 or GLFW, we habe to map keyboard keys
     * to something we can use (they don't use the same values)
     */
    enum class PsyKey {

    };

    // TODO: Get rid of this and use psychic-color
    typedef SkColor Color;
}
