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
        style/style_tests.cpp
        style/style_rule_tests.cpp
        style/yoga_tests.cpp
        layout/layout_pipeline_tests.cpp
        div/children_tests.cpp
        keyboard/keycodes.cpp
        text/text_layout_tests.cpp)

    target_link_libraries(psychic-ui-tests PRIVATE psychic-ui Catch2::Catch2WithMain)
    target_compile_definitions(psychic-ui-tests PRIVATE
        PSYCHIC_UI_TEST_FONTS="${PROJECT_SOURCE_DIR}/res/fonts/Ubuntu/")

    add_test(NAME psychic-ui-tests COMMAND psychic-ui-tests)

endif()
