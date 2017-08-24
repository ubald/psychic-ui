cmake_minimum_required(VERSION 3.8)
project(psychicui)

set(CMAKE_CXX_STANDARD 17)

include(ExternalProject)


option(PSYCHICUI_BUILD_EXAMPLE "Build PsychicUI example application?" ON)
option(PSYCHICUI_BUILD_SHARED "Build PsychicUI as a shared library?" ON)

set(PSYCHICUI_EXTRA_LIBS "")
set(PSYCHICUI_EXTRA_DEFS "")
set(LIBPSYCHICUI_EXTRA_SOURCE "")

if (PSYCHICUI_BUILD_SHARED)
    list(APPEND PSYCHICUI_EXTRA_DEFS -DPSYCHICUI_SHARED -DNVG_SHARED -DGLAD_GLAPI_EXPORT)
endif ()


# GLAD

if (WIN32)
    set(PSYCHICUI_USE_GLAD_DEFAULT ON)
else ()
    set(PSYCHICUI_USE_GLAD_DEFAULT OFF)
endif ()

option(PSYCHICUI_USE_GLAD "Use Glad OpenGL loader library?" ${PSYCHICUI_USE_GLAD_DEFAULT})

if (PSYCHICUI_USE_GLAD)
    # Build and include GLAD
    ExternalProject_Add(glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad
        GIT_TAG master
        CMAKE_ARGS -DGLAD_API=gl=4.5
        CMAKE_ARGS -DGLAD_PROFILE=core
        CMAKE_ARGS -DGLAD_GENERATOR=c
        CMAKE_ARGS -DGLAD_EXPORT=OFF
        CMAKE_ARGS -DGLAD_INSTALL=OFF
        INSTALL_COMMAND ""
        )

    set(GLAD_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/glad-prefix/src/glad-build/include)
    set(GLAD_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/glad-prefix/src/glad-build/${CMAKE_STATIC_LIBRARY_PREFIX}glad${CMAKE_STATIC_LIBRARY_SUFFIX})

    include_directories(
        ${GLAD_INCLUDE_DIRS}
    )

    link_libraries(
        ${GLAD_LIBRARIES}
    )

    list(APPEND PSYCHICUI_EXTRA_DEFS -DPSYCHICUI_GLAD)
endif ()


# GLFW

ExternalProject_Add(glfw
    URL https://github.com/glfw/glfw/archive/3.2.1.tar.gz
    CMAKE_ARGS -DBUILD_SHARED_LIBS=FALSE
    CMAKE_ARGS -DGLFW_BUILD_EXAMPLES=FALSE
    CMAKE_ARGS -DGLFW_BUILD_TESTS=FALSE
    CMAKE_ARGS -DGLFW_BUILD_DOCS=FALSE
    CMAKE_ARGS -DGLFW_INSTALL=FALSE
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_COMMAND ""
    )

set(GLFW_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/glfw-prefix/src/glfw/include)
set(GLFW_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/glfw-prefix/src/glfw-build/src/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})

include_directories(
    ${GLFW_INCLUDE_DIRS}
)

link_libraries(
    ${GLFW_LIBRARIES}
)


# EIGEN

ExternalProject_Add(eigen
    URL http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )

set(EIGEN_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/eigen-prefix/src/eigen)

include_directories(
    ${EIGEN_INCLUDE_DIRS}
)


# NANOVG

ExternalProject_Add(nanovg
    GIT_REPOSITORY git@github.com:memononen/nanovg.git
    GIT_TAG master
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )

set(NANOVG_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/nanovg-prefix/src/nanovg/src)
set(NANOVG_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/nanovg-prefix/src/nanovg/src/nanovg.c)

include_directories(
    ${NANOVG_INCLUDE_DIRS}
)


# EXTRAS

# Required core libraries on various platforms
if (WIN32)
    list(APPEND PSYCHICUI_EXTRA_LIBS opengl32)
elseif (APPLE)
    find_library(cocoa_library Cocoa)
    find_library(opengl_library OpenGL)
    find_library(corevideo_library CoreVideo)
    find_library(iokit_library IOKit)
    list(APPEND PSYCHICUI_EXTRA_LIBS ${cocoa_library} ${opengl_library} ${corevideo_library} ${iokit_library})
    list(APPEND LIBPSYCHICUI_EXTRA_SOURCE psychicui/darwin.mm)
elseif (CMAKE_SYSTEM MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "BSD")
    list(APPEND PSYCHICUI_EXTRA_LIBS GL Xxf86vm Xrandr Xinerama Xcursor Xi X11 pthread)
    if (NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
        list(APPEND PSYCHICUI_EXTRA_LIBS rt)
    endif ()
    if (CMAKE_SYSTEM MATCHES "Linux")
        list(APPEND PSYCHICUI_EXTRA_LIBS dl)
    endif ()
endif ()

# LIBRARY

add_definitions(${PSYCHICUI_EXTRA_DEFS})

set(SOURCE_FILES ${LIBPSYCHICUI_EXTRA_SOURCE}
    ${NANOVG_SOURCES}
    psychicui/psychicui.cpp
    psychicui/Screen.cpp
    psychicui/Screen.hpp
    psychicui/Widget.cpp
    psychicui/Widget.hpp
    psychicui/Style.cpp
    psychicui/Style.hpp
    psychicui/Window.cpp
    psychicui/Window.hpp
    psychicui/Layout.cpp
    psychicui/Layout.hpp
    psychicui/Popup.cpp
    psychicui/Popup.hpp psychicui/Button.cpp psychicui/Button.hpp)
add_library(psychicui SHARED ${SOURCE_FILES})
target_include_directories(psychicui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(psychicui eigen)
add_dependencies(psychicui glfw)
add_dependencies(psychicui nanovg)

if (PSYCHICUI_BUILD_SHARED)
    set_property(TARGET psychicui PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET psychicui APPEND PROPERTY COMPILE_DEFINITIONS "_GLFW_BUILD_DLL;NVG_SHARED")
    target_link_libraries(psychicui ${PSYCHICUI_EXTRA_LIBS})
endif ()

set_property(TARGET psychicui APPEND PROPERTY COMPILE_DEFINITIONS "PSYCHICUI_BUILD;NVG_BUILD")

if (PSYCHICUI_USE_GLAD AND PSYCHICUI_BUILD_SHARED)
    set_property(TARGET psychicui APPEND PROPERTY COMPILE_DEFINITIONS "GLAD_GLAPI_EXPORT;GLAD_GLAPI_EXPORT_BUILD")
endif ()

if (PSYCHICUI_BUILD_SHARED AND NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEB)
    # Platform-specific strip flags for reducing the library size.
    if (APPLE)
        # Strip .dylib library on OSX
        add_custom_command(TARGET psychicui POST_BUILD COMMAND strip -u -r "$<TARGET_FILE_DIR:nanogui>/$<TARGET_FILE_NAME:nanogui>")
    elseif (UNIX)
        # Strip .so library on Linux
        add_custom_command(TARGET psychicui POST_BUILD COMMAND strip "$<TARGET_FILE_DIR:nanogui>/$<TARGET_FILE_NAME:nanogui>")
    endif ()
endif ()

# SAMPLE

if (PSYCHICUI_BUILD_EXAMPLE)
    add_executable(psychicui_demo example/demo.cpp example/demo.cpp)
    target_link_libraries(psychicui_demo psychicui ${PSYCHICUI_EXTRA_LIBS})
endif ()