// Known bugs from review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <memory>
#include <type_traits>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

// M7: Div::insertChild doesn't detach the child from its old parent
TEST_CASE("M7: adding a child that has a parent moves it", "[div][children][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    const FakeSystemWindow sys{window, 800, 600};
    auto            &p1    = window->appContainer().add<Div>();
    auto            &p2    = window->appContainer().add<Div>();
    auto             child = std::make_shared<Div>();
    p1.add(child);
    p2.add(child);
    CHECK(p1.childCount() == 0);
    CHECK(p2.childCount() == 1);
    CHECK(child->parent() == &p2);
}

// M12: Div and Observer were polymorphic without a virtual destructor; only make_shared<Concrete> kept deleting
// through a base pointer safe
TEST_CASE("M12: Div has a virtual destructor", "[div]") {
    CHECK(std::has_virtual_destructor_v<Div>);
}
