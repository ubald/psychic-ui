if (NOT IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../external/yoga")
  message(FATAL_ERROR "PsychicUI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../external/yoga" "${CMAKE_CURRENT_BINARY_DIR}/build")

file(GLOB YOGA_SOURCES_ ${CMAKE_CURRENT_LIST_DIR}/../external/yoga/yoga/*.c)
set(YOGA_SOURCES "${YOGA_SOURCES_}" PARENT_SCOPE)
set(YOGA_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../external/yoga PARENT_SCOPE)
