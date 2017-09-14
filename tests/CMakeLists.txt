option(PSYCHIC_UI_BUILD_TESTS "Build Psychic UI tests?" ON)
add_feature_info("psychic-ui-tests" PSYCHIC_UI_BUILD_TESTS "Psychic UI tests")

if (PSYCHIC_UI_BUILD_TESTS)

    if (NOT TARGET catch)
        ExternalProject_Add(catch
            GIT_REPOSITORY https://github.com/philsquared/Catch.git
            GIT_TAG master
            PREFIX "${CMAKE_BINARY_DIR}/catch"
            CMAKE_ARGS "-DNO_SELFTEST=YES"
            INSTALL_COMMAND ""
            )

        set(CATCH_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/catch/src/catch/single_include CACHE INTERNAL "Global catch include")
    endif()

    add_executable(psychic-ui-tests
        main.cpp
        style/style_manager_tests.cpp
        style/style_tests.cpp
        style/style_rule_tests.cpp
        style/yoga_tests.cpp
    )

    target_include_directories(psychic-ui-tests PUBLIC ${CATCH_INCLUDE_DIRS})
    target_link_libraries(psychic-ui-tests psychic-ui ${PSYCHIC_UI_EXTRA_LIBS})

    add_dependencies(psychic-ui-tests psychic-ui)
    add_dependencies(psychic-ui-tests catch)

endif()
