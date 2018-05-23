if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../external/skia/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../external/depot_tools/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

set(SKIA_ARGS_LIST skia_use_dng_sdk=false skia_use_expat=false skia_use_libwebp=false skia_use_piex=false skia_use_freetype=true)

if (APPLE)
    list(APPEND SKIA_ARGS_LIST "extra_cflags=[\"-I/usr/local/opt/icu4c/include\", \"-I/usr/local/opt/jpeg-turbo/include\", \"-I/usr/local/opt/freetype/include/freetype2\"]")
    list(APPEND SKIA_ARGS_LIST "extra_ldflags=[\"-L/usr/local/opt/icu4c/lib\", \"-L/usr/local/opt/jpeg-turbo\", \"-L/usr/local/opt/freetype\"]")
elseif (CMAKE_SYSTEM MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "BSD")
#    list(APPEND SKIA_ARGS_LIST "extra_cflags=[\"-I${JPEGTURBO_INCLUDE_DIR}\"]")
#    list(APPEND SKIA_ARGS_LIST "extra_ldflags=[\"-L${TURBOJPEG_LIBRARY}\"]")
    list(APPEND SKIA_ARGS_LIST "skia_use_system_libjpeg_turbo=false")  # Unfortunately Ubuntu 16.04 does't have this one at the right version
endif ()

string(REPLACE ";" " " SKIA_ARGS "${SKIA_ARGS_LIST}")
set(NINJA ${CMAKE_CURRENT_LIST_DIR}/../external/depot_tools/ninja)
set(SKIA_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build)

configure_file("skia_build.sh.in" "skia_build.sh" @ONLY)

set(SKIA_STATIC_LIB ${CMAKE_CURRENT_BINARY_DIR}/build/${CMAKE_STATIC_LIBRARY_PREFIX}skia${CMAKE_STATIC_LIBRARY_SUFFIX})

add_custom_command(OUTPUT ${SKIA_STATIC_LIB}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/skia_build.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../external/skia
)

add_custom_target(skia DEPENDS ${SKIA_STATIC_LIB})

file(GLOB SKIA_INCLUDE_DIRS_ ${CMAKE_CURRENT_LIST_DIR}/../external/skia/include/* ${CMAKE_CURRENT_LIST_DIR}/../external/skia/src/*)
set(SKIA_INCLUDE_DIRS "${SKIA_INCLUDE_DIRS_}" PARENT_SCOPE)
set(SKIA_LIBRARIES ${SKIA_STATIC_LIB} PARENT_SCOPE)
