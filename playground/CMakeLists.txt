option(PSYCHIC_UI_BUILD_PLAYGROUND "Build Psychic UI playground?" ON)
add_feature_info("playground" PSYCHIC_UI_BUILD_PLAYGROUND "Test scratch pad")

if (PSYCHIC_UI_BUILD_PLAYGROUND)
    add_executable(psychic-ui-playground
        playground.cpp
    )
    target_link_libraries(psychic-ui-playground psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
endif()
