cmake_minimum_required(VERSION 3.4)
project(psychic-ui)

#
# CONFIGURATION
#

# Project Variables
set(PSYCHIC_UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PSYCHIC_UI_VERSION_MAJOR 0)
set(PSYCHIC_UI_VERSION_MINOR 0)
set(PSYCHIC_UI_VERSION_PATCH 0)
set(PSYCHIC_UI_VERSION_STRING ${PSYCHIC_UI_VERSION_MAJOR}.${PSYCHIC_UI_VERSION_MINOR}.${PSYCHIC_UI_VERSION_PATCH})
set(PSYCHIC_UI_API_VERSION ${PSYCHIC_UI_VERSION_MAJOR}.${PSYCHIC_UI_VERSION_MINOR})

# Package (pkg-config) Information
set(PACKAGE_NAME psychic-ui)
set(PACKAGE_DESCRIPTION "Psychic UI C++/OpenGL UI Library")
set(PACKAGE_URL "https://github.com/ubald/psychic-ui")
set(PACKAGE_VERSION ${PSYCHIC_UI_VERSION_STRING})

# CPack - General
set(CPACK_PACKAGE_NAME ${PACKAGE_NAME})
set(CPACK_PACKAGE_VENDOR "Ubald Design")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE_DESCRIPTION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PSYCHIC_UI_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PSYCHIC_UI_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PSYCHIC_UI_VERSION_PATCH}")
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_COMPONENTS_ALL applications libraries headers)

# CPack - Applications
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Psychic UI")
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "C++/OpenGL UI Library")
set(CPACK_COMPONENT_APPLICATIONS_DEPENDS libraries)

# CPack - Libraries
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "Libraries used to build programs with Psychic UI")

# CPack - Includes
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "C/C++ header used to build programs with Psychic UI")
set(CPACK_COMPONENT_HEADERS_DEPENDS libraries)

# CPack - Source
set(CPACK_SOURCE_IGNORE_FILES "/\.idea/;/build/;/cmake-build-debug/;${CPACK_SOURCE_IGNORE_FILES}")

#
# BUILDING
#

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (APPLE)
    list(APPEND CMAKE_PREFIX_PATH /usr/local/opt)
    list(APPEND CMAKE_LIBRARY_PATH /usr/local/opt)
endif ()

include(ExternalProject)
include(CPack)
include(FeatureSummary)

option(PSYCHIC_UI_BUILD_SHARED "Build Psychic UI as a shared library?" ON)
add_feature_info("psychic-ui-shared" PSYCHIC_UI_BUILD_SHARED "Build shared library")

option(PSYCHIC_UI_BUILD_SDL2 "SDL2 Support" ON)
add_feature_info("psychic-ui-sdl2" PSYCHIC_UI_BUILD_SDL2 "Build with support for SDL2")

option(PSYCHIC_UI_BUILD_GLFW "GLFW Support" ON)
add_feature_info("psychic-ui-glfw" PSYCHIC_UI_BUILD_GLFW "Build with support for GLFW")

find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(JPEGTURBO REQUIRED)
find_package(Freetype REQUIRED)

if(APPLE)
    # Use brew's version, FindICU can't find it
    set(ICU_LIBRARIES /usr/local/opt/icu4c/lib/libicuuc.a /usr/local/opt/icu4c/lib/libicudata.a)
    set(ICU_INCLUDE_DIRS /usr/local/opt/icu4c/include)
else()
    find_package(ICU REQUIRED COMPONENTS uc)
endif()

set(PSYCHIC_UI_EXTRA_LIBS "")
set(LIBPSYCHIC_UI_EXTRA_SOURCE "")

if (WIN32)
    add_definitions(-DWIN)
elseif (APPLE)
    add_definitions(-DAPPLE)
elseif (UNIX)
    add_definitions(-DUNIX)
endif ()


# GLAD
add_subdirectory(extlib/glad)

