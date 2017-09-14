option(PSYCHICUI_BUILD_TESTS "Build PsychicUI tests?" ON)
add_feature_info("tests" PSYCHICUI_BUILD_TESTS "Test Suite")

if (PSYCHICUI_BUILD_TESTS)
    ExternalProject_Add(psychic-ui-catch
        GIT_REPOSITORY git@github.com:philsquared/Catch.git
        GIT_TAG master
        CMAKE_ARGS "-DNO_SELFTEST=YES"
        INSTALL_COMMAND ""
        )

    set(CATCH_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/catch-prefix/src/catch/single_include)

    add_executable(psychic_ui_tests
        main.cpp
        style/style_manager_tests.cpp
        style/style_tests.cpp
        style/style_rule_tests.cpp
        style/yoga_tests.cpp)
    target_include_directories(psychic_ui_tests PUBLIC ${CATCH_INCLUDE_DIRS})
    target_link_libraries(psychic_ui_tests psychicui ${PSYCHICUI_EXTRA_LIBS})
    add_dependencies(psychic_ui_tests psychic-ui-catch)
endif()
