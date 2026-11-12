if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../external/yoga/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

# Only Yoga's library directory: its top-level CMakeLists also builds its tests (and fetches GoogleTest)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../external/yoga/yoga ${CMAKE_CURRENT_BINARY_DIR}/yogacore)
