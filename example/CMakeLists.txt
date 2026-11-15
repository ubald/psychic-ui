option(PSYCHIC_UI_BUILD_EXAMPLE "Build Psychic UI example application?" ${PROJECT_IS_TOP_LEVEL})
add_feature_info("example" PSYCHIC_UI_BUILD_EXAMPLE "Example Application")

if (PSYCHIC_UI_BUILD_EXAMPLE)
    # The demo runs on SDL3
    if (PSYCHIC_UI_BUILD_SDL3)
        add_executable(psychic-ui-demo demo.cpp)
        target_link_libraries(psychic-ui-demo PRIVATE psychic-ui)
        psychic_ui_copy_resources(psychic-ui-demo)
    endif ()

    # A GLFW application that embeds Psychic UI through the adapter
    if (PSYCHIC_UI_BUILD_GLFW_ADAPTER)
        add_executable(psychic-ui-glfw-host glfw-host/main.cpp)
        target_link_libraries(psychic-ui-glfw-host PRIVATE psychic-ui)
        psychic_ui_copy_resources(psychic-ui-glfw-host)
    endif ()

    # RESOURCES: the demo's own fonts (fonts/, relative to the executable's directory like res/)
    file(COPY fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif ()
