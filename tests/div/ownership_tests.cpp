// Known bugs from review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

// M7: Div::insertChild didn't detach the child from its old parent
TEST_CASE("M7: adding a child that has a parent moves it", "[div][children]") {
    auto             window = std::make_shared<TestWindow>();
    const FakeSystemWindow sys{window, 800, 600};
    auto            &parent1 = window->appContainer().add<Div>();
    auto            &parent2 = window->appContainer().add<Div>();
    auto             child   = std::make_shared<Div>();
    parent1.add(child);
    parent2.add(child);
    CHECK(parent1.childCount() == 0);
    CHECK(parent2.childCount() == 1);
    CHECK(child->parent() == &parent2);
}

TEST_CASE("A child added again to its parent moves among its siblings", "[div][children]") {
    auto  parent = std::make_shared<Div>();
    auto  a      = std::make_shared<Div>();
    auto  b      = std::make_shared<Div>();
    auto  child  = std::make_shared<Div>();
    parent->add(a);
    parent->add(b);
    parent->add(child);
    parent->add(a); // Last
    CHECK(parent->childIndex(a) == 2);
    parent->add(0, child); // First
    CHECK(parent->childIndex(child) == 0);
    CHECK(parent->childIndex(b) == 1);
    CHECK(parent->childCount() == 3);
    CHECK(a->parent() == parent.get());
    CHECK_THROWS_AS(parent->add(3, b), std::out_of_range); // Among the other two
    CHECK(b->parent() == parent.get());
}

// M12: Div and Observer were polymorphic without a virtual destructor; only make_shared<Concrete> kept deleting
// through a base pointer safe
TEST_CASE("M12: Div has a virtual destructor", "[div]") {
    CHECK(std::has_virtual_destructor_v<Div>);
}