# GLFW
if (PSYCHIC_UI_BUILD_GLFW)
    add_subdirectory(extlib/glfw)
    add_definitions(-DWITH_GLFW)
endif ()

# SDL
if (PSYCHIC_UI_BUILD_SDL2)
    find_package(SDL2 REQUIRED)
    if (SDL2_FOUND)
        add_definitions(-DWITH_SDL2)
    endif ()
endif ()

# YOGA
add_subdirectory(extlib/yoga)

# SKIA
add_subdirectory(extlib/skia)


# PSYCHIC COLOR
if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/lib/psychic-color/.git")
    message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif ()

add_subdirectory(lib/psychic-color)

# EXTRAS

# Required core libraries on various platforms
if (WIN32)
    list(APPEND PSYCHIC_UI_EXTRA_LIBS opengl32)
elseif (APPLE)
    list(APPEND LIBPSYCHIC_UI_EXTRA_SOURCE psychic-ui/darwin.mm)
elseif (CMAKE_SYSTEM MATCHES "Linux")
    find_package(Fontconfig REQUIRED)
    list(APPEND PSYCHIC_UI_EXTRA_LIBS
        fontconfig
        )
endif ()

# LIBRARY

set(SOURCE_FILES ${LIBPSYCHIC_UI_EXTRA_SOURCE}
    ${YOGA_SOURCES}
    psychic-ui/components/Button.cpp
    psychic-ui/components/Button.hpp
    psychic-ui/components/CheckBox.cpp
    psychic-ui/components/CheckBox.hpp
    psychic-ui/components/DataContainer.cpp
    psychic-ui/components/DataContainer.hpp
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
    psychic-ui/components/ToolBar.cpp
    psychic-ui/components/ToolBar.hpp
    psychic-ui/signals/Observer.hpp
    psychic-ui/signals/Signal.hpp
    psychic-ui/signals/Slot.hpp
    psychic-ui/skins/DefaultBasicButtonSkin.cpp
    psychic-ui/skins/DefaultBasicButtonSkin.hpp
    psychic-ui/skins/DefaultButtonSkin.cpp
    psychic-ui/skins/DefaultButtonSkin.hpp
    psychic-ui/skins/DefaultCheckBoxSkin.cpp
    psychic-ui/skins/DefaultCheckBoxSkin.hpp
    psychic-ui/skins/DefaultMenuButtonSkin.cpp
    psychic-ui/skins/DefaultMenuButtonSkin.hpp
    psychic-ui/skins/DefaultScrollBarSkin.cpp
    psychic-ui/skins/DefaultScrollBarSkin.hpp
    psychic-ui/skins/SliderRangeSkin.cpp
    psychic-ui/skins/SliderRangeSkin.hpp
    psychic-ui/skins/TitleBarButtonSkin.cpp
    psychic-ui/skins/TitleBarButtonSkin.hpp
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
    psychic-ui/utils/ColorUtils.hpp
    psychic-ui/utils/Hatcher.hpp
    psychic-ui/utils/StringUtils.hpp
    psychic-ui/utils/YogaUtils.hpp
    psychic-ui/Component.hpp
    psychic-ui/Div.cpp
    psychic-ui/Div.hpp
    psychic-ui/Modal.cpp
    psychic-ui/Modal.hpp
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
    psychic-ui/Application.cpp
    psychic-ui/Application.hpp
    psychic-ui/applications/GLFWApplication.cpp
    psychic-ui/applications/GLFWApplication.hpp
    psychic-ui/applications/SDL2Application.cpp
    psychic-ui/applications/SDL2Application.hpp
    psychic-ui/skins/DefaultSkin.hpp
    psychic-ui/components/TextInput.cpp
    psychic-ui/components/TextInput.hpp
    psychic-ui/skins/DefaultTextInputSkin.cpp
    psychic-ui/skins/DefaultTextInputSkin.hpp
    psychic-ui/utils/TextBox.cpp
    psychic-ui/utils/TextBox.hpp
    psychic-ui/components/Text.cpp
    psychic-ui/components/Text.hpp
    psychic-ui/TextBase.cpp
    psychic-ui/TextBase.hpp
    )

