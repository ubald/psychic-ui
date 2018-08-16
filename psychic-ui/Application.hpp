#pragma once

#ifdef USE_GLFW
#include "applications/GLFWApplication.hpp"
using Impl = psychic_ui::GLFWApplication;
#endif

#ifdef USE_SDL2
#include "applications/SDL2Application.hpp"
using Impl = psychic_ui::SDL2Application;
#endif

namespace psychic_ui {
    using Application = Impl;
}
