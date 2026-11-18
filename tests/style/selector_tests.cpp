// Known bugs from docs/review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <stdexcept>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/style/StyleSelector.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    int weight(const std::string &selector) {
        auto parsed = StyleSelector::fromSelector(selector);
        REQUIRE(parsed);
        return parsed->weight();
    }
}

// S1: StyleSelector::weight() adds 15 per id and 10 per tag and class, so classes and tags add up past an id
TEST_CASE("S1: an id outweighs any number of classes, as in CSS", "[style][selector][!shouldfail]") {
    CHECK(weight("#save") > weight(".a.b"));
    CHECK(weight("#save") > weight("div .a"));
}

TEST_CASE("S1: a pseudo-class weighs as much as a class, more than a tag", "[style][selector][!shouldfail]") {
    // (0,2,0) against (0,1,1)
    CHECK(weight(".a:hover") > weight("div .a"));
}

TEST_CASE("S1: pseudo-classes weigh the same, so the one declared last wins", "[style][selector][!shouldfail]") {
    CHECK(weight("button:disabled") == weight("button:hover"));
    CHECK(weight("button:focus") == weight("button:active"));
}

// S2: :focus-visible was missing from weight()'s switch
TEST_CASE("S2: :focus-visible weighs like any pseudo-class", "[style][selector]") {
    CHECK(weight("button:focus-visible") > weight("button"));
    CHECK(weight("button:focus-visible") == weight("button:focus"));
}

// S3: an unknown pseudo-class is dropped, so the rule matches what the selector without it matches
TEST_CASE("S3: an unknown pseudo-class is an invalid selector", "[style][selector][!shouldfail]") {
    auto window = std::make_shared<TestWindow>();
    CHECK_THROWS_AS(window->styleManager()->style(".probe:checked"), std::invalid_argument);
    CHECK_THROWS_AS(window->styleManager()->style(".probe:no-such-state"), std::invalid_argument);
}

// S3: :focus-within is the useful one to support, so that a group can show that one of its children has the focus
TEST_CASE("S3: :focus-within matches while a descendant has the focus", "[style][selector][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    auto            &group  = window->appContainer().add<Div>().addClassName("group");
    auto            &button = group.add<Button>("Save");
    window->styleManager()->style(".group:focus-within").paddingLeft(5);
    window->styleManager()->invalidate();
    sys.layout();
    REQUIRE(group.computedStyle().paddingLeft() != Length{5});
    window->requestFocus(&button);
    sys.layout();
    CHECK(group.computedStyle().paddingLeft() == Length{5});
    window->requestFocus(&window->appContainer());
    sys.layout();
    CHECK(group.computedStyle().paddingLeft() != Length{5});
}

// S4: child combinators and selector lists parse as tags
TEST_CASE("S4: combinators and selector lists that aren't supported are invalid selectors", "[style][selector][!shouldfail]") {
    auto window = std::make_shared<TestWindow>();
    CHECK_THROWS_AS(window->styleManager()->style("list>row"), std::invalid_argument);
    CHECK_THROWS_AS(window->styleManager()->style("list > row"), std::invalid_argument);
    CHECK_THROWS_AS(window->styleManager()->style("a, b"), std::invalid_argument);
}

// S5: adding the first child or removing the last doesn't restyle the parent
TEST_CASE("S5: :empty stops matching once a child is added", "[style][selector][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    auto            &box = window->appContainer().add<Div>().addClassName("box");
    window->styleManager()->style(".box:empty").paddingLeft(5);
    window->styleManager()->invalidate();
    sys.layout();
    REQUIRE(box.computedStyle().paddingLeft() == Length{5});
    box.add<Div>();
    sys.layout();
    CHECK(box.computedStyle().paddingLeft() != Length{5});
}
