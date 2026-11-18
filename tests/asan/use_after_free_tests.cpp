// Use-after-free and crash paths from review-2026-11.md (section Memory), hidden: they need AddressSanitizer
// to show anything, and without it they may pass by luck or crash the whole run. Run them with the asan preset:
//   cmake --preset asan && cmake --build --preset asan -j"$(nproc)" --target psychic-ui-tests
//   ./build/asan/tests/psychic-ui-tests "[asan]"
// Each one aborts today; a test that passes there has had its bug fixed (tick it off in the checklist). A div
// that should be freed is allocated with shared_ptr<T>(new T) where make_shared's single block would keep its
// memory readable after the destructor and hide the bug.
#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkSurface.h"
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/ColorSwatches.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/SearchField.hpp>
#include <psychic-ui/components/Sidebar.hpp>
#include <psychic-ui/components/TabContainer.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/Tree.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    struct F {
        std::shared_ptr<TestWindow>       window{std::make_shared<TestWindow>()};
        std::unique_ptr<FakeSystemWindow> sys{std::make_unique<FakeSystemWindow>(window, 800, 600)};

        void layout() { sys->layout(); }
        static std::pair<float, float> center(const Div &div) {
            int x = 0, y = 0;
            div.localToGlobal(x, y, div.getWidth() / 2, div.getHeight() / 2);
            return {float(x), float(y)};
        }
        void press(float x, float y, MouseButton b = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = x, .y = y, .button = b, .down = true});
            layout();
        }
        void release(float x, float y, MouseButton b = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = x, .y = y, .button = b, .down = false});
            layout();
        }
        void click(const Div &d) {
            auto [x, y] = center(d);
            press(x, y);
            release(x, y);
        }
        void key(Key k, Mod m = Mod{}) { window->handleKeyPress(k, m); layout(); }
    };
}

// M1: the press's keepAlive ends, and _focusPath still holds the item the blur handler detached
TEST_CASE("M1: a blur handler detaches the div the press focuses", "[.][asan]") {
    F     f;
    auto &field = f.window->appContainer().add<Div>().size(100, 20).focusEnabled(true);
    auto &list  = f.window->appContainer().add<Div>().size(200, 200);
    list.add<Div>().size(200, 50);
    f.layout();
    f.window->requestFocus(&field);
    field.onBlur([&] { list.removeAll(); });
    f.press(50, 40); // the item: field is 0..20, list 20..220
    f.release(50, 40);
    f.key(Key::A); // walks _focusPath
}

// M2: requestFocus from a focused handler frees the path vector being iterated
TEST_CASE("M2: a focused handler on an ancestor redirects the focus", "[.][asan]") {
    F     f;
    auto &group = f.window->appContainer().add<Div>();
    auto &inner = group.add<Div>().focusEnabled(true);
    auto &other = f.window->appContainer().add<Div>().focusEnabled(true);
    f.layout();
    bool once = false;
    group.onFocus([&] {
        if (!once) {
            once = true;
            f.window->requestFocus(&other);
        }
    });
    f.window->requestFocus(&inner); // the outer loop goes on to `inner` over the freed vector
    CHECK(f.window->focusedDiv() == &other);
}

// M2: an ancestor's blurred handler that moves the focus recurses until the stack overflows
TEST_CASE("M2: a blurred handler on an ancestor moves the focus", "[.][asan]") {
    F     f;
    auto &group = f.window->appContainer().add<Div>().size(200, 200);
    auto &a     = group.add<Div>().size(50, 50).focusEnabled(true);
    auto &b     = f.window->appContainer().add<Div>().size(50, 50).focusEnabled(true);
    auto &c     = f.window->appContainer().add<Div>().size(50, 50).focusEnabled(true);
    f.layout();
    f.window->requestFocus(&a);
    group.onBlur([&] { f.window->requestFocus(&c); });
    f.window->requestFocus(&b);
    CHECK(f.window->focusedDiv() == &c);
}

// M5: Menu.cpp's check branch runs the item's callback in place, and closing the menu destroys it while it runs
TEST_CASE("M5: a check item's callback closes the menu", "[.][asan]") {
    F           f;
    std::string payload(200, 'x');
    std::size_t seen = 0;
    Window     *w    = f.window.get();
    w->openMenu({Menu::check("Toggle", false, [w, payload, &seen](bool) {
                    w->closeMenu();
                    seen = payload.size(); // a capture of the destroyed std::function
                })},
                10, 10);
    f.layout();
    f.key(Key::DOWN);
    f.key(Key::ENTER);
    CHECK(seen == 200);
}

// M6: ~Div doesn't clear its children's _parent
TEST_CASE("M6: a child outlives its parent", "[.][asan]") {
    F    f;
    auto child = std::make_shared<Div>();
    {
        auto parent = std::shared_ptr<Div>(new Div());
        parent->add(child);
    }
    CHECK(child->window() == nullptr); // walks the dangling _parent
    f.window->appContainer().add(child);
}

// M6: a part handed out as a shared_ptr outlives its component, whose `this` its slots capture
TEST_CASE("M6: a SearchField's input outlives the field", "[.][asan]") {
    F                          f;
    std::shared_ptr<TextInput> input;
    std::weak_ptr<SearchField> gone;
    {
        auto  field = std::shared_ptr<SearchField>(new SearchField(""));
        auto &added = f.window->appContainer().add(field);
        f.layout();
        input = added.input();
        gone  = field;
        f.window->appContainer().remove(added);
    }
    REQUIRE(gone.expired());
    f.window->appContainer().add(input);
    f.layout();
    f.window->requestFocus(input.get(), true);
    f.window->handleType(icu::UnicodeString(static_cast<UChar32>('a'))); // TextInput's changed -> the dead field
    CHECK(input->text() == "a");
}

