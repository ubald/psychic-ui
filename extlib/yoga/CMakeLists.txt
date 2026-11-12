if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../external/yoga/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

# Yoga's own CMakeLists doesn't build anything useful for us, so build its sources directly
file(GLOB YOGA_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../external/yoga/yoga/*.cpp)

add_library(yogacore STATIC ${YOGA_SOURCES})
target_include_directories(yogacore PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../external/yoga)
