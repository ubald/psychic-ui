if (NOT IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../external/rxcpp")
  message(FATAL_ERROR "PsychicUI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

# There is no need building it
# add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../external/rxcpp" "${CMAKE_CURRENT_BINARY_DIR}/build")

set(RXCPP_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../external/rxcpp/Rx/v2/src PARENT_SCOPE)