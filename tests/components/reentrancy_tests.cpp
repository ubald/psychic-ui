// Handlers that remove, close or restyle what is dispatching them, from the review's stress cases
// (review-2026-11.md): clean under AddressSanitizer, kept as regression tests. The cases that found
// bugs are the hidden [asan] tests in tests/asan/use_after_free_tests.cpp.
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/MenuButton.hpp>
#include <psychic-ui/components/Select.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/Toast.hpp> // IWYU pragma: keep: toast({...}), a ToastOptions, which include-cleaner misses
#include <psychic-ui/components/Tree.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/Chip.hpp>
#include <psychic-ui/components/SegmentedControl.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/MenuBar.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/SearchField.hpp>
#include <psychic-ui/skins/float/FloatSkins.hpp> // IWYU pragma: keep: skinSet<FloatSkins>(), which include-cleaner misses
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Popup.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;
using namespace std::chrono_literals;

namespace {
    struct F {
        std::shared_ptr<TestWindow>       window{std::make_shared<TestWindow>()};
        std::unique_ptr<FakeSystemWindow> sys{std::make_unique<FakeSystemWindow>(window, 800, 600)};

        void layout() const { sys->layout(); }
        static std::pair<float, float> center(const Div &div) {
            int x = 0;
            int y = 0;
            div.localToGlobal(x, y, div.getWidth() / 2, div.getHeight() / 2);
            return {float(x), float(y)};
        }
        void press(float x, float y, MouseButton b = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = x, .y = y, .button = b, .down = true});
        }
        void release(float x, float y, MouseButton b = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = x, .y = y, .button = b, .down = false});
        }
        void click(const Div &div) {
            auto [x, y] = center(div);
            press(x, y);
            release(x, y);
            layout();
        }
        void move(float x, float y) { window->handleMouseMove({.x = x, .y = y}, false); }
        void key(Key k, Mod mod = Mod{}) { window->handleKeyPress(k, mod); layout(); }
    };
}

TEST_CASE("Reentrancy: remove a button from its own click", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    fixture.layout();
    b.onClick([&parent, &b]() { parent.remove(b); });
    fixture.click(b);
    fixture.layout();
    CHECK(parent.children().empty());
}

TEST_CASE("Reentrancy: remove a button from its own mousePressed", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    fixture.layout();
    b.onMousePress([&parent, &b]() { parent.remove(b); });
    fixture.click(b);
    fixture.layout();
}

TEST_CASE("Reentrancy: remove a button from its own mouseReleased", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    fixture.layout();
    b.onMouseRelease([&parent, &b]() { parent.remove(b); });
    b.onClick([]() {});
    fixture.click(b);
    fixture.layout();
}

TEST_CASE("Reentrancy: remove the parent of a button from its click", "[reentrancy]") {
    F fixture;
    auto &grand  = fixture.window->appContainer().add<Div>();
    auto &parent = grand.add<Div>();
    auto &b      = parent.add<Button>("x");
    fixture.layout();
    b.onClick([&grand, &parent]() { grand.removeAll(); (void) parent; });
    fixture.click(b);
    fixture.layout();
}

TEST_CASE("Reentrancy: remove focused text input on key press", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &t      = parent.add<TextInput>("hello");
    fixture.layout();
    fixture.click(t);
    t.onKeyPress([&parent, &t]() { parent.remove(t); });
    fixture.key(Key::A);
    fixture.key(Key::TAB);
    fixture.layout();
}

TEST_CASE("Reentrancy: remove focused div on blur", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &a      = parent.add<Button>("a");
    auto &b      = parent.add<Button>("b");
    (void) b;
    fixture.layout();
    fixture.key(Key::TAB);
    a.onBlur([&parent, &a]() { parent.remove(a); });
    fixture.key(Key::TAB);
    fixture.key(Key::TAB);
}

TEST_CASE("Reentrancy: close a popup from its closed handler", "[reentrancy]") {
    const F fixture;
    auto &b = fixture.window->appContainer().add<Button>("a");
    fixture.layout();
    Menu &menu = fixture.window->openMenu({Menu::item("One", []() {})}, b);
    fixture.layout();
    menu.onClose([&menu]() { menu.close(); });
    menu.close();
    fixture.layout();
}

