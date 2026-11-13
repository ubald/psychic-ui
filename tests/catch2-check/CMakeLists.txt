# Built by tests/CMakeLists.txt with try_compile, as a project of its own so the
# installed Catch2's imported targets can't clash with a download if this fails
cmake_minimum_required(VERSION 3.24...4.4)
project(catch2-check LANGUAGES CXX)

find_package(Catch2 3 CONFIG)
if (NOT Catch2_FOUND)
    message(FATAL_ERROR "PSYCHIC_UI_CATCH2_NOT_FOUND")
endif ()

add_executable(catch2-check main.cpp)
target_compile_features(catch2-check PRIVATE cxx_std_23)
target_link_libraries(catch2-check PRIVATE Catch2::Catch2WithMain)
