cmake_minimum_required(VERSION 3.4)
project(psychic-ui)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(ExternalProject)
include(FeatureSummary)

option(PSYCHIC_UI_BUILD_SHARED "Build Psychic UI as a shared library?" ON)
add_feature_info("psychic-ui-shared" PSYCHIC_UI_BUILD_SHARED "Build shared library")

find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(JPEGTURBO REQUIRED)
find_package(Freetype REQUIRED)
#find_package(ICU REQUIRED)

set(PSYCHIC_UI_EXTRA_LIBS "")
set(PSYCHIC_UI_EXTRA_DEFS "")
set(LIBPSYCHIC_UI_EXTRA_SOURCE "")

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

# PSYCHIC COLOR
if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/lib/psychic-color/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

add_subdirectory(lib/psychic-color)
# EXTRAS

# Required core libraries on various platforms
if (WIN32)
    list(APPEND PSYCHIC_UI_EXTRA_LIBS opengl32)
elseif (APPLE)
    find_library(cocoa_library Cocoa)
    find_library(opengl_library OpenGL)
    find_library(corevideo_library CoreVideo)
    find_library(iokit_library IOKit)
    list(APPEND PSYCHIC_UI_EXTRA_LIBS
#        ${cocoa_library}
#        ${opengl_library}
#        ${corevideo_library}
#        ${iokit_library}
        )
    list(APPEND LIBPSYCHIC_UI_EXTRA_SOURCE psychic-ui/darwin.mm)
elseif (CMAKE_SYSTEM MATCHES "Linux")
    find_package(Fontconfig REQUIRED)
    list(APPEND PSYCHIC_UI_EXTRA_LIBS
#        GL
#        Xxf86vm
#        Xrandr
#        Xinerama
#        Xcursor
#        Xi
#        X11
#        pthread
        fontconfig
        )
    if (CMAKE_SYSTEM MATCHES "Linux")
#        list(APPEND PSYCHIC_UI_EXTRA_LIBS dl)
    endif ()
endif ()

# LIBRARY

add_definitions(${PSYCHIC_UI_EXTRA_DEFS})

set(SOURCE_FILES ${LIBPSYCHIC_UI_EXTRA_SOURCE}
    #    ${NANOVG_SOURCES}
    ${YOGA_SOURCES}
    psychic-ui/psychic-ui.cpp
    psychic-ui/Window.cpp
    psychic-ui/Window.hpp
    psychic-ui/Div.cpp
    psychic-ui/Div.hpp
    psychic-ui/style/Style.cpp
    psychic-ui/style/Style.hpp
    psychic-ui/components/Button.cpp
    psychic-ui/components/Button.hpp
    psychic-ui/components/Label.cpp
    psychic-ui/components/Label.hpp
    psychic-ui/style/StyleManager.cpp
    psychic-ui/style/StyleManager.hpp
    psychic-ui/utils/StringUtils.hpp
    psychic-ui/style/StyleDeclaration.cpp
    psychic-ui/style/StyleDeclaration.hpp
    psychic-ui/style/StyleSelector.cpp
    psychic-ui/style/StyleSelector.hpp
    psychic-ui/style/StyleSheet.cpp
    psychic-ui/style/StyleSheet.hpp
    psychic-ui/utils/ColorUtils.hpp
    psychic-ui/utils/YogaUtils.hpp
    psychic-ui/components/Spacer.cpp
    psychic-ui/components/Spacer.hpp
    psychic-ui/components/DataContainer.cpp
    psychic-ui/components/DataContainer.hpp
    psychic-ui/components/Tabs.cpp
    psychic-ui/components/Tabs.hpp
    psychic-ui/components/TabContainer.cpp
    psychic-ui/components/TabContainer.hpp
    psychic-ui/components/Range.cpp
    psychic-ui/components/Range.hpp
    psychic-ui/Skin.cpp
    psychic-ui/Skin.hpp
    psychic-ui/skins/SliderRangeSkin.cpp
    psychic-ui/skins/SliderRangeSkin.hpp
    psychic-ui/Component.hpp
    psychic-ui/skins/DefaultButtonSkin.cpp
    psychic-ui/skins/DefaultButtonSkin.hpp
    psychic-ui/utils/Hatcher.hpp
    psychic-ui/components/TitleBar.cpp
    psychic-ui/components/TitleBar.hpp
    psychic-ui/skins/TitleBarButtonSkin.cpp
    psychic-ui/skins/TitleBarButtonSkin.hpp
    psychic-ui/components/MenuBar.cpp
    psychic-ui/components/MenuBar.hpp
    psychic-ui/components/Menu.cpp
    psychic-ui/components/Menu.hpp
    psychic-ui/signals/Signal.hpp
    psychic-ui/signals/Slot.hpp
    psychic-ui/signals/Observer.hpp
    psychic-ui/skins/DefaultMenuButtonSkin.cpp
    psychic-ui/skins/DefaultMenuButtonSkin.hpp
    psychic-ui/components/MenuButton.cpp
    psychic-ui/components/MenuButton.hpp
    psychic-ui/Shape.cpp
    psychic-ui/Shape.hpp
    psychic-ui/skins/DefaultCheckBoxSkin.cpp
    psychic-ui/skins/DefaultCheckBoxSkin.hpp
    psychic-ui/components/CheckBox.cpp
    psychic-ui/components/CheckBox.hpp
    psychic-ui/Modal.cpp
    psychic-ui/Modal.hpp
    psychic-ui/components/Scroller.cpp
    psychic-ui/components/Scroller.hpp
    psychic-ui/components/ScrollBar.cpp
    psychic-ui/components/ScrollBar.hpp
    psychic-ui/skins/DefaultScrollBarSkin.cpp
    psychic-ui/skins/DefaultScrollBarSkin.hpp)

