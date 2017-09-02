if (WIN32)
    set(PSYCHICUI_USE_GLAD_DEFAULT ON)
else ()
    set(PSYCHICUI_USE_GLAD_DEFAULT OFF)
endif ()

option(PSYCHICUI_USE_GLAD "Use Glad OpenGL loader library?" ${PSYCHICUI_USE_GLAD_DEFAULT})

if (PSYCHICUI_USE_GLAD)
    # Build and include GLAD
    ExternalProject_Add(glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad
        GIT_TAG master
        CMAKE_ARGS -DGLAD_API=gl=4.5
        CMAKE_ARGS -DGLAD_PROFILE=core
        CMAKE_ARGS -DGLAD_GENERATOR=c
        CMAKE_ARGS -DGLAD_EXPORT=OFF
        CMAKE_ARGS -DGLAD_INSTALL=OFF
        INSTALL_COMMAND ""
        )

    set(GLAD_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/glad-prefix/src/glad-build/include)
    set(GLAD_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/glad-prefix/src/glad-build/${CMAKE_STATIC_LIBRARY_PREFIX}glad${CMAKE_STATIC_LIBRARY_SUFFIX})

    include_directories(
        ${GLAD_INCLUDE_DIRS}
    )

    link_libraries(
        ${GLAD_LIBRARIES}
    )

    list(APPEND PSYCHICUI_EXTRA_DEFS -DPSYCHICUI_GLAD)
    if (PSYCHICUI_BUILD_SHARED)
        list(APPEND PSYCHICUI_EXTRA_DEFS -DGLAD_GLAPI_EXPORT)
    endif ()
endif ()
