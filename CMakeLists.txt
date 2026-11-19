cmake_minimum_required(VERSION 3.24...4.4)
project(psychic-ui VERSION 0.0.0 LANGUAGES C CXX)

#
# CONFIGURATION
#

# Project Variables
set(PSYCHIC_UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PSYCHIC_UI_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(PSYCHIC_UI_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(PSYCHIC_UI_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(PSYCHIC_UI_VERSION_STRING ${PSYCHIC_UI_VERSION_MAJOR}.${PSYCHIC_UI_VERSION_MINOR}.${PSYCHIC_UI_VERSION_PATCH})
set(PSYCHIC_UI_API_VERSION ${PSYCHIC_UI_VERSION_MAJOR}.${PSYCHIC_UI_VERSION_MINOR})

#
# BUILDING
#

# Homebrew's keg-only ICU isn't on the default search
# path; ask brew where it lives rather than assuming /usr/local or /opt/homebrew.
if (APPLE)
    find_program(BREW brew)
    if (BREW)
        execute_process(COMMAND ${BREW} --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/icu4c" "${HOMEBREW_PREFIX}")
    endif ()
endif ()

# The API relies on C++23's explicit object parameters ("deducing this"). Older compilers accept
# -std=c++23 but fail deep in the headers, so stop here with the versions that support it.
if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18)
    OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
    OR (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.32))
    message(FATAL_ERROR "Psychic UI needs C++23 deducing this: GCC 14+, Clang 18+, Apple Clang 16+ or MSVC 17.2+ "
        "(found ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}). Pass another one with -DCMAKE_CXX_COMPILER=...")
endif ()

include(FeatureSummary)

if (WIN32)
    option(PSYCHIC_UI_SHARED "Build Psychic UI as a shared library" OFF)
    if (PSYCHIC_UI_SHARED)
        message(FATAL_ERROR "Psychic UI can't be built as a shared library on Windows yet: it exports no symbols. Configure with -DPSYCHIC_UI_SHARED=OFF.")
    endif ()
else ()
    option(PSYCHIC_UI_SHARED "Build Psychic UI as a shared library" ${PROJECT_IS_TOP_LEVEL})
endif ()
add_feature_info("psychic-ui-shared" PSYCHIC_UI_SHARED "Build shared library")

# The static dependencies below end up inside the shared library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(PSYCHIC_UI_BUILD_SDL3 "SDL3 backend: the application runner, and the adapter for SDL hosts" ON)
add_feature_info("psychic-ui-sdl3" PSYCHIC_UI_BUILD_SDL3 "Build the SDL3 backend")

option(PSYCHIC_UI_BUILD_GLFW_ADAPTER "GLFW adapter, to embed Psychic UI in a GLFW application" ON)
add_feature_info("psychic-ui-glfw-adapter" PSYCHIC_UI_BUILD_GLFW_ADAPTER "Build the GLFW embedding adapter")

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc data)
if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD")
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
endif ()

include(FetchContent)

# SDL3: an installed one if there is one, otherwise this release, built static with only what windows,
# events and OpenGL need
if (PSYCHIC_UI_BUILD_SDL3)
    FetchContent_Declare(SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-3.4.20.tar.gz
        URL_HASH SHA256=f3bb06f55bed9557227a700af2ea62e49497ebdf58037732a731a5fb3ef8581f
        FIND_PACKAGE_ARGS 3.2 CONFIG
        )
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
    set(SDL_TEST_LIBRARY OFF)
    set(SDL_TESTS OFF)
    set(SDL_EXAMPLES OFF)
    set(SDL_INSTALL OFF)
    set(SDL_AUDIO OFF)
    set(SDL_CAMERA OFF)
    set(SDL_GPU OFF)
    set(SDL_RENDER OFF)
    set(SDL_JOYSTICK OFF)
    set(SDL_HAPTIC OFF)
    set(SDL_HIDAPI OFF)
    set(SDL_SENSOR OFF)
    set(SDL_POWER OFF)
    set(SDL_VULKAN OFF)
    set(SDL_KMSDRM OFF)
    # X11 extensions for features Psychic UI doesn't use (screensaver inhibition, shaped windows,
    # synthetic input, message box double buffering): SDL stops if their headers are missing
    set(SDL_X11_XSCRNSAVER OFF)
    set(SDL_X11_XSHAPE OFF)
    set(SDL_X11_XTEST OFF)
    set(SDL_X11_XDBE OFF)
    FetchContent_MakeAvailable(SDL3)
endif ()

# GLFW, for the embedding adapter: an installed 3.4+ if there is one, otherwise this release, built static
if (PSYCHIC_UI_BUILD_GLFW_ADAPTER)
    FetchContent_Declare(glfw3
        URL https://github.com/glfw/glfw/releases/download/3.5.1/glfw-3.5.1.zip
        URL_HASH SHA256=ea79bc5feffc254c87291980c2d0bce9acebb68c4983b79f961dcd2cb8a611a0
        FIND_PACKAGE_ARGS 3.4 CONFIG
        )
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_INSTALL OFF)
    # Wayland only when its development files are there (GLFW stops otherwise), and not next to a
    # fetched, static SDL3: both compile the same Wayland protocol code, whose symbols then clash
    # when linked together. X11 always.
    if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD" AND NOT DEFINED GLFW_BUILD_WAYLAND)
        if (DEFINED sdl3_SOURCE_DIR)
            set(GLFW_BUILD_WAYLAND OFF)
        endif ()
    endif ()
    if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD" AND NOT DEFINED GLFW_BUILD_WAYLAND)
        find_package(PkgConfig QUIET)
        if (PKG_CONFIG_FOUND)
            pkg_check_modules(GLFW_WAYLAND_DEPS QUIET wayland-client wayland-cursor wayland-egl xkbcommon)
        endif ()
        find_program(WAYLAND_SCANNER wayland-scanner)
        if (GLFW_WAYLAND_DEPS_FOUND AND WAYLAND_SCANNER)
            set(GLFW_BUILD_WAYLAND ON)
        else ()
            set(GLFW_BUILD_WAYLAND OFF)
        endif ()
    endif ()
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_MakeAvailable(glfw3)
    unset(BUILD_SHARED_LIBS)
