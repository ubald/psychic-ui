option(PSYCHIC_UI_BUILD_SHOWCASE "Build the showcase, which captures the website's screenshots?" ${PROJECT_IS_TOP_LEVEL})
add_feature_info("showcase" PSYCHIC_UI_BUILD_SHOWCASE "Showcase: the website's screenshots, rendered offscreen")

if (PSYCHIC_UI_BUILD_SHOWCASE)
    # Renders on the CPU into a raster Skia surface, with no window: SDL3 is only for --live
    add_executable(psychic-ui-showcase
        main.cpp
        Compose.cpp
        Compose.hpp
        HeadlessSystemWindow.cpp
        HeadlessSystemWindow.hpp
        Looks.cpp
        Looks.hpp
        PngWriter.cpp
        PngWriter.hpp
        Scene.hpp
        Scenes.cpp
        ShowcaseStyleSheet.cpp
        ShowcaseStyleSheet.hpp
        Stage.cpp
        Stage.hpp
        scenes/Collections.cpp
        scenes/Controls.cpp
        scenes/Foundations.cpp
        scenes/Hero.cpp
        scenes/Popups.cpp
        )
    target_link_libraries(psychic-ui-showcase PRIVATE psychic-ui)
    if (PSYCHIC_UI_BUILD_SDL3)
        target_compile_definitions(psychic-ui-showcase PRIVATE PSYCHIC_UI_SHOWCASE_LIVE)
    endif ()
    psychic_ui_copy_resources(psychic-ui-showcase)
    # The demo's own font, which its stylesheet loads (the Canvas and app scenes are the demo's)
    add_custom_command(TARGET psychic-ui-showcase POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PSYCHIC_UI_DIR}/example/fonts $<TARGET_FILE_DIR:psychic-ui-showcase>/fonts)

    # Captures every scene into <build dir>/showcase/captures (tools/showcase/publish.sh publishes them)
    add_custom_target(showcase-capture
        COMMAND psychic-ui-showcase --out ${CMAKE_CURRENT_BINARY_DIR}/captures
        DEPENDS psychic-ui-showcase
        COMMENT "Capturing the showcase into ${CMAKE_CURRENT_BINARY_DIR}/captures"
        USES_TERMINAL
        VERBATIM)
endif ()
