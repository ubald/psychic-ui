option(PSYCHIC_UI_BUILD_PLAYGROUND "Build Psychic UI playground?" ON)
add_feature_info("playground" PSYCHIC_UI_BUILD_PLAYGROUND "Playground Application")

if (PSYCHIC_UI_BUILD_PLAYGROUND)
    set(PLAYGROUND_SOURCES
        playground.hpp
        playground.cpp
        )

    if (PSYCHIC_UI_BUILD_GLFW)
        add_executable(psychic-ui-playground-glfw ${PLAYGROUND_SOURCES})
        target_compile_definitions(psychic-ui-playground-glfw PRIVATE USE_GLFW)
        target_link_libraries(psychic-ui-playground-glfw PRIVATE psychic-ui)
    endif ()

    if (PSYCHIC_UI_BUILD_SDL2)
        add_executable(psychic-ui-playground-sdl2 ${PLAYGROUND_SOURCES})
        target_compile_definitions(psychic-ui-playground-sdl2 PRIVATE USE_SDL2)
        target_link_libraries(psychic-ui-playground-sdl2 PRIVATE psychic-ui)
    endif ()

endif ()
