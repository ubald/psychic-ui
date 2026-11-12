option(PSYCHIC_UI_BUILD_EXAMPLE "Build Psychic UI example application?" ON)
add_feature_info("example" PSYCHIC_UI_BUILD_EXAMPLE "Example Application")

# The demo runs on SDL3
if (PSYCHIC_UI_BUILD_EXAMPLE AND PSYCHIC_UI_BUILD_SDL3)
    add_executable(psychic-ui-demo demo.cpp)
    target_link_libraries(psychic-ui-demo PRIVATE psychic-ui)

    # RESOURCES
    file(COPY fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif ()