TEST_CASE("Reentrancy: open another menu from a menu's closed handler", "[reentrancy]") {
    F fixture;
    auto &b = fixture.window->appContainer().add<Button>("a");
    fixture.layout();
    Menu &menu = fixture.window->openMenu({Menu::item("One", []() {})}, b);
    fixture.layout();
    Window *w = fixture.window.get();
    menu.onClose([w, &b]() { w->openMenu({Menu::item("Two", []() {})}, b); });
    fixture.key(Key::ESCAPE);
    fixture.layout();
    fixture.key(Key::ESCAPE);
}

TEST_CASE("Reentrancy: a menu item callback removes the anchor and opens a menu", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("a");
    fixture.layout();
    Window *w = fixture.window.get();
    w->openMenu({Menu::item("One", [&parent, &b, w]() {
        parent.remove(b);
        w->openMenu({Menu::item("x", []() {})}, 10, 10);
    })}, b);
    fixture.layout();
    fixture.key(Key::DOWN);
    fixture.key(Key::ENTER);
    fixture.layout();
    fixture.layout();
}

TEST_CASE("Reentrancy: a menu item callback closes the menu itself and the window menu", "[reentrancy]") {
    F fixture;
    auto &b = fixture.window->appContainer().add<Button>("a");
    fixture.layout();
    Window     *w    = fixture.window.get();
    const Menu &menu = w->openMenu({Menu::item("One", [w]() { w->closeMenu(); w->overlay().closeAll<Popup>(); })}, b);
    (void) menu;
    fixture.layout();
    auto *row = menu.rows().at(0); (void) row;
    fixture.click(*row);
    fixture.layout();
}

TEST_CASE("Reentrancy: submenu chosen item closes everything", "[reentrancy]") {
    F fixture;
    auto &b = fixture.window->appContainer().add<Button>("a");
    fixture.layout();
    Window *w = fixture.window.get();
    w->openMenu({Menu::item("Sub", {Menu::item("Leaf", [w]() { w->overlay().closeAll<Popup>(); })})}, b);
    fixture.layout();
    fixture.key(Key::DOWN);
    fixture.key(Key::RIGHT);
    fixture.key(Key::ENTER);
    fixture.layout();
}

TEST_CASE("Reentrancy: destroy window while popups are open", "[reentrancy]") {
    auto fixture = std::make_unique<F>();
    auto &b = fixture->window->appContainer().add<Button>("a").tooltip("tip");
    auto &s = fixture->window->appContainer().add<Select>(std::vector<std::string>{"A", "B"}, 0);
    fixture->layout();
    fixture->window->openMenu({Menu::item("Sub", {Menu::item("Leaf", []() {})})}, b);
    fixture->layout();
    fixture->key(Key::DOWN);
    fixture->key(Key::RIGHT);
    fixture->window->overlay().open<Dialog>("Hi").action("OK", []() {});
    fixture->window->toast({.title = "t"});
    fixture->layout();
    (void) s;
    fixture->sys.reset();
    fixture->window.reset();
    fixture.reset();
}

TEST_CASE("Reentrancy: destroy window (system window first kept) while popups open", "[reentrancy]") {
    auto window = std::make_shared<TestWindow>();
    auto sys    = std::make_unique<FakeSystemWindow>(window, 800, 600);
    auto &b     = window->appContainer().add<Button>("a");
    sys->layout();
    window->openMenu({Menu::item("One", []() {})}, b);
    window->toast({.title = "t"});
    sys->layout();
    window.reset();          // sys still holds a ref through testWindow
    sys.reset();
}

TEST_CASE("Reentrancy: remove the anchor of an open tooltip", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("a").tooltip("tip");
    fixture.layout();
    auto [x, y] = F::center(b);
    fixture.move(x, y);
    fixture.window->updatePopups(std::chrono::steady_clock::now().time_since_epoch());
    fixture.window->updatePopups(std::chrono::steady_clock::now().time_since_epoch() + 1s);
    fixture.layout();
    parent.remove(b);
    fixture.layout();
    fixture.window->updatePopups(std::chrono::steady_clock::now().time_since_epoch() + 2s);
    fixture.layout();
    fixture.move(x + 1, y);
    fixture.layout();
}

