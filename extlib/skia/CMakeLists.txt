if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../external/skia/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../external/depot_tools/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

set(SKIA_ARGS_LIST skia_use_dng_sdk=false skia_use_expat=false skia_use_libwebp=false skia_use_piex=false skia_use_freetype=true)
list(APPEND SKIA_ARGS_LIST "skia_use_sfntly=false")  # PDF font subsetting, unused; its C++14 can't include ICU 64+ headers

if (APPLE)
    # Homebrew's keg-only ICU and jpeg-turbo, found through CMAKE_PREFIX_PATH by the root CMakeLists
    list(APPEND SKIA_ARGS_LIST "extra_cflags=[\"-I${ICU_INCLUDE_DIR}\", \"-I${JPEG_INCLUDE_DIR}\", \"-I${FREETYPE_INCLUDE_DIR_freetype2}\"]")
    list(APPEND SKIA_ARGS_LIST "extra_ldflags=[\"-L${ICU_INCLUDE_DIR}/../lib\", \"-L${JPEG_INCLUDE_DIR}/../lib\"]")
elseif (CMAKE_SYSTEM MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "BSD")
#    list(APPEND SKIA_ARGS_LIST "extra_cflags=[\"-I${JPEGTURBO_INCLUDE_DIR}\"]")
#    list(APPEND SKIA_ARGS_LIST "extra_ldflags=[\"-L${TURBOJPEG_LIBRARY}\"]")
    list(APPEND SKIA_ARGS_LIST "skia_use_system_libjpeg_turbo=false")  # Unfortunately Ubuntu 16.04 does't have this one at the right version
    list(APPEND SKIA_ARGS_LIST "extra_cflags_cc=[\"-include\", \"cstdint\"]")  # GCC 13+ no longer includes it transitively
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

add_custom_target(skia-build DEPENDS ${SKIA_STATIC_LIB})

# The m73 headers include each other by bare name, so every include/ and src/ subdirectory is on the path
file(GLOB SKIA_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../external/skia/include/* ${CMAKE_CURRENT_LIST_DIR}/../external/skia/src/*)

add_library(Skia::Skia STATIC IMPORTED GLOBAL)
set_target_properties(Skia::Skia PROPERTIES IMPORTED_LOCATION ${SKIA_STATIC_LIB})
target_include_directories(Skia::Skia INTERFACE ${SKIA_INCLUDE_DIRS})
target_link_libraries(Skia::Skia INTERFACE
    PNG::PNG
    JPEG::JPEG
    ZLIB::ZLIB
    Freetype::Freetype
    ICU::uc
    Threads::Threads
    ${CMAKE_DL_LIBS}
    $<$<TARGET_EXISTS:Fontconfig::Fontconfig>:Fontconfig::Fontconfig>
    )
add_dependencies(Skia::Skia skia-build)
