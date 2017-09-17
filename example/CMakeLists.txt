option(PSYCHIC_UI_BUILD_EXAMPLE "Build Psychic UI example application?" ON)
add_feature_info("example" PSYCHIC_UI_BUILD_EXAMPLE "Example Application")

if (PSYCHIC_UI_BUILD_EXAMPLE)
    add_executable(psychic-ui-demo
        demo.cpp
        components/ToolBar.cpp
    )
    target_link_libraries(psychic-ui-demo psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
endif ()