// M8: removed() fires only on the moved TreeNode, so the first tree keeps the row as its selection, and reaches it
// after it is freed
TEST_CASE("M8: a selected row moved to another tree, then freed", "[.][asan]") {
    F     f;
    auto &tree1 = f.window->appContainer().add<Tree>();
    auto &tree2 = f.window->appContainer().add<Tree>();
    auto  keep  = std::shared_ptr<TreeNode>(new TreeNode("a"));
    tree1.add(keep);
    tree1.node("b");
    f.layout();
    tree1.select(0);
    keep->parent()->remove(*keep);
    tree2.add(keep);
    f.layout();
    tree2.removeAll();
    keep.reset();    // frees the row tree1 still holds as its selection
    tree1.select(1); // deselects the freed row
}

// M9: Tabs::_selected is null until something is selected, and the first render dereferences it
TEST_CASE("M9: a TabContainer renders before anything is selected", "[.][asan]") {
    F f;
    f.window->appContainer().add<TabContainer<std::string>>(
        std::vector<std::string>{"a", "b"},
        [](const std::string &s) { return s; },
        [](const std::string &) { return std::make_shared<Div>(); }
    );
    f.layout(); // DataContainer builds its tabs while rendering
}

// M3 and M4, from the review's stress cases: a handler removes what is dispatching the key, and the component or
// its row touches itself after the emit

TEST_CASE("M4: delete rows during selectionChanged", "[.][asan]") {
    F f;
    auto &list = f.window->appContainer().add<List>();
    auto &a    = list.row("a");
    auto &b    = list.row("b");
    auto &c    = list.row("c");
    (void) c;
    f.layout();
    list.onSelectionChange([&list](ListRow *row) {
        if (row) list.remove(*row);
    });
    f.click(a);
    f.layout();
    f.click(b);
    f.key(Key::DOWN);
    f.key(Key::UP);
    f.key(Key::HOME);
    f.key(Key::END);
    f.layout();
}

TEST_CASE("M4: clear all rows during selectionChanged, then keys", "[.][asan]") {
    F f;
    auto &list = f.window->appContainer().add<List>();
    for (int i = 0; i < 5; ++i) list.row("r" + std::to_string(i));
    f.layout();
    list.onSelectionChange([&list](ListRow *) { list.removeAll(); });
    f.key(Key::TAB);
    f.key(Key::DOWN);
    f.key(Key::DOWN);
    f.key(Key::SPACE);
    f.layout();
}

TEST_CASE("M3: text input removes itself on change while typing", "[.][asan]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &t      = parent.add<TextInput>("");
    f.layout();
    f.click(t);
    t.onChange([&parent, &t]() { if (t.parent()) parent.remove(t); });
    f.window->handleType("a");
    f.window->handleType("b");
    f.key(Key::BACKSPACE);
    f.layout();
}

TEST_CASE("M4: sidebar removes the item on change (keys)", "[.][asan]") {
    F f;
    auto &sb  = f.window->appContainer().add<Sidebar>();
    auto &sec = sb.section("S");
    sec.item("a");
    sec.item("b");
    sec.item("c");
    f.layout();
    sb.onChange([](SidebarItem *item) { if (item && item->parent()) item->parent()->remove(*item); });
    f.key(Key::TAB);
    f.key(Key::DOWN);
    f.key(Key::ENTER);
    f.key(Key::DOWN);
    f.key(Key::SPACE);
    f.layout();
    f.key(Key::TAB);
}

TEST_CASE("M4: tree removes the selected node on selection change (keys)", "[.][asan]") {
    F f;
    auto &tree = f.window->appContainer().add<Tree>();
    auto &a    = tree.node("a");
    a.node("a1");
    a.node("a2");
    tree.node("b");
    a.expanded(true);
    f.layout();
    tree.onSelectionChange([](TreeNode *n) { if (n && n->parent()) n->parent()->remove(*n); });
    f.key(Key::TAB);
    f.key(Key::DOWN);
    f.key(Key::DOWN);
    f.key(Key::RIGHT);
    f.key(Key::LEFT);
    f.layout();
}

TEST_CASE("M3: list removes itself on selection change (keys)", "[.][asan]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &list   = parent.add<List>();
    list.row("a");
    list.row("b");
    f.layout();
    list.onSelectionChange([&parent, &list](ListRow *) { if (list.parent()) parent.remove(list); });
    f.key(Key::TAB);
    f.key(Key::DOWN);
    f.layout();
}

namespace {
    /**
     * A component that removes itself from its parent when it changes, then the keys that change it
     */
    template<class T, class... Args>
    void removeItselfOnChange(const std::vector<Key> &keys, Args &&...args) {
        F f;
        auto &parent = f.window->appContainer().add<Div>();
        auto &w      = parent.add<T>(std::forward<Args>(args)...);
        f.layout();
        w.onChange([&parent, &w]() {
            if (w.parent()) {
                parent.remove(w);
            }
        });
        f.key(Key::TAB);
        for (const Key k: keys) {
            f.key(k);
        }
        f.layout();
        f.key(Key::TAB);
    }
}

TEST_CASE("M3: color swatches remove themselves on change", "[.][asan]") {
    removeItselfOnChange<ColorSwatches>({Key::RIGHT, Key::SPACE, Key::ENTER});
}

TEST_CASE("M3: range removes itself on change", "[.][asan]") {
    removeItselfOnChange<Range<int>>({Key::RIGHT, Key::RIGHT, Key::HOME}, 0, 10, 5, nullptr);
}
