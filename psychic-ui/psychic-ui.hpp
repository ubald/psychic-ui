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
    // RXCPP Namespaces are a mess
//    namespace rx {
//        using namespace rxcpp;
//        using namespace rxcpp::sources;
//        using namespace rxcpp::operators;
//        using namespace rxcpp::util;
//    }

    typedef SkColor Color;
}