add_library(psychic-ui SHARED ${SOURCE_FILES})

# WARNINGS
add_definitions(-Wall)
add_definitions(-Wextra)
#add_definitions(-Werror)

# On Travis' GCC 4.9 it defaults to old C standards, required for Yoga
set_property(TARGET psychic-ui PROPERTY C_STANDARD 11)

target_include_directories(psychic-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${YOGA_INCLUDE_DIRS}
    ${SKIA_INCLUDE_DIRS}
    ${ICU_INCLUDE_DIRS}
    ${PSYCHIC_COLOR_INCLUDE_DIRS}
    )

target_link_libraries(psychic-ui
    # OPENGL
    ${OPENGL_LIBRARY}
    # Skia
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARY}
    ${TURBOJPEG_LIBRARY}
    ${FREETYPE_LIBRARIES}
    ${SKIA_LIBRARIES}
    # Yoga
    ${YOGA_LIBRARIES}
    # ICU
    ${ICU_LIBRARIES}
    # PSYCHIC COLOR
    ${PSYCHIC_COLOR_LIBRARIES}
    )

if (PSYCHIC_UI_BUILD_GLFW)
    target_include_directories(psychic-ui PUBLIC ${GLFW_INCLUDE_DIRS})
    target_link_libraries(psychic-ui glfw)
endif ()

if (PSYCHIC_UI_BUILD_SDL2)
    target_include_directories(psychic-ui PUBLIC ${SDL2_INCLUDE_DIR})
    target_link_libraries(psychic-ui ${SDL2_LIBRARY})
endif ()

if (CMAKE_SYSTEM MATCHES "Linux" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_libraries(psychic-ui c++abi)  # Necessary for Travis Clang builds
endif ()

add_dependencies(psychic-ui psychic-color)
add_dependencies(psychic-ui glfw)
add_dependencies(psychic-ui skia)

if (PSYCHIC_UI_BUILD_SHARED)
    set_property(TARGET psychic-ui PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET psychic-ui APPEND PROPERTY COMPILE_DEFINITIONS "_GLFW_BUILD_DLL")
    target_link_libraries(psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
endif ()

if (PSYCHIC_UI_USE_GLAD)
    add_dependencies(psychic-ui glad)
    target_include_directories(psychic-ui PRIVATE ${GLAD_INCLUDE_DIR})
    target_link_libraries(psychic-ui ${GLAD_LIBRARIES})
    target_compile_definitions(psychic-ui PRIVATE -DPSYCHIC_UI_WITH_GLAD)
    if (PSYCHIC_UI_BUILD_SHARED)
        set_property(TARGET psychic-ui APPEND PROPERTY COMPILE_DEFINITIONS "GLAD_GLAPI_EXPORT;GLAD_GLAPI_EXPORT_BUILD")
    endif ()
endif ()

if (PSYCHIC_UI_BUILD_SHARED AND NOT ${CMAKE_BUILD_TYPE} MATCHES "Deb")
    if (APPLE)
        add_custom_command(TARGET psychic-ui POST_BUILD COMMAND strip -u -r "$<TARGET_FILE_DIR:psychic-ui>/$<TARGET_FILE_NAME:psychic-ui>")
    elseif (UNIX)
        add_custom_command(TARGET psychic-ui POST_BUILD COMMAND strip "$<TARGET_FILE_DIR:psychic-ui>/$<TARGET_FILE_NAME:psychic-ui>")
    endif ()
endif ()

# RESOURCES
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# SUBDIRECTORIES
add_subdirectory(tests)
add_subdirectory(example)

#
# OTHER TARGETS
#

# Documentation
find_package(Doxygen)
if (DOXYGEN_FOUND)
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
feature_summary(WHAT ALL)
