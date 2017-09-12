cmake_minimum_required(VERSION 3.8)
project(psychicui)

set(CMAKE_CXX_STANDARD 17)

include(ExternalProject)
include(FeatureSummary)

option(PSYCHICUI_BUILD_SHARED "Build PsychicUI as a shared library?" ON)

set(PSYCHICUI_EXTRA_LIBS "")
set(PSYCHICUI_EXTRA_DEFS "")
set(LIBPSYCHICUI_EXTRA_SOURCE "")

# GLAD
add_subdirectory(extlib/glad)

# GLFW
add_subdirectory(extlib/glfw)

# YOGA
add_subdirectory(extlib/yoga)

# SKIA
add_subdirectory(extlib/skia)

# RXCPP
# add_subdirectory(extlib/rxcpp)

# EXTRAS

# Required core libraries on various platforms
if (WIN32)
    list(APPEND PSYCHICUI_EXTRA_LIBS opengl32)
elseif (APPLE)
    find_library(cocoa_library Cocoa)
    find_library(opengl_library OpenGL)
    find_library(corevideo_library CoreVideo)
    find_library(iokit_library IOKit)
    #    find_library(coreservices_library CoreServices)
    #    find_library(coregraphics_library CoreGraphics)
    #    find_library(coretext_library CoreText)
    #    find_library(corefoundation_library CoreFoundation)
    list(APPEND PSYCHICUI_EXTRA_LIBS
        ${cocoa_library}
        ${opengl_library}
        ${corevideo_library}
        ${iokit_library}
        png
        -L/usr/local/opt/jpeg-turbo/lib
        jpeg
        turbojpeg
        freetype
        #        ${coreservices_library}
        #        ${coregraphics_library}
        #        ${coretext_library}
        #        ${corefoundation_library}
        )
    list(APPEND LIBPSYCHICUI_EXTRA_SOURCE psychicui/darwin.mm)
elseif (CMAKE_SYSTEM MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "BSD")
    list(APPEND PSYCHICUI_EXTRA_LIBS
        GL
        Xxf86vm
        Xrandr
        Xinerama
        Xcursor
        Xi
        X11
        pthread
        png
        jpeg
        turbojpeg
        freetype
        fontconfig
        )
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
    #    ${NANOVG_SOURCES}
    ${YOGA_SOURCES}
    psychicui/psychicui.cpp
    psychicui/Window.cpp
    psychicui/Window.hpp
    psychicui/Div.cpp
    psychicui/Div.hpp
    psychicui/style/Style.cpp
    psychicui/style/Style.hpp
    psychicui/components/Button.cpp
    psychicui/components/Button.hpp
    psychicui/components/Label.cpp
    psychicui/components/Label.hpp
    psychicui/Registry.cpp
    psychicui/Registry.hpp
    psychicui/style/StyleManager.cpp
    psychicui/style/StyleManager.hpp
    psychicui/utils/StringUtils.hpp
    psychicui/style/StyleDeclaration.cpp
    psychicui/style/StyleDeclaration.hpp
    psychicui/style/StyleSelector.cpp
    psychicui/style/StyleSelector.hpp
    psychicui/style/StyleSheet.cpp
    psychicui/style/StyleSheet.hpp
    psychicui/utils/ColorUtils.hpp
    psychicui/utils/YogaUtils.hpp
    psychicui/components/Spacer.cpp
    psychicui/components/Spacer.hpp
    psychicui/components/DataContainer.cpp
    psychicui/components/DataContainer.hpp
    psychicui/components/Tabs.cpp
    psychicui/components/Tabs.hpp
    psychicui/components/TabContainer.cpp
    psychicui/components/TabContainer.hpp
    psychicui/components/Range.cpp
    psychicui/components/Range.hpp
    psychicui/Skin.cpp
    psychicui/Skin.hpp
    psychicui/skins/SliderRangeSkin.cpp
    psychicui/skins/SliderRangeSkin.hpp
    psychicui/Component.hpp
    psychicui/skins/DefaultButtonSkin.cpp
    psychicui/skins/DefaultButtonSkin.hpp
    psychicui/utils/Hatcher.hpp
    psychicui/components/TitleBar.cpp
    psychicui/components/TitleBar.hpp
    psychicui/skins/TitleBarButtonSkin.cpp
    psychicui/skins/TitleBarButtonSkin.hpp
    psychicui/components/MenuBar.cpp
    psychicui/components/MenuBar.hpp
    psychicui/components/Menu.cpp
    psychicui/components/Menu.hpp
    psychicui/signals/Signal.hpp
    psychicui/signals/Slot.hpp
    psychicui/signals/Observer.hpp
    psychicui/skins/DefaultMenuButtonSkin.cpp psychicui/skins/DefaultMenuButtonSkin.hpp psychicui/components/MenuButton.cpp psychicui/components/MenuButton.hpp psychicui/Shape.cpp psychicui/Shape.hpp psychicui/skins/DefaultCheckBoxSkin.cpp psychicui/skins/DefaultCheckBoxSkin.hpp psychicui/components/CheckBox.cpp psychicui/components/CheckBox.hpp psychicui/Modal.cpp psychicui/Modal.hpp psychicui/Scroller.cpp psychicui/Scroller.hpp psychicui/components/ScrollBar.cpp psychicui/components/ScrollBar.hpp psychicui/skins/DefaultScrollBarSkin.cpp psychicui/skins/DefaultScrollBarSkin.hpp)
add_library(psychicui SHARED ${SOURCE_FILES})
target_include_directories(psychicui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${YOGA_INCLUDE_DIRS}
    ${SKIA_INCLUDE_DIRS}
    # ${RXCPP_INCLUDE_DIRS}
)
target_link_libraries(psychicui
    ${GLFW_LIBRARIES}
    ${YOGA_LIBRARIES}
    ${SKIA_LIBRARIES}
)
#add_dependencies(psychicui eigen)
add_dependencies(psychicui glfw)
#add_dependencies(psychicui yoga)
#add_dependencies(psychicui nanovg)
add_dependencies(psychicui skia)

if (PSYCHICUI_BUILD_SHARED)
    set_property(TARGET psychicui PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET psychicui APPEND PROPERTY COMPILE_DEFINITIONS "_GLFW_BUILD_DLL")
    target_link_libraries(psychicui ${PSYCHICUI_EXTRA_LIBS})
endif ()

if (PSYCHICUI_USE_GLAD)
    add_dependencies(psychicui glad)
    if (PSYCHICUI_BUILD_SHARED)
        set_property(TARGET psychicui APPEND PROPERTY COMPILE_DEFINITIONS "GLAD_GLAPI_EXPORT;GLAD_GLAPI_EXPORT_BUILD")
    endif ()
endif ()

if (PSYCHICUI_BUILD_SHARED AND NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEB)
    # Platform-specific strip flags for reducing the library setSize.
    if (APPLE)
        # Strip .dylib library on OSX
        add_custom_command(TARGET psychicui POST_BUILD COMMAND strip -u -r "$<TARGET_FILE_DIR:nanogui>/$<TARGET_FILE_NAME:nanogui>")
    elseif (UNIX)
        # Strip .so library on Linux
        add_custom_command(TARGET psychicui POST_BUILD COMMAND strip "$<TARGET_FILE_DIR:nanogui>/$<TARGET_FILE_NAME:nanogui>")
    endif ()
endif ()

# RESOURCES
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# SUBDIRECTORIES
add_subdirectory(tests)
add_subdirectory(example)
add_subdirectory(playground)
