option(PSYCHIC_UI_BUILD_PLAYGROUND "Build Psychic UI playground?" ON)
add_feature_info("playground" PSYCHIC_UI_BUILD_PLAYGROUND "Playground Application")

# The playground runs on SDL3
if (PSYCHIC_UI_BUILD_PLAYGROUND AND PSYCHIC_UI_BUILD_SDL3)
    add_executable(psychic-ui-playground playground.hpp playground.cpp)
    target_link_libraries(psychic-ui-playground PRIVATE psychic-ui)
endif ()
