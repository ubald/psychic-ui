option(PSYCHIC_UI_BUILD_EXAMPLE "Build Psychic UI example application?" ON)
add_feature_info("example" PSYCHIC_UI_BUILD_EXAMPLE "Example Application")

if (PSYCHIC_UI_BUILD_EXAMPLE)
    set(DEMO_SOURCES
        demo.cpp
        )

    if (PSYCHIC_UI_BUILD_GLFW)
        add_executable(psychic-ui-demo-glfw ${DEMO_SOURCES})
        target_compile_definitions(psychic-ui-demo-glfw PRIVATE USE_GLFW)
        target_link_libraries(psychic-ui-demo-glfw PRIVATE psychic-ui)
    endif ()

    if (PSYCHIC_UI_BUILD_SDL2)
        add_executable(psychic-ui-demo-sdl2 ${DEMO_SOURCES})
        target_compile_definitions(psychic-ui-demo-sdl2 PRIVATE USE_SDL2)
        target_link_libraries(psychic-ui-demo-sdl2 PRIVATE psychic-ui)
    endif ()


    # RESOURCES
    file(COPY fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

endif ()