add_library(psychic-ui SHARED ${SOURCE_FILES})
set_property(TARGET psychic-ui PROPERTY C_STANDARD 11)

target_include_directories(psychic-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${YOGA_INCLUDE_DIRS}
    ${SKIA_INCLUDE_DIRS}
    # ${RXCPP_INCLUDE_DIRS}
    ${PSYCHIC_COLOR_INCLUDE_DIRS}
)

target_link_libraries(psychic-ui
    # OPENGL
    ${OPENGL_LIBRARY}
    # GLFW
    glfw
#    ${GLFW_LIBRARIES}
    # Skia
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARY}
    ${TURBOJPEG_LIBRARY}
    ${FREETYPE_LIBRARIES}
    ${SKIA_LIBRARIES}
    # Yoga
    ${YOGA_LIBRARIES}
    # PSYCHIC COLOR
    ${PSYCHIC_COLOR_LIBRARIES}
)
#add_dependencies(psychic-ui eigen)
add_dependencies(psychic-ui psychic-color)
add_dependencies(psychic-ui glfw)
#add_dependencies(psychic-ui yoga)
#add_dependencies(psychic-ui nanovg)
add_dependencies(psychic-ui skia)

if (PSYCHIC_UI_BUILD_SHARED)
    set_property(TARGET psychic-ui PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET psychic-ui APPEND PROPERTY COMPILE_DEFINITIONS "_GLFW_BUILD_DLL")
    target_link_libraries(psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
endif ()

if (PSYCHIC_UI_USE_GLAD)
    add_dependencies(psychic-ui glad)
    if (PSYCHIC_UI_BUILD_SHARED)
        set_property(TARGET psychic-ui APPEND PROPERTY COMPILE_DEFINITIONS "GLAD_GLAPI_EXPORT;GLAD_GLAPI_EXPORT_BUILD")
    endif ()
endif ()

if (PSYCHIC_UI_BUILD_SHARED AND NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEB)
    # Platform-specific strip flags for reducing the library setSize.
    if (APPLE)
        # Strip .dylib library on OSX
        add_custom_command(TARGET psychic-ui POST_BUILD COMMAND strip -u -r "$<TARGET_FILE_DIR:nanogui>/$<TARGET_FILE_NAME:nanogui>")
    elseif (UNIX)
        # Strip .so library on Linux
        add_custom_command(TARGET psychic-ui POST_BUILD COMMAND strip "$<TARGET_FILE_DIR:nanogui>/$<TARGET_FILE_NAME:nanogui>")
    endif ()
endif ()

# RESOURCES
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# SUBDIRECTORIES
add_subdirectory(tests)
add_subdirectory(example)
add_subdirectory(playground)
