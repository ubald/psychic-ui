# Configures, builds and runs the consumer project (the test psychic-ui-consumer, tests/CMakeLists.txt):
#   cmake -DSOURCE_DIR=<consumer> -DBINARY_DIR=<build> -DGENERATOR=<generator> -DPSYCHIC_UI_SOURCE_DIR=<checkout>
#         -DPSYCHIC_UI_SKIA_DIR=<prebuilt Skia> -DC_COMPILER=<cc> -DCXX_COMPILER=<c++> -DPARALLEL=<jobs>
#         -DRUN_DIR=<working directory for the app> -P run.cmake
cmake_minimum_required(VERSION 3.24)

foreach (_var SOURCE_DIR BINARY_DIR GENERATOR PSYCHIC_UI_SOURCE_DIR PSYCHIC_UI_SKIA_DIR C_COMPILER CXX_COMPILER
        PARALLEL RUN_DIR)
    if (NOT DEFINED ${_var})
        message(FATAL_ERROR "run.cmake needs -D${_var}=...")
    endif ()
endforeach ()

function(run step)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE output
        WORKING_DIRECTORY "${working_directory}")
    if (NOT result EQUAL 0)
        message(FATAL_ERROR "The consumer's ${step} failed (${result}):\n${output}")
    endif ()
endfunction()

# The options' defaults are what is tested, so nothing stays cached from an earlier run: -U before the -Ds
set(working_directory "${BINARY_DIR}")
file(MAKE_DIRECTORY "${BINARY_DIR}")
run(configure ${CMAKE_COMMAND} -S "${SOURCE_DIR}" -B "${BINARY_DIR}" -G "${GENERATOR}"
    "-UPSYCHIC_UI_*" -UBUILD_SHARED_LIBS
    "-DPSYCHIC_UI_SOURCE_DIR=${PSYCHIC_UI_SOURCE_DIR}"
    "-DPSYCHIC_UI_SKIA_DIR=${PSYCHIC_UI_SKIA_DIR}"
    "-DCMAKE_C_COMPILER=${C_COMPILER}"
    "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}"
    -DCMAKE_BUILD_TYPE=Debug)
run(build ${CMAKE_COMMAND} --build "${BINARY_DIR}" --parallel ${PARALLEL})

# From a directory with no res/: the app must find its resources next to its executable
set(working_directory "${RUN_DIR}")
run(app "${BINARY_DIR}/consumer-app")