TEST_CASE("Reentrancy: remove a tooltip's div before its delay ends", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("a").tooltip("tip");
    fixture.layout();
    auto [x, y] = F::center(b);
    fixture.move(x, y);
    auto start = std::chrono::steady_clock::now().time_since_epoch();
    fixture.window->updatePopups(start);
    parent.remove(b);
    fixture.window->updatePopups(start + 1s);
    fixture.layout();
}

TEST_CASE("Reentrancy: remove the anchor of an open menu and select", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &s      = parent.add<Select>(std::vector<std::string>{"A", "B"}, 0);
    fixture.layout();
    fixture.click(s);
    fixture.layout();
    parent.remove(s);
    fixture.layout();
    fixture.key(Key::DOWN);
    fixture.key(Key::ENTER);
    fixture.layout();
}

TEST_CASE("Reentrancy: select removes itself on change", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &s      = parent.add<Select>(std::vector<std::string>{"A", "B"}, 0);
    fixture.layout();
    s.onChange([&parent, &s]() { parent.remove(s); });
    fixture.click(s);
    fixture.key(Key::DOWN);
    fixture.key(Key::ENTER);
    fixture.layout();
}

TEST_CASE("Reentrancy: remove a tree node holding the selection / collapse during change", "[reentrancy]") {
    F fixture;
    auto &tree = fixture.window->appContainer().add<Tree>();
    auto &root = tree.node("root");
    auto &kid  = root.node("kid");
    kid.node("grandkid");
    root.expanded(true);
    kid.expanded(true);
    fixture.layout();
    tree.onExpansionChange([&kid](TreeNode *node, bool) {
        if (node == &kid && kid.parent()) {
            kid.parent()->remove(kid);
        }
    });
    tree.onSelectionChange([](TreeNode *) {});
    fixture.key(Key::TAB);
    fixture.key(Key::DOWN);
    fixture.key(Key::DOWN);
    fixture.key(Key::LEFT);
    fixture.key(Key::LEFT);
    fixture.key(Key::ENTER);
    fixture.layout();
}

TEST_CASE("Reentrancy: sort a data grid from its selection change and remove rows", "[reentrancy]") {
    F fixture;
    auto &grid = fixture.window->appContainer().add<DataGrid>();
    grid.columns({{"Name"}, {.title = "Size", .width = 80}});
    for (int i = 0; i < 6; ++i) {
        grid.row({"n" + std::to_string(i), std::to_string(10 - i)});
    }
    fixture.layout();
    grid.onSelectionChange([&grid](DataGridRow *) { grid.sort(1, SortOrder::Ascending); });
    fixture.key(Key::TAB);
    fixture.key(Key::DOWN);
    fixture.key(Key::DOWN);
    fixture.layout();
}

TEST_CASE("Reentrancy: change theme and skin set inside styleUpdated / mid event", "[reentrancy]") {
    F fixture;
    auto &b = fixture.window->appContainer().add<Button>("a");
    auto &t = fixture.window->appContainer().add<TextInput>("hello");
    auto &toggle = fixture.window->appContainer().add<Switch>();
    (void) toggle;
    fixture.layout();
    Window *w = fixture.window.get();
    int     n = 0;
    b.onClick([w, &n]() {
        if (n++ % 2) {
            w->skinSet<FloatSkins>();
        } else {
            w->skinSet<PivotalSkins>();
        }
        w->theme(themes::pivotalLight);
    });
    fixture.click(b);
    fixture.click(b);
    fixture.click(t);
    t.onKeyPress([w]() { w->skinSet<FloatSkins>(); });
    fixture.key(Key::A);
    fixture.key(Key::BACKSPACE);
    fixture.layout();
}

namespace {
    struct ThemeFlipper : Div {
        Window *w;
        int     n{0};
        explicit ThemeFlipper(Window *w) : w(w) {}
        void styleUpdated() override {
            Div::styleUpdated();
            if (n++ < 3) {
                w->theme(n % 2 ? themes::pivotalLight : themes::pivotalDark);
            }
        }
    };
    struct SkinFlipper : Div {
        Window *w;
        int     n{0};
        explicit SkinFlipper(Window *w) : w(w) {}
        void styleUpdated() override {
            Div::styleUpdated();
            if (n++ < 3) {
                if (n % 2) {
                    w->skinSet<FloatSkins>();
                } else {
                    w->skinSet<PivotalSkins>();
                }
            }
        }
    };
}

