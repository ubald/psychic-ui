# Another project using Psychic UI from source with add_subdirectory, as an application would: the test
# `psychic-ui-consumer` (tests/CMakeLists.txt, PSYCHIC_UI_TEST_CONSUMER) configures, builds and runs it
cmake_minimum_required(VERSION 3.24)
project(psychic-ui-consumer C CXX)

# An application that builds its own libraries shared: Psychic UI must neither follow it nor change it
set(BUILD_SHARED_LIBS ON)

if (NOT PSYCHIC_UI_SOURCE_DIR)
    message(FATAL_ERROR "Pass the Psychic UI checkout with -DPSYCHIC_UI_SOURCE_DIR=<path>")
endif ()
add_subdirectory(${PSYCHIC_UI_SOURCE_DIR} psychic-ui)

# What the library builds for itself stays out of its consumers' builds
foreach (_target psychic-ui-tests psychic-ui-demo psychic-ui-playground)
    if (TARGET ${_target})
        message(FATAL_ERROR "Psychic UI built its ${_target} target inside another project")
    endif ()
endforeach ()
get_target_property(_psychic_ui_type psychic-ui TYPE)
if (NOT _psychic_ui_type STREQUAL "STATIC_LIBRARY")
    message(FATAL_ERROR "Psychic UI is a ${_psychic_ui_type} inside another project, not a STATIC_LIBRARY")
endif ()
if (NOT BUILD_SHARED_LIBS)
    message(FATAL_ERROR "Psychic UI changed the including project's BUILD_SHARED_LIBS")
endif ()
if (DEFINED CACHE{BUILD_SHARED_LIBS})
    message(FATAL_ERROR "Psychic UI put BUILD_SHARED_LIBS in the cache, overriding the including project's")
endif ()

add_executable(consumer-app main.cpp)
target_link_libraries(consumer-app PRIVATE psychic-ui::psychic-ui)
psychic_ui_copy_resources(consumer-app)
