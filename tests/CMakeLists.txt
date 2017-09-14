option(PSYCHIC_UI_BUILD_TESTS "Build Psychic UI tests?" ON)
add_feature_info("tests" PSYCHIC_UI_BUILD_TESTS "Test Suite")

if (PSYCHIC_UI_BUILD_TESTS)
    ExternalProject_Add(psychic-ui-catch
        GIT_REPOSITORY https://github.com/philsquared/Catch.git
        GIT_TAG master
        CMAKE_ARGS "-DNO_SELFTEST=YES"
        INSTALL_COMMAND ""
        )

    set(CATCH_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/psychic-ui-catch-prefix/src/psychic-ui-catch/single_include)

    add_executable(psychic-ui-tests
        main.cpp
        style/style_manager_tests.cpp
        style/style_tests.cpp
        style/style_rule_tests.cpp
        style/yoga_tests.cpp)

    target_include_directories(psychic-ui-tests PUBLIC ${CATCH_INCLUDE_DIRS})
    target_link_libraries(psychic-ui-tests psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})
    add_dependencies(psychic-ui-tests psychic-ui-catch)
endif()