TEST_CASE("Reentrancy: theme switched inside styleUpdated", "[reentrancy]") {
    const F fixture;
    fixture.window->appContainer().add<Button>("a");
    fixture.window->appContainer().add<ThemeFlipper>(fixture.window.get());
    fixture.window->appContainer().add<TextInput>("t");
    fixture.layout();
    fixture.layout();
}

TEST_CASE("Reentrancy: skin set switched inside styleUpdated", "[reentrancy]") {
    const F fixture;
    fixture.window->appContainer().add<Button>("a");
    fixture.window->appContainer().add<SkinFlipper>(fixture.window.get());
    fixture.window->appContainer().add<TextInput>("t");
    fixture.window->appContainer().add<Switch>();
    fixture.layout();
    fixture.layout();
}

TEST_CASE("Reentrancy: re-hatch a button's skin from its own press", "[reentrancy]") {
    F fixture;
    auto &b = fixture.window->appContainer().add<Button>("a");
    fixture.layout();
    b.onMousePress([&b]() { b.skin("primary"); });
    b.onClick([&b]() { b.skin(""); });
    fixture.click(b);
    fixture.layout();
    fixture.click(b);
    fixture.layout();
}

TEST_CASE("Reentrancy: toast action removes all toasts and opens a dialog", "[reentrancy]") {
    F fixture;
    Window *w = fixture.window.get();
    auto &t   = w->toast({.title = "t", .actions = {{.label = "Go", .callback = [w]() {
        w->overlay().closeAll<Popup>();
        w->overlay().open<Dialog>("x");
    }}}});
    w->toast({.title = "u"});
    fixture.layout();
    Div *btn = nullptr;
    std::function<void(Div &)> find = [&](Div &div) {
        for (const auto &child: div.children()) {
            if (auto *button = dynamic_cast<Button *>(child.get()); button && !btn) {
                btn = button;
            }
            find(*child);
        }
    };
    find(t);
    REQUIRE(btn);
    fixture.click(*btn);
    fixture.layout();
    fixture.key(Key::ESCAPE);
}

TEST_CASE("Reentrancy: toasts time out while one is being clicked away; more than the limit", "[reentrancy]") {
    const F fixture;
    Window *w = fixture.window.get();
    for (int i = 0; i < 8; ++i) {
        w->toast({.title = "t" + std::to_string(i), .timeout = 1ms});
        fixture.layout();
    }
    auto start = std::chrono::steady_clock::now().time_since_epoch();
    w->updatePopups(start);
    w->updatePopups(start + 1s);
    fixture.layout();
    w->toast({.title = "z"});
    w->updatePopups(start + 2s);
    w->updatePopups(start + 10s);
    fixture.layout();
}

TEST_CASE("Reentrancy: dialog action closes and removes the opener", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("open");
    fixture.layout();
    fixture.key(Key::TAB);
    const Window *w = fixture.window.get();
    auto   &dialog = w->overlay().open<Dialog>("Hi");
    dialog.action("OK", [&parent, &b]() { parent.remove(b); });
    dialog.body().add<TextInput>("x");
    fixture.layout();
    fixture.key(Key::TAB);
    fixture.key(Key::TAB);
    fixture.key(Key::TAB);
    fixture.key(Key::ENTER);
    fixture.layout();
    fixture.key(Key::TAB);
}

TEST_CASE("Reentrancy: chip dismissed removes itself", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &chip   = parent.add<Chip>("x").removable(true);
    fixture.layout();
    chip.onDismiss([&parent, &chip]() { parent.remove(chip); });
    fixture.key(Key::TAB);
    fixture.key(Key::BACKSPACE);
    fixture.layout();
    fixture.key(Key::TAB);
}

TEST_CASE("Reentrancy: search field clears and removes itself on submit", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &s      = parent.add<SearchField>();
    fixture.layout();
    fixture.click(s);
    fixture.window->handleType("abc");
    fixture.layout();
    s.onSubmit([&parent, &s]() { parent.remove(s); });
    fixture.key(Key::ENTER);
    fixture.layout();
    fixture.key(Key::ESCAPE);
}