endif ()

# YOGA
add_subdirectory(extlib/yoga)

# SKIA
add_subdirectory(extlib/skia)

# PSYCHIC COLOR
if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/lib/psychic-color/.git")
    message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif ()

# Static, without overriding BUILD_SHARED_LIBS for the rest of the project
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(lib/psychic-color)
unset(BUILD_SHARED_LIBS)

# LIBRARY

set(SOURCE_FILES
    ${LIBPSYCHIC_UI_EXTRA_SOURCE}
    psychic-ui/animation/Animation.hpp
    psychic-ui/animation/Interpolation.cpp
    psychic-ui/animation/Interpolation.hpp
    psychic-ui/animation/StyleTransitions.cpp
    psychic-ui/animation/StyleTransitions.hpp
    psychic-ui/animation/Timing.cpp
    psychic-ui/animation/Timing.hpp
    psychic-ui/animation/Tween.cpp
    psychic-ui/animation/Tween.hpp
    psychic-ui/components/Button.cpp
    psychic-ui/components/Button.hpp
    psychic-ui/components/CheckBox.cpp
    psychic-ui/components/CheckBox.hpp
    psychic-ui/components/DataContainer.cpp
    psychic-ui/components/DataContainer.hpp
    psychic-ui/components/Icon.cpp
    psychic-ui/components/Icon.hpp
    psychic-ui/components/Label.cpp
    psychic-ui/components/Label.hpp
    psychic-ui/components/Menu.cpp
    psychic-ui/components/Menu.hpp
    psychic-ui/components/MenuBar.cpp
    psychic-ui/components/MenuBar.hpp
    psychic-ui/components/MenuButton.cpp
    psychic-ui/components/MenuButton.hpp
    psychic-ui/components/Range.cpp
    psychic-ui/components/Range.hpp
    psychic-ui/components/ScrollBar.cpp
    psychic-ui/components/ScrollBar.hpp
    psychic-ui/components/Scroller.cpp
    psychic-ui/components/Scroller.hpp
    psychic-ui/components/Spacer.cpp
    psychic-ui/components/Spacer.hpp
    psychic-ui/components/TabContainer.cpp
    psychic-ui/components/TabContainer.hpp
    psychic-ui/components/Tabs.cpp
    psychic-ui/components/Tabs.hpp
    psychic-ui/components/TitleBar.cpp
    psychic-ui/components/TitleBar.hpp
    psychic-ui/components/AppShell.cpp
    psychic-ui/components/AppShell.hpp
    psychic-ui/components/AppShellPage.cpp
    psychic-ui/components/AppShellPage.hpp
    psychic-ui/components/PageHeader.cpp
    psychic-ui/components/PageHeader.hpp
    psychic-ui/components/Sidebar.cpp
    psychic-ui/components/Sidebar.hpp
    psychic-ui/components/Avatar.cpp
    psychic-ui/components/Avatar.hpp
    psychic-ui/components/Badge.cpp
    psychic-ui/components/Badge.hpp
    psychic-ui/components/Chip.cpp
    psychic-ui/components/Chip.hpp
    psychic-ui/components/StatusDot.cpp
    psychic-ui/components/StatusDot.hpp
    psychic-ui/components/DataGrid.cpp
    psychic-ui/components/DataGrid.hpp
    psychic-ui/components/EmptyState.cpp
    psychic-ui/components/EmptyState.hpp
    psychic-ui/components/Inspector.cpp
    psychic-ui/components/Inspector.hpp
    psychic-ui/components/Kbd.cpp
    psychic-ui/components/Kbd.hpp
    psychic-ui/components/List.cpp
    psychic-ui/components/List.hpp
    psychic-ui/components/Rows.cpp
    psychic-ui/components/Rows.hpp
    psychic-ui/components/Tree.cpp
    psychic-ui/components/Tree.hpp
    psychic-ui/components/Tone.cpp
    psychic-ui/components/Tone.hpp
    psychic-ui/components/WindowControls.cpp
    psychic-ui/components/WindowControls.hpp
    psychic-ui/components/WindowControlsStyle.hpp
    psychic-ui/components/ToolBar.cpp
    psychic-ui/components/ToolBar.hpp
    psychic-ui/components/ColorSwatches.cpp
    psychic-ui/components/ColorSwatches.hpp
    psychic-ui/components/NumberStepper.cpp
    psychic-ui/components/NumberStepper.hpp
    psychic-ui/components/SearchField.cpp
    psychic-ui/components/SearchField.hpp
    psychic-ui/components/SegmentedControl.cpp
    psychic-ui/components/SegmentedControl.hpp
    psychic-ui/components/Splitter.cpp
    psychic-ui/components/Splitter.hpp
    psychic-ui/components/SplitterDivider.cpp
    psychic-ui/components/SplitterDivider.hpp
    psychic-ui/components/Switch.cpp
    psychic-ui/components/Switch.hpp
    psychic-ui/components/Dialog.cpp
    psychic-ui/components/Dialog.hpp
    psychic-ui/components/Select.cpp
    psychic-ui/components/Select.hpp
    psychic-ui/components/Toast.cpp
    psychic-ui/components/Toast.hpp
    psychic-ui/components/Tooltip.cpp
    psychic-ui/components/Tooltip.hpp
    psychic-ui/icons/IconRegistry.cpp
    psychic-ui/icons/IconRegistry.hpp
    psychic-ui/icons/IconSet.hpp
    psychic-ui/icons/LucideIcons.cpp
    psychic-ui/icons/PivotalIcons.cpp
    psychic-ui/paint/Box.cpp
    psychic-ui/paint/Box.hpp
    psychic-ui/paint/Crisp.cpp
    psychic-ui/paint/Crisp.hpp
    psychic-ui/paint/Icon.cpp
    psychic-ui/paint/Icon.hpp
    psychic-ui/paint/Marks.cpp
    psychic-ui/paint/Marks.hpp
    psychic-ui/signals/Observer.hpp
    psychic-ui/signals/Signal.hpp
    psychic-ui/signals/Slot.hpp
    psychic-ui/skins/SkinSet.hpp
    psychic-ui/skins/base/AvatarSkinBase.cpp
    psychic-ui/skins/base/AvatarSkinBase.hpp
    psychic-ui/skins/base/ButtonSkinBase.cpp
    psychic-ui/skins/base/ButtonSkinBase.hpp
    psychic-ui/skins/base/CheckBoxSkinBase.cpp
    psychic-ui/skins/base/CheckBoxSkinBase.hpp
    psychic-ui/skins/base/MenuButtonSkinBase.cpp
    psychic-ui/skins/base/MenuButtonSkinBase.hpp
    psychic-ui/skins/base/RangeSkinBase.cpp
    psychic-ui/skins/base/RangeSkinBase.hpp
    psychic-ui/skins/base/ScrollBarSkinBase.cpp
    psychic-ui/skins/base/ScrollBarSkinBase.hpp
    psychic-ui/skins/base/SkinState.hpp
    psychic-ui/skins/base/TextAreaSkinBase.cpp
    psychic-ui/skins/base/TextAreaSkinBase.hpp
    psychic-ui/skins/base/TextInputSkinBase.cpp
    psychic-ui/skins/base/TextInputSkinBase.hpp
    psychic-ui/skins/base/ColorSwatchesSkinBase.cpp
    psychic-ui/skins/base/ColorSwatchesSkinBase.hpp
    psychic-ui/skins/base/NumberStepperSkinBase.cpp
    psychic-ui/skins/base/NumberStepperSkinBase.hpp
    psychic-ui/skins/base/SearchFieldSkinBase.cpp
    psychic-ui/skins/base/SearchFieldSkinBase.hpp
    psychic-ui/skins/base/SegmentedControlSkinBase.cpp
    psychic-ui/skins/base/SegmentedControlSkinBase.hpp
    psychic-ui/skins/base/SplitterDividerSkinBase.cpp
    psychic-ui/skins/base/SplitterDividerSkinBase.hpp
    psychic-ui/skins/base/SwitchSkinBase.cpp
    psychic-ui/skins/base/SwitchSkinBase.hpp
    psychic-ui/skins/desk/DeskSkins.cpp
    psychic-ui/skins/desk/DeskSkins.hpp
    psychic-ui/skins/float/FloatSkins.cpp
    psychic-ui/skins/float/FloatSkins.hpp
    psychic-ui/skins/pivotal/PivotalAvatarSkin.cpp
    psychic-ui/skins/pivotal/PivotalAvatarSkin.hpp
    psychic-ui/skins/pivotal/PivotalSkins.cpp
    psychic-ui/skins/pivotal/PivotalSkins.hpp
    psychic-ui/skins/pivotal/TabButtonSkin.cpp
    psychic-ui/skins/pivotal/TabButtonSkin.hpp
    psychic-ui/skins/pivotal/CaptionButtonSkin.cpp
    psychic-ui/skins/pivotal/CaptionButtonSkin.hpp
    psychic-ui/skins/pivotal/TitleBarButtonSkin.cpp
    psychic-ui/skins/pivotal/TitleBarButtonSkin.hpp
    psychic-ui/style/Atom.cpp
    psychic-ui/style/Atom.hpp
    psychic-ui/style/Color.cpp
    psychic-ui/style/Color.hpp
    psychic-ui/style/Length.cpp
    psychic-ui/style/Length.hpp
    psychic-ui/style/Properties.hpp
    psychic-ui/style/Style.cpp
    psychic-ui/style/Style.hpp
    psychic-ui/style/StyleDeclaration.cpp
    psychic-ui/style/StyleDeclaration.hpp
    psychic-ui/style/StyleManager.cpp
    psychic-ui/style/StyleManager.hpp
    psychic-ui/style/StyleSelector.cpp
    psychic-ui/style/StyleSelector.hpp
    psychic-ui/style/StyleSheet.cpp
    psychic-ui/style/StyleSheet.hpp
    psychic-ui/style/Values.hpp
    psychic-ui/themes/desk.hpp
    psychic-ui/themes/float.hpp
    psychic-ui/themes/one-dark.hpp
    psychic-ui/themes/one-light.hpp
    psychic-ui/themes/palette.hpp
    psychic-ui/themes/pivotal.hpp
    psychic-ui/themes/psychic.hpp
    psychic-ui/themes/Theme.hpp
    psychic-ui/themes/ubuntu-type.hpp
    psychic-ui/utils/ColorUtils.hpp
    psychic-ui/utils/Hatcher.hpp
    psychic-ui/utils/NoDestructor.hpp
    psychic-ui/utils/StringUtils.hpp
    psychic-ui/Component.hpp
    psychic-ui/Div.cpp
    psychic-ui/Div.hpp
    psychic-ui/Measure.hpp
    psychic-ui/Overlay.cpp
    psychic-ui/Overlay.hpp
    psychic-ui/Popup.cpp
    psychic-ui/Popup.hpp
    psychic-ui/opengl.hpp
    psychic-ui/psychic-ui.hpp
    psychic-ui/Shape.cpp
    psychic-ui/Shape.hpp
    psychic-ui/Skin.cpp
    psychic-ui/Skin.hpp
    psychic-ui/Window.cpp
    psychic-ui/Window.hpp
    psychic-ui/components/Box.cpp
    psychic-ui/components/Box.hpp
    psychic-ui/SystemWindow.cpp
    psychic-ui/SystemWindow.hpp
    psychic-ui/WindowScale.cpp
    psychic-ui/WindowScale.hpp
    psychic-ui/components/TextField.cpp
    psychic-ui/components/TextField.hpp
    psychic-ui/components/TextInput.cpp
    psychic-ui/components/TextInput.hpp
    psychic-ui/components/TextArea.cpp
    psychic-ui/components/TextArea.hpp
    psychic-ui/utils/ExecutablePath.cpp
    psychic-ui/utils/ExecutablePath.hpp
    psychic-ui/text/TextEditing.cpp
    psychic-ui/text/TextEditing.hpp
    psychic-ui/text/TextGeometry.hpp
    psychic-ui/text/TextLayout.cpp
    psychic-ui/text/TextLayout.hpp
    psychic-ui/components/Text.cpp
    psychic-ui/components/Text.hpp
    )

