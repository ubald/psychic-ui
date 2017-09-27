option(PSYCHIC_UI_BUILD_EXAMPLE "Build Psychic UI example application?" ON)
add_feature_info("example" PSYCHIC_UI_BUILD_EXAMPLE "Example Application")

if (PSYCHIC_UI_BUILD_EXAMPLE)
    set(DEMO_SOURCES
        demo.cpp
        demo.cpp
        )

    if (PSYCHIC_UI_BUILD_GLFW)
        add_executable(psychic-ui-demo-glfw ${DEMO_SOURCES})
        target_compile_definitions(psychic-ui-demo-glfw PRIVATE -DUSE_GLFW)
        target_link_libraries(psychic-ui-demo-glfw psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
    endif ()

    if (PSYCHIC_UI_BUILD_SDL2)
        add_executable(psychic-ui-demo-sdl2 ${DEMO_SOURCES})
        target_compile_definitions(psychic-ui-demo-sdl2 PRIVATE -DUSE_SDL2)
        target_link_libraries(psychic-ui-demo-sdl2 psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
    endif ()

endif ()