TEST_CASE("Reentrancy: context menu whose item removes the div with the context menu", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &div    = parent.add<Div>().size(100, 100);
    div.contextMenu({Menu::item("Delete", [&parent, &div]() { parent.remove(div); })});
    fixture.layout();
    auto [x, y] = F::center(div);
    fixture.press(x, y, MouseButton::RIGHT);
    fixture.release(x, y, MouseButton::RIGHT);
    fixture.layout();
    fixture.key(Key::DOWN);
    fixture.key(Key::ENTER);
    fixture.layout();
    fixture.press(x, y, MouseButton::RIGHT);
    fixture.release(x, y, MouseButton::RIGHT);
    fixture.layout();
}

TEST_CASE("Reentrancy: touch long press then remove target before release", "[reentrancy]") {
    const F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x").tooltip("tip");
    fixture.layout();
    auto [x, y] = F::center(b);
    auto start  = std::chrono::steady_clock::now().time_since_epoch();
    fixture.window->handleTouchPress(x, y, start);
    fixture.window->updateGestures(start + 600ms);
    fixture.window->updatePopups(start + 600ms);
    fixture.layout();
    parent.remove(b);
    fixture.window->handleTouchMove(x + 30, y, start + 700ms);
    fixture.window->handleTouchRelease(x + 30, y, start + 800ms);
    fixture.layout();
}

TEST_CASE("Reentrancy: touch pan a scroller and remove it mid-fling", "[reentrancy]") {
    const F fixture;
    auto &parent   = fixture.window->appContainer().add<Div>();
    auto &scroller = parent.add<Div>().size(200, 200).overflow(Overflow::Scroll);
    for (int i = 0; i < 30; ++i) {
        scroller.add<Div>().height(40);
    }
    fixture.layout();
    auto [x, y] = F::center(scroller);
    auto start  = std::chrono::steady_clock::now().time_since_epoch();
    fixture.window->handleTouchPress(x, y, start);
    fixture.window->handleTouchMove(x, y - 20, start + 10ms);
    fixture.window->handleTouchMove(x, y - 60, start + 20ms);
    fixture.window->handleTouchRelease(x, y - 80, start + 30ms);
    fixture.window->updateGestures(start + 40ms);
    parent.remove(scroller);
    fixture.window->updateGestures(start + 60ms);
    fixture.window->updateGestures(start + 200ms);
    fixture.layout();
}

TEST_CASE("Reentrancy: press on a button, remove it, release elsewhere", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    fixture.layout();
    auto [x, y] = F::center(b);
    fixture.press(x, y);
    parent.remove(b);
    fixture.move(x + 50, y + 50);
    fixture.release(x + 50, y + 50);
    fixture.layout();
}

TEST_CASE("Reentrancy: hovered div removed then mouse moves", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    fixture.layout();
    auto [x, y] = F::center(b);
    fixture.move(x, y);
    b.onMouseExit([]() {});
    parent.remove(b);
    fixture.move(x + 1, y);
    fixture.move(500, 500);
    fixture.layout();
}

TEST_CASE("Reentrancy: mouseExited removes the div being exited", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    fixture.layout();
    auto [x, y] = F::center(b);
    fixture.move(x, y);
    b.onMouseExit([&parent, &b]() { parent.remove(b); });
    fixture.move(700, 500);
    fixture.layout();
}

TEST_CASE("Reentrancy: mouseEntered removes siblings on the path", "[reentrancy]") {
    F fixture;
    auto &outer = fixture.window->appContainer().add<Div>();
    auto &inner = outer.add<Div>().size(100, 100);
    auto &b     = inner.add<Button>("x");
    fixture.layout();
    inner.onMouseEnter([&outer]() { outer.removeAll(); });
    auto [x, y] = F::center(b);
    fixture.move(x, y);
    fixture.layout();
}

TEST_CASE("Reentrancy: menu radio callback closes the overlay and switches skin set", "[reentrancy]") {
    F fixture;
    auto &b = fixture.window->appContainer().add<Button>("a");
    fixture.layout();
    Window *w     = fixture.window.get();
    auto    group = std::make_shared<MenuRadioGroup>("0", [w](const std::string &) {
        w->skinSet<FloatSkins>();
        w->overlay().closeAll<Popup>();
    });
    w->openMenu({Menu::radio(group, "A", "0"), Menu::radio(group, "B", "1")}, b);
    fixture.layout();
    fixture.key(Key::DOWN);
    fixture.key(Key::DOWN);
    fixture.key(Key::ENTER);
    fixture.layout();
}

