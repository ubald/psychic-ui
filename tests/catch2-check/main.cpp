// Links only if the installed Catch2 library was built with std::string_view
// support (C++17 or newer), which the tests' C++23 build of its headers expects
#include <catch2/catch_test_macros.hpp>

#include <string_view>

TEST_CASE("string_view stringifies") {
    CHECK(Catch::Detail::stringify(std::string_view("psychic")) == "\"psychic\"");
}
