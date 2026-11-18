// Known bugs from review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/List.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

// C1: the key goes to the deepest keyPressed subscriber and stops there, whether it used the key or not
TEST_CASE("C1: a key a focused Button doesn't use reaches the app's shortcut", "[keyboard][focus][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    auto            &app       = window->appContainer();
    int              shortcuts = 0;
    app.onKeyPress([&](Key key, Mod mod) {
        if (key == Key::S && mod.ctrl) {
            ++shortcuts;
        }
    });
    auto &button = app.add<Button>("Save");
    sys.layout();
    window->requestFocus(&button);
    window->handleKeyPress(Key::S, Mod{.ctrl = true});
    CHECK(shortcuts == 1);
}

TEST_CASE("C1: a key a focused list row doesn't use reaches the app's shortcut", "[keyboard][focus][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    int              shortcuts = 0;
    window->appContainer().onKeyPress([&](Key key, Mod) {
        if (key == Key::DELETE) {
            ++shortcuts;
        }
    });
    auto &list = window->appContainer().add<List>();
    auto &row  = list.row("One");
    sys.layout();
    window->requestFocus(&row, true);
    window->handleKeyPress(Key::DELETE, Mod{});
    CHECK(shortcuts == 1);
}
