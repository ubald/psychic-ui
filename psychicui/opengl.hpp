#pragma once

#if defined(PSYCHICUI_GLAD)
    #if defined(PSYCHICUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
        #define GLAD_GLAPI_EXPORT
    #endif
    #include <glad/glad.h>
#else
    #if defined(__APPLE__)
        #define GLFW_INCLUDE_GLCOREARB
    #else
        #define GL_GLEXT_PROTOTYPES
    #endif
#endif

#include <GLFW/glfw3.h>

// Special treatment of linux Nvidia opengl headers
#if !defined(_WIN32) && !defined(__APPLE__)
  #if !defined(GL_UNIFORM_BUFFER)
    #warning PSYCHICUI suspects you have the NVIDIA OpenGL headers installed.  \
             Compilation will likely fail. If it does, you have two choices: \
             (1) Re-install the mesa-libGL header files.                     \
             (2) Compile with PSYCHICUI_USE_GLAD.
  #endif
#endif
