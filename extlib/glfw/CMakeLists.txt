if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../external/glfw/.git")
  message(FATAL_ERROR "Psychic UI dependency missing! Use \"git submodule update --init --recursive\" to download dependencies.")
endif()

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
set(GLFW_INSTALL OFF CACHE BOOL " " FORCE)
# Static, without overriding BUILD_SHARED_LIBS for the rest of the project
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../external/glfw" "${CMAKE_CURRENT_BINARY_DIR}/build")

