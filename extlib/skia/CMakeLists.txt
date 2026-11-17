set(SKIA_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/skia)

if (NOT EXISTS "${SKIA_SOURCE_DIR}/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

# Skia's build runs Python 3 scripts (gn's exec_script calls) and ninja.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_program(SKIA_NINJA ninja REQUIRED)

# SkShaper shapes with the system HarfBuzz. CMake has no Find module for it.
find_path(HARFBUZZ_INCLUDE_DIR hb.h PATH_SUFFIXES harfbuzz REQUIRED)
find_library(HARFBUZZ_LIBRARY harfbuzz REQUIRED)

# What Psychic UI uses: Ganesh on OpenGL, fonts, text shaping and SkParagraph (ICU for Unicode,
# HarfBuzz for shaping). An official build takes ICU, HarfBuzz and FreeType from the system.
# Everything else is off: image codecs, PDF, SVG, Skottie, Graphite, Vulkan, Metal, tracing,
# and Skia's own GL loaders (Psychic UI assembles its GL interface from the window's procs).
set(SKIA_ARGS_LIST
    is_official_build=true
    "cc=\"${CMAKE_C_COMPILER}\""
    "cxx=\"${CMAKE_CXX_COMPILER}\""
    skia_enable_ganesh=true
    skia_use_gl=true
    skia_enable_graphite=false
    skia_use_vulkan=false
    skia_use_metal=false
    skia_use_dawn=false
    skia_use_egl=false
    skia_use_x11=false
    skia_enable_pdf=false
    skia_enable_svg=false
    skia_enable_skottie=false
    skia_enable_tools=false
    skia_use_libpng_decode=false
    skia_use_libpng_encode=false
    skia_use_libjpeg_turbo_decode=false
    skia_use_libjpeg_turbo_encode=false
    skia_use_libwebp_decode=false
    skia_use_libwebp_encode=false
    skia_use_wuffs=false
    skia_use_dng_sdk=false
    skia_use_piex=false
    skia_use_expat=false
    skia_use_xps=false
    skia_use_zlib=false
    skia_use_lua=false
    skia_use_perfetto=false
    skia_use_partition_alloc=false
    skia_use_icu=true
    skia_use_harfbuzz=true
    skia_pdf_subset_harfbuzz=false
    skia_enable_skshaper=true
    skia_enable_skunicode=true
    skia_enable_skparagraph=true
    skia_enable_skshaper_tests=false
    skunicode_tests_enabled=false
    paragraph_gms_enabled=false
    paragraph_tests_enabled=false
    paragraph_bench_enabled=false
    )

# Header and library directories of the system libraries Skia compiles against, which aren't
# necessarily on the compiler's default paths (Nix, Homebrew's keg-only ICU). Skia only builds
# static libraries, so the -L flags are for anything gn would link, not for the .a files.
set(SKIA_INCLUDE_FLAGS "-I${ICU_INCLUDE_DIR}" "-I${HARFBUZZ_INCLUDE_DIR}")
get_filename_component(SKIA_ICU_LIB_DIR "${ICU_UC_LIBRARY}" DIRECTORY)
get_filename_component(SKIA_HARFBUZZ_LIB_DIR "${HARFBUZZ_LIBRARY}" DIRECTORY)
set(SKIA_LIBRARY_FLAGS "-L${SKIA_ICU_LIB_DIR}" "-L${SKIA_HARFBUZZ_LIB_DIR}")

if (APPLE)
    # CoreText fonts, no FreeType or Fontconfig
    list(APPEND SKIA_ARGS_LIST skia_use_fonthost_mac=true skia_use_freetype=false skia_use_fontconfig=false)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD")
    list(APPEND SKIA_ARGS_LIST skia_use_freetype=true skia_use_fontconfig=true)
    # Skia's system FreeType config only knows /usr/include/freetype2
    list(APPEND SKIA_ARGS_LIST "skia_system_freetype2_include_path=\"${FREETYPE_INCLUDE_DIR_freetype2}\"")
    list(APPEND SKIA_INCLUDE_FLAGS "-I${Fontconfig_INCLUDE_DIRS}")
endif ()

list(TRANSFORM SKIA_INCLUDE_FLAGS REPLACE "(.+)" "\"\\1\"")
list(TRANSFORM SKIA_LIBRARY_FLAGS REPLACE "(.+)" "\"\\1\"")
list(JOIN SKIA_INCLUDE_FLAGS ", " SKIA_INCLUDE_FLAGS)
list(JOIN SKIA_LIBRARY_FLAGS ", " SKIA_LIBRARY_FLAGS)
list(APPEND SKIA_ARGS_LIST "extra_cflags=[${SKIA_INCLUDE_FLAGS}]" "extra_ldflags=[${SKIA_LIBRARY_FLAGS}]")

set(PSYCHIC_UI_SKIA_DIR "" CACHE PATH "A prebuilt Skia to link from, read-only, to share one build between build directories (default: build it into this build directory's extlib/skia/build)")
if (PSYCHIC_UI_SKIA_DIR)
    # A relative directory (set by a parent project or a preset) is the build directory's: CMake already made one given
    # with -D absolute, from the working directory
    cmake_path(ABSOLUTE_PATH PSYCHIC_UI_SKIA_DIR BASE_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE SKIA_OUTPUT)
else ()
    set(SKIA_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build)
endif ()
set(PSYCHIC_UI_SKIA_OUTPUT "${SKIA_OUTPUT}" CACHE INTERNAL "The directory Skia is linked from (built into it, unless PSYCHIC_UI_SKIA_DIR gives a read-only prebuilt one)")

if (NOT PSYCHIC_UI_SKIA_DIR)
    # gn reads the args from the output directory's args.gn, one per line. A prebuilt Skia is never written to.
    list(JOIN SKIA_ARGS_LIST "\n" SKIA_ARGS)
    file(WRITE ${SKIA_OUTPUT}/args.gn "${SKIA_ARGS}\n")

    # The gn Skia pins, downloaded here rather than by Skia's bin/fetch-gn, which writes into Skia's checkout. The
    # revision is bin/fetch-gn's (it extracts the same package): update both together when Skia moves.
    set(SKIA_GN_REVISION b2afae122eeb6ce09c52d63f67dc53fc517dbdc8)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
        set(SKIA_GN_PLATFORM linux-amd64)
        set(SKIA_GN_SHA256 6a3bfc53d825bccac5e5b4b7bdcc10ce9396a04a689ac684ec3308ca5c7f3d9c)
    elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
        set(SKIA_GN_PLATFORM linux-arm64)
        set(SKIA_GN_SHA256 376e1358ded109d955b8eb55ba2a7dec0e95ab1f8dfe0e18011e06eed78a011f)
    elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
        set(SKIA_GN_PLATFORM mac-amd64)
        set(SKIA_GN_SHA256 4bea817159f060980f1ca1d0400ef6cd2e0ab0ade66bb34e91aab9c2cffc9d83)
    elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
        set(SKIA_GN_PLATFORM mac-arm64)
        set(SKIA_GN_SHA256 cbb260f2cf999f7d74da96b3828a061299c6337354dd6db642e778828576025c)
    elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
        set(SKIA_GN_PLATFORM windows-amd64)
        set(SKIA_GN_SHA256 7249f55d6f22b55affa68aaf19d2f597c39946299216e12d78eac6cc1469f8a1)
    else ()
        message(FATAL_ERROR "No gn is published for ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}: Skia can't be built here. "
            "Use -DPSYCHIC_UI_SKIA_DIR=<a prebuilt Skia>.")
    endif ()
    set(SKIA_GN ${CMAKE_CURRENT_BINARY_DIR}/gn/gn${CMAKE_EXECUTABLE_SUFFIX})
    if (NOT EXISTS "${SKIA_GN}")
        set(SKIA_GN_ZIP ${CMAKE_CURRENT_BINARY_DIR}/gn-${SKIA_GN_PLATFORM}.zip)
        file(DOWNLOAD
            "https://chrome-infra-packages.appspot.com/dl/gn/gn/${SKIA_GN_PLATFORM}/+/git_revision:${SKIA_GN_REVISION}"
            ${SKIA_GN_ZIP} EXPECTED_HASH SHA256=${SKIA_GN_SHA256})
        file(ARCHIVE_EXTRACT INPUT ${SKIA_GN_ZIP} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/gn)
        file(REMOVE ${SKIA_GN_ZIP})
    endif ()
endif ()

# The static libraries Psychic UI links, in link order (each before what it uses)
set(SKIA_LIBRARY_NAMES skparagraph skshaper skunicode_icu skunicode_core skia skcms)
set(SKIA_STATIC_LIBS)
foreach (name ${SKIA_LIBRARY_NAMES})
    list(APPEND SKIA_STATIC_LIBS ${SKIA_OUTPUT}/${CMAKE_STATIC_LIBRARY_PREFIX}${name}${CMAKE_STATIC_LIBRARY_SUFFIX})
endforeach ()

if (PSYCHIC_UI_SKIA_DIR)
    # A given Skia is a prebuilt, read-only input: its libraries are no build's outputs, so no clean deletes them, and
    # skia-build does nothing
    foreach (library ${SKIA_STATIC_LIBS})
        if (NOT EXISTS "${library}")
            message(FATAL_ERROR "PSYCHIC_UI_SKIA_DIR has no ${library}: it must hold a built Skia. Build the skia-build "
                "target in a Psychic UI build directory configured without PSYCHIC_UI_SKIA_DIR, then point "
                "PSYCHIC_UI_SKIA_DIR at that build directory's extlib/skia/build.")
        endif ()
    endforeach ()
    add_custom_target(skia-build)
else ()
    add_custom_command(OUTPUT ${SKIA_STATIC_LIBS}
        COMMAND ${SKIA_GN} gen ${SKIA_OUTPUT} --script-executable=${Python3_EXECUTABLE}
        COMMAND ${SKIA_NINJA} -C ${SKIA_OUTPUT} skia modules/skparagraph modules/skshaper modules/skunicode
        WORKING_DIRECTORY ${SKIA_SOURCE_DIR}
        USES_TERMINAL
        VERBATIM
    )
    add_custom_target(skia-build DEPENDS ${SKIA_STATIC_LIBS})
endif ()

# Skia's headers include each other from the source root ("include/core/SkCanvas.h"), and its
# modules the same way ("modules/skparagraph/include/Paragraph.h")
add_library(Skia::Skia INTERFACE IMPORTED GLOBAL)
target_include_directories(Skia::Skia INTERFACE ${SKIA_SOURCE_DIR})
# The public defines of the configuration Skia was built with (gn's skia_public config and the
# modules' public configs), which its headers depend on. SK_RELEASE because the libraries are a
# release build: a debug consumer would otherwise get SK_DEBUG, which changes class layouts.
target_compile_definitions(Skia::Skia INTERFACE
    SK_RELEASE
    SK_CODEC_DECODES_BMP
    SK_CODEC_DECODES_WBMP
    SK_ENABLE_PRECOMPILE
    SK_ENABLE_YUVA_PACKED_422
    SK_DISABLE_TRACING
    SK_GANESH
    SK_GL
    SK_UNICODE_AVAILABLE
    SK_UNICODE_ICU_IMPLEMENTATION
    SK_SHAPER_HARFBUZZ_AVAILABLE
    SK_SHAPER_UNICODE_AVAILABLE
    SK_SHAPER_PRIMITIVE_AVAILABLE
    SK_ENABLE_PARAGRAPH
    $<$<PLATFORM_ID:Linux>:SK_R32_SHIFT=16>
    $<$<PLATFORM_ID:Darwin>:SK_ASSUME_GL=1>
    $<$<PLATFORM_ID:Darwin>:SK_ENABLE_API_AVAILABLE>
    )
target_link_libraries(Skia::Skia INTERFACE
    ${SKIA_STATIC_LIBS}
    ${HARFBUZZ_LIBRARY}
    ICU::uc
    Threads::Threads
    ${CMAKE_DL_LIBS}
    )
if (APPLE)
    target_link_libraries(Skia::Skia INTERFACE
        "-framework CoreFoundation" "-framework CoreGraphics" "-framework CoreText" "-framework CoreServices" "-framework AppKit")
else ()
    target_link_libraries(Skia::Skia INTERFACE Freetype::Freetype $<$<TARGET_EXISTS:Fontconfig::Fontconfig>:Fontconfig::Fontconfig>)
endif ()
add_dependencies(Skia::Skia skia-build)