if (PSYCHIC_UI_BUILD_SDL3)
    list(APPEND SOURCE_FILES
        psychic-ui/applications/SDL3Application.cpp
        psychic-ui/applications/SDL3Application.hpp
        psychic-ui/applications/SDL3SystemWindow.cpp
        psychic-ui/applications/SDL3SystemWindow.hpp
        )
endif ()

if (PSYCHIC_UI_BUILD_GLFW_ADAPTER)
    list(APPEND SOURCE_FILES
        psychic-ui/applications/GLFWSystemWindow.cpp
        psychic-ui/applications/GLFWSystemWindow.hpp
        )
endif ()

if (PSYCHIC_UI_SHARED)
    set(_psychic_ui_type SHARED)
else ()
    set(_psychic_ui_type STATIC)
endif ()
add_library(psychic-ui ${_psychic_ui_type} ${SOURCE_FILES})
add_library(psychic-ui::psychic-ui ALIAS psychic-ui)

target_compile_features(psychic-ui PUBLIC cxx_std_23)
if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(psychic-ui PRIVATE /W4)
else ()
    target_compile_options(psychic-ui PRIVATE -Wall -Wextra)
endif ()

target_include_directories(psychic-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(psychic-ui PUBLIC
    OpenGL::GL
    Skia::Skia
    ICU::uc
    ICU::data
    psychic-color::psychic-color
    )

# Yoga is an implementation detail: no public header includes it
target_link_libraries(psychic-ui PRIVATE yogacore)

if (WIN32)
    target_compile_definitions(psychic-ui PUBLIC WIN)
elseif (APPLE)
    target_compile_definitions(psychic-ui PUBLIC APPLE)
elseif (UNIX)
    target_compile_definitions(psychic-ui PUBLIC UNIX)
endif ()

if (PSYCHIC_UI_BUILD_SDL3)
    target_link_libraries(psychic-ui PUBLIC SDL3::SDL3)
endif ()

if (PSYCHIC_UI_BUILD_GLFW_ADAPTER)
    target_link_libraries(psychic-ui PUBLIC glfw)
endif ()

if (PSYCHIC_UI_SHARED AND NOT CMAKE_BUILD_TYPE MATCHES "Deb")
    if (APPLE)
        add_custom_command(TARGET psychic-ui POST_BUILD COMMAND strip -u -r "$<TARGET_FILE:psychic-ui>")
    elseif (UNIX)
        add_custom_command(TARGET psychic-ui POST_BUILD COMMAND strip "$<TARGET_FILE:psychic-ui>")
    endif ()
endif ()

# RESOURCES
# Copies res/ (the bundled fonts) next to an executable after each build of it, where relative font paths resolve
# (StyleManager::resourceRoot, the executable's directory by default)
function(psychic_ui_copy_resources target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/res" "$<TARGET_FILE_DIR:${target}>/res"
        VERBATIM)
endfunction()

# SUBDIRECTORIES
option(PSYCHIC_UI_BUILD_TESTS "Build Psychic UI tests?" ${PROJECT_IS_TOP_LEVEL})
if (PSYCHIC_UI_BUILD_TESTS)
    enable_testing()
endif ()
add_subdirectory(tests)
add_subdirectory(example)
add_subdirectory(playground)
add_subdirectory(showcase)

#
# OTHER TARGETS
#

# Documentation
find_package(Doxygen)
if (DOXYGEN_FOUND AND PROJECT_IS_TOP_LEVEL)
    set(PACKAGE_NAME psychic-ui) # Doxyfile.in
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif ()

#
# POST
#

# Report
if (PROJECT_IS_TOP_LEVEL)
    feature_summary(WHAT ALL)
endif ()
