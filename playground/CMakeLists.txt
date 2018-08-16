option(PSYCHIC_UI_BUILD_PLAYGROUND "Build Psychic UI playground?" ON)
add_feature_info("playground" PSYCHIC_UI_BUILD_PLAYGROUND "Playground Application")

if (PSYCHIC_UI_BUILD_PLAYGROUND)
    set(PLAYGROUND_SOURCES
        playground.hpp
        playground.cpp
        )

    if (PSYCHIC_UI_BUILD_GLFW)
        add_executable(psychic-ui-playground-glfw ${PLAYGROUND_SOURCES})
        target_compile_definitions(psychic-ui-playground-glfw PRIVATE -DUSE_GLFW)
        target_link_libraries(psychic-ui-playground-glfw psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
    endif ()

    if (PSYCHIC_UI_BUILD_SDL2)
        add_executable(psychic-ui-playground-sdl2 ${PLAYGROUND_SOURCES})
        target_compile_definitions(psychic-ui-playground-sdl2 PRIVATE -DUSE_SDL2)
        target_link_libraries(psychic-ui-playground-sdl2 psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
    endif ()

endif ()
