# Built by tests/CMakeLists.txt with try_compile, as a project of its own so the
# installed Catch2's imported targets can't clash with a download if this fails.
# Without an installed Catch2 it still configures (an error here would stop the
# outer configure) and fails to build instead, with a marker the caller matches.
cmake_minimum_required(VERSION 3.24...4.4)
project(catch2-check LANGUAGES CXX)

find_package(Catch2 3 CONFIG)
if (Catch2_FOUND)
    add_executable(catch2-check main.cpp)
    target_link_libraries(catch2-check PRIVATE Catch2::Catch2WithMain)
else ()
    add_executable(catch2-check not_found.cpp)
endif ()
target_compile_features(catch2-check PRIVATE cxx_std_23)