TEST_CASE("Reentrancy: data grid removes the selected row on selection change (keys)", "[reentrancy]") {
    F fixture;
    auto &grid = fixture.window->appContainer().add<DataGrid>();
    grid.columns({{"Name"}});
    for (int i = 0; i < 5; ++i) {
        grid.row({"n" + std::to_string(i)});
    }
    fixture.layout();
    grid.onSelectionChange([](DataGridRow *r) {
        if (r && r->parent()) {
            r->parent()->remove(*r);
        }
    });
    fixture.key(Key::TAB);
    fixture.key(Key::DOWN);
    fixture.key(Key::DOWN);
    fixture.layout();
}

TEST_CASE("Reentrancy: list removes itself on selection change (click)", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    auto &list   = parent.add<List>();
    auto &a      = list.row("a");
    list.row("b");
    fixture.layout();
    list.onSelectionChange([&parent, &list](ListRow *) {
        if (list.parent()) {
            parent.remove(list);
        }
    });
    fixture.click(a);
    fixture.layout();
}

TEST_CASE("Reentrancy: menu bar removed by its own item", "[reentrancy]") {
    F fixture;
    auto &parent = fixture.window->appContainer().add<Div>();
    MenuBar *bar = nullptr;
    bar = &parent.add<MenuBar>(std::vector<std::shared_ptr<MenuItem>>{
        Menu::item("File", {Menu::item("Quit", [&parent, &bar]() { parent.remove(*bar); })}),
        Menu::item("Edit", {Menu::item("Copy", []() {})})});
    fixture.layout();
    const Div *trigger = bar->children().size() > 1 ? bar->children()[1].get() : bar->children()[0].get();
    fixture.click(*trigger);
    fixture.key(Key::DOWN);
    fixture.key(Key::ENTER);
    fixture.layout();
    fixture.layout();
}

TEST_CASE("Reentrancy: window destroyed with focus and hover inside", "[reentrancy]") {
    auto fixture = std::make_unique<F>();
    auto &t = fixture->window->appContainer().add<TextInput>("x");
    auto &b = fixture->window->appContainer().add<Button>("b").tooltip("tip");
    fixture->layout();
    fixture->click(t);
    auto [x, y] = F::center(b);
    fixture->move(x, y);
    fixture->press(x, y);
    fixture.reset();
}

namespace {
    /**
     * A component that removes itself from its parent when it changes, then the keys that change it
     */
    template<class T, class... Args>
    void removeItselfOnChange(const std::vector<Key> &keys, Args &&...args) {
        F fixture;
        auto &parent = fixture.window->appContainer().add<Div>();
        auto &w      = parent.add<T>(std::forward<Args>(args)...);
        fixture.layout();
        w.onChange([&parent, &w]() {
            if (w.parent()) {
                parent.remove(w);
            }
        });
        fixture.key(Key::TAB);
        for (const Key k: keys) {
            fixture.key(k);
        }
        fixture.layout();
        fixture.key(Key::TAB);
    }
}

TEST_CASE("Reentrancy: switch removes itself on change", "[reentrancy]") {
    removeItselfOnChange<Switch>({Key::SPACE, Key::ENTER}, std::string{"s"});
}

TEST_CASE("Reentrancy: checkbox removes itself on change", "[reentrancy]") {
    removeItselfOnChange<CheckBox>({Key::SPACE, Key::SPACE}, std::string{"c"});
}

TEST_CASE("Reentrancy: segmented control removes itself on change", "[reentrancy]") {
    removeItselfOnChange<SegmentedControl>({Key::RIGHT, Key::RIGHT, Key::END}, std::vector<Segment>{{"A"}, {"B"}, {"C"}});
}

TEST_CASE("Reentrancy: number stepper removes itself on change", "[reentrancy]") {
    removeItselfOnChange<NumberStepper>({Key::UP, Key::UP, Key::DOWN}, 1.0);
}
