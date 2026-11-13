option(PSYCHIC_UI_BUILD_TESTS "Build Psychic UI tests?" ON)
add_feature_info("psychic-ui-tests" PSYCHIC_UI_BUILD_TESTS "Psychic UI tests")

if (PSYCHIC_UI_BUILD_TESTS)

    # An installed Catch2 3 if there is one, otherwise this release, built static
    set(BUILD_SHARED_LIBS OFF)
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.16.1
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS 3
        )
    FetchContent_MakeAvailable(Catch2)

    add_executable(psychic-ui-tests
        style/style_manager_tests.cpp
        style/theme_tests.cpp
        skins/skin_set_tests.cpp
        style/style_tests.cpp
        style/style_rule_tests.cpp
        style/cascade_tests.cpp
        style/computed_style_tests.cpp
        style/values_tests.cpp
        style/yoga_tests.cpp
        layout/layout_pipeline_tests.cpp
        layout/measure_tests.cpp
        div/children_tests.cpp
        div/api_tests.cpp
        div/event_tests.cpp
        div/opacity_tests.cpp
        div/outline_shadow_tests.cpp
        icons/icon_tests.cpp
        backends/system_window_tests.cpp
        text/text_layout_tests.cpp
        text/font_tests.cpp
        signals/signal_tests.cpp
        signals/lifetime_tests.cpp)

    if (PSYCHIC_UI_BUILD_SDL3)
        target_sources(psychic-ui-tests PRIVATE
            keyboard/keycodes.cpp
            backends/sdl3_tests.cpp)
    endif ()

    if (PSYCHIC_UI_BUILD_GLFW_ADAPTER)
        target_sources(psychic-ui-tests PRIVATE
            backends/glfw_tests.cpp)
    endif ()

    # yogacore too: some tests check the Yoga nodes directly
    target_link_libraries(psychic-ui-tests PRIVATE psychic-ui yogacore Catch2::Catch2WithMain)
    target_compile_definitions(psychic-ui-tests PRIVATE
        PSYCHIC_UI_TEST_FONTS="${PROJECT_SOURCE_DIR}/res/fonts/Ubuntu/"
        PSYCHIC_UI_TEST_RES="${PROJECT_SOURCE_DIR}/res/")

    add_test(NAME psychic-ui-tests COMMAND psychic-ui-tests)

endif()
