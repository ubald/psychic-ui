option(PSYCHIC_UI_BUILD_TESTS "Build Psychic UI tests?" ${PROJECT_IS_TOP_LEVEL})
add_feature_info("psychic-ui-tests" PSYCHIC_UI_BUILD_TESTS "Psychic UI tests")

if (PSYCHIC_UI_BUILD_TESTS)

    # An installed Catch2 3 if there is one that works here, otherwise this
    # release, built static. Catch2's library only defines the std::string_view
    # StringMaker when it was itself compiled as C++17 or newer, while the
    # tests' C++23 build of its headers expects it, so the installed one must
    # first link a program that stringifies a string_view. catch2-check is a
    # project of its own (try_compile), so the installed Catch2's imported
    # targets don't clash with a download when it fails; without an installed
    # Catch2 it fails to build with a marker, never to configure.
    set(_catch2_find_args)
    if (NOT CMAKE_DISABLE_FIND_PACKAGE_Catch2)
        set(_catch2_check_key "${CMAKE_CXX_COMPILER}|${CMAKE_CXX_FLAGS}|${CMAKE_BUILD_TYPE}|${CMAKE_PREFIX_PATH}|$ENV{CMAKE_PREFIX_PATH}|${Catch2_DIR}|${CMAKE_IGNORE_PREFIX_PATH}|${CMAKE_IGNORE_PATH}|${CMAKE_OSX_ARCHITECTURES}|${CMAKE_OSX_SYSROOT}")
        if (NOT DEFINED PSYCHIC_UI_CATCH2_INSTALLED OR NOT _catch2_check_key STREQUAL PSYCHIC_UI_CATCH2_CHECK_KEY)
            set(_catch2_check_flags)
            foreach (_var CMAKE_TOOLCHAIN_FILE CMAKE_CXX_COMPILER CMAKE_BUILD_TYPE CMAKE_CXX_FLAGS CMAKE_EXE_LINKER_FLAGS
                    CMAKE_PREFIX_PATH Catch2_DIR CMAKE_IGNORE_PREFIX_PATH CMAKE_IGNORE_PATH
                    CMAKE_OSX_ARCHITECTURES CMAKE_OSX_SYSROOT CMAKE_OSX_DEPLOYMENT_TARGET)
                if (DEFINED ${_var})
                    string(REPLACE ";" "\\;" _value "${${_var}}")
                    list(APPEND _catch2_check_flags "-D${_var}=${_value}")
                endif ()
            endforeach ()
            set(_catch2_check_dir "${CMAKE_CURRENT_BINARY_DIR}/catch2-check")
            file(REMOVE_RECURSE "${_catch2_check_dir}")
            try_compile(_catch2_check_ok "${_catch2_check_dir}" "${CMAKE_CURRENT_SOURCE_DIR}/catch2-check" catch2-check
                CMAKE_FLAGS ${_catch2_check_flags}
                OUTPUT_VARIABLE _catch2_check_output)
            if (_catch2_check_ok)
                set(_catch2_state "usable")
            elseif (_catch2_check_output MATCHES "PSYCHIC_UI_CATCH2_NOT_FOUND")
                set(_catch2_state "missing")
            else ()
                set(_catch2_state "unusable")
                file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/catch2-check.log" "${_catch2_check_output}")
            endif ()
            set(PSYCHIC_UI_CATCH2_INSTALLED "${_catch2_state}" CACHE INTERNAL "Whether the installed Catch2 3 is usable")
            set(PSYCHIC_UI_CATCH2_CHECK_KEY "${_catch2_check_key}" CACHE INTERNAL "What the Catch2 check ran with")
        endif ()
        if (PSYCHIC_UI_CATCH2_INSTALLED STREQUAL "usable")
            message(STATUS "Catch2: using the installed Catch2 3")
            set(_catch2_find_args FIND_PACKAGE_ARGS 3)
        elseif (PSYCHIC_UI_CATCH2_INSTALLED STREQUAL "missing")
            message(STATUS "Catch2: no installed Catch2 3, downloading v3.16.1")
        else ()
            message(STATUS "Catch2: the installed Catch2 3 can't link a test that stringifies a "
                "std::string_view (its library was likely built before C++17), downloading v3.16.1; "
                "see ${CMAKE_CURRENT_BINARY_DIR}/catch2-check.log")
        endif ()
    endif ()

    set(BUILD_SHARED_LIBS OFF)
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.16.1
        GIT_SHALLOW TRUE
        ${_catch2_find_args}
        )
    # A download builds as C++23 like the tests, whatever the compiler's
    # default: Catch2 only asks for C++14, which AppleClang would then use,
    # leaving out the string_view StringMaker for the same reason as above.
    if (DEFINED CMAKE_CXX_STANDARD)
        set(_catch2_saved_std "${CMAKE_CXX_STANDARD}")
    endif ()
    set(CMAKE_CXX_STANDARD 23)
    FetchContent_MakeAvailable(Catch2)
    if (DEFINED _catch2_saved_std)
        set(CMAKE_CXX_STANDARD "${_catch2_saved_std}")
    else ()
        unset(CMAKE_CXX_STANDARD)
    endif ()

    add_executable(psychic-ui-tests
        style/style_manager_tests.cpp
        style/theme_tests.cpp
        skins/skin_set_tests.cpp
        skins/default_skins_tests.cpp
        skins/desk_skins_tests.cpp
        skins/float_skins_tests.cpp
        demo/canvas_page_tests.cpp
        components/app_shell_tests.cpp
        components/list_tests.cpp
        components/input_tests.cpp
        components/popup_tests.cpp
        components/popup_motion_tests.cpp
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
        div/mouse_event_tests.cpp
        div/opacity_tests.cpp
        div/transform_tests.cpp
        div/outline_shadow_tests.cpp
        icons/icon_tests.cpp
        backends/system_window_tests.cpp
        backends/touch_tests.cpp
        text/text_layout_tests.cpp
        text/font_tests.cpp
        signals/signal_tests.cpp
        signals/lifetime_tests.cpp
        animation/animation_tests.cpp
        animation/layout_transition_tests.cpp
        components/reentrancy_tests.cpp
        # Known bugs ([!shouldfail]) and hidden use-after-free cases ([asan]) from docs/review-2026-11.md
        keyboard/key_routing_tests.cpp
        text/editing_tests.cpp
        style/selector_tests.cpp
        div/press_tests.cpp
        div/scroll_tests.cpp
        div/focus_tests.cpp
        div/ownership_tests.cpp
        components/modal_input_tests.cpp
        components/rows_tests.cpp
        components/number_stepper_tests.cpp
        asan/use_after_free_tests.cpp)

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
