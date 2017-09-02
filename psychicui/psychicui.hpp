#pragma once

#include <SkColor.h>
#include <yoga/Yoga.h>
#include "opengl.hpp"
//#include <rxcpp/rx.hpp>

// Configuration
#define DEBUG_STYLES
//#define DEBUG_LAYOUT

// Define command key for windows/mac/linux
#ifdef __APPLE__
#define SYSTEM_COMMAND_MOD GLFW_MOD_SUPER
#else
#define SYSTEM_COMMAND_MOD GLFW_MOD_CONTROL
#endif

namespace psychicui {
    // RXCPP Namespaces are a mess
//    namespace rx {
//        using namespace rxcpp;
//        using namespace rxcpp::sources;
//        using namespace rxcpp::operators;
//        using namespace rxcpp::util;
//    }

    enum class Cursor {
        Arrow = 0,
        IBeam,
        Crosshair,
        Hand,
        HResize,
        VResize,
        /// Not a cursor --- should always be last: enables a loop over the cursor types.
        CursorCount
    };

    typedef SkColor Color;

    void init();
    void mainloop();
    void shutdown();

}