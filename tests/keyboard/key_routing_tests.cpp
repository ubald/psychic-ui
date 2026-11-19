// A key bubbles up the focus path until a handler uses it, up to the window (review-2026-11.md, C1)
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

TEST_CASE("C1: a key a focused Button doesn't use reaches the app's shortcut", "[keyboard][focus]") {
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

TEST_CASE("C1: a key a focused list row doesn't use reaches the app's shortcut", "[keyboard][focus]") {
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

namespace {
    /**
     * A window counting the keys that reach its own handler, as an app's shortcuts would
     */
    struct Shortcuts {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            sys{window, 800, 600};
        int                         keys{0};
        int                         typed{0};

        Shortcuts() {
            window->onKeyPress([this] { ++keys; });
            window->onType([this] { ++typed; });
        }
    };
}

TEST_CASE("A key reaches the window's handler with nothing focused", "[keyboard][focus]") {
    Shortcuts shortcuts{};
    shortcuts.sys.layout();
    REQUIRE(shortcuts.window->focusedDiv() == nullptr);
    shortcuts.window->handleKeyPress(Key::I, Mod{.ctrl = true});
    CHECK(shortcuts.keys == 1);
}

TEST_CASE("A key bubbles up the focus path until a handler uses it", "[keyboard][focus]") {
    Shortcuts shortcuts{};
    auto     &panel = shortcuts.window->appContainer().add<Div>();
    auto     &inner = panel.add<Div>();
    int       panelKeys = 0;
    int       innerKeys = 0;
    panel.onKeyPress([&](Key key, Mod) {
        ++panelKeys;
        return key == Key::P;
    });
    inner.onKeyPress([&] { ++innerKeys; }); // Returns nothing: doesn't use it
    shortcuts.sys.layout();
    shortcuts.window->requestFocus(&inner);

    CHECK(shortcuts.window->handleKeyPress(Key::P, Mod{}));
    CHECK(innerKeys == 1);
    CHECK(panelKeys == 1);
    CHECK(shortcuts.keys == 0);

    shortcuts.window->handleKeyPress(Key::Q, Mod{});
    CHECK(innerKeys == 2);
    CHECK(panelKeys == 2);
    CHECK(shortcuts.keys == 1);
}

TEST_CASE("A key a focused Button uses doesn't reach the window", "[keyboard][focus]") {
    Shortcuts shortcuts{};
    int       clicks = 0;
    auto     &button = shortcuts.window->appContainer().add<Button>("Play");
    button.onClick([&] { ++clicks; });
    shortcuts.sys.layout();
    shortcuts.window->requestFocus(&button, true);
    shortcuts.window->handleKeyPress(Key::SPACE, Mod{});
    CHECK(clicks == 1);
    CHECK(shortcuts.keys == 0);
}

TEST_CASE("A text field keeps the keys it types and lets the shortcuts through", "[keyboard][focus]") {
    Shortcuts shortcuts{};
    auto     &field = shortcuts.window->appContainer().add<TextInput>();
    shortcuts.sys.layout();
    shortcuts.window->requestFocus(&field, true);

    // A bare letter and its character, Space, Backspace with nothing to erase, Ctrl+A
    shortcuts.window->handleKeyPress(Key::S, Mod{});
    shortcuts.window->handleType("s");
    shortcuts.window->handleKeyPress(Key::S, Mod{.shift = true});
    shortcuts.window->handleType("S");
    shortcuts.window->handleKeyPress(Key::SPACE, Mod{});
    shortcuts.window->handleType(" ");
    shortcuts.window->handleKeyPress(Key::HOME, Mod{});
    shortcuts.window->handleKeyPress(Key::BACKSPACE, Mod{});
    shortcuts.window->handleKeyPress(Key::A, Mod{.ctrl = true});
    CHECK(field.text() == "sS ");
    CHECK(shortcuts.keys == 0);
    CHECK(shortcuts.typed == 0);

    // Shortcuts it has no use for, and the function keys
    shortcuts.window->handleKeyPress(Key::I, Mod{.ctrl = true});
    shortcuts.window->handleKeyPress(Key::LEFT, Mod{.alt = true});
    shortcuts.window->handleKeyPress(Key::F5, Mod{});
    CHECK(shortcuts.keys == 3);
    CHECK(field.text() == "sS ");
}

TEST_CASE("A disabled text field keeps no key", "[keyboard][focus]") {
    Shortcuts shortcuts{};
    auto     &panel = shortcuts.window->appContainer().add<Div>();
    auto     &field = panel.add<TextInput>();
    shortcuts.sys.layout();
    shortcuts.window->requestFocus(&field, true);
    field.enabled(false);
    shortcuts.window->handleKeyPress(Key::S, Mod{});
    shortcuts.window->handleType("s");
    CHECK(shortcuts.keys == 1);
    CHECK(shortcuts.typed == 1);
}

TEST_CASE("Escape closes the topmost popup before the window's handler sees it", "[keyboard][focus][popup]") {
    Shortcuts shortcuts{};
    auto     &button = shortcuts.window->appContainer().add<Button>("Open");
    shortcuts.sys.layout();
    shortcuts.window->openMenu({Menu::item("One", [] {})}, button);
    shortcuts.sys.layout();
    REQUIRE(shortcuts.window->overlay().popups().size() == 1);
    CHECK(shortcuts.window->handleKeyPress(Key::ESCAPE, Mod{}));
    shortcuts.sys.layout();
    CHECK(shortcuts.window->overlay().popups().empty());
    CHECK(shortcuts.keys == 0);

    // With no popup left, Escape is the app's
    shortcuts.window->handleKeyPress(Key::ESCAPE, Mod{});
    CHECK(shortcuts.keys == 1);
}

TEST_CASE("A key doesn't leave a popup that traps the focus", "[keyboard][focus][popup]") {
    Shortcuts shortcuts{};
    auto     &button = shortcuts.window->appContainer().add<Button>("Open");
    shortcuts.sys.layout();
    const Menu &menu = shortcuts.window->openMenu({Menu::item("One", [] {})}, button);
    shortcuts.sys.layout();
    REQUIRE(shortcuts.window->focusedDiv() == &menu);
    shortcuts.window->handleKeyPress(Key::S, Mod{.ctrl = true});
    CHECK(shortcuts.keys == 0);
}
