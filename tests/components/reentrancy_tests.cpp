// Handlers that remove, close or restyle what is dispatching them, from the review's stress cases
// (review-2026-11.md): clean under AddressSanitizer, kept as regression tests. The cases that found
// bugs are the hidden [asan] tests in tests/asan/use_after_free_tests.cpp.
#include <chrono>
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
#include <psychic-ui/components/Toast.hpp>
#include <psychic-ui/components/Tooltip.hpp>
#include <psychic-ui/components/Tree.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/Chip.hpp>
#include <psychic-ui/components/Sidebar.hpp>
#include <psychic-ui/components/SegmentedControl.hpp>
#include <psychic-ui/components/ColorSwatches.hpp>
#include <psychic-ui/components/NumberStepper.hpp>
#include <psychic-ui/components/MenuBar.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/SearchField.hpp>
#include <psychic-ui/skins/float/FloatSkins.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include <psychic-ui/themes/float.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;
using namespace std::chrono_literals;

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
        }
        void release(float x, float y, MouseButton b = MouseButton::LEFT) {
            window->handleMouseButtonChange({.x = x, .y = y, .button = b, .down = false});
        }
        void click(const Div &d) {
            auto [x, y] = center(d);
            press(x, y);
            release(x, y);
            layout();
        }
        void move(float x, float y) { window->handleMouseMove({.x = x, .y = y}, false); }
        void key(Key k, Mod m = Mod{}) { window->handleKeyPress(k, m); layout(); }
    };
}

TEST_CASE("Reentrancy: remove a button from its own click", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    f.layout();
    b.onClick([&parent, &b]() { parent.remove(b); });
    f.click(b);
    f.layout();
    CHECK(parent.children().empty());
}

TEST_CASE("Reentrancy: remove a button from its own mousePressed", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    f.layout();
    b.onMousePress([&parent, &b]() { parent.remove(b); });
    f.click(b);
    f.layout();
}

TEST_CASE("Reentrancy: remove a button from its own mouseReleased", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    f.layout();
    b.onMouseRelease([&parent, &b]() { parent.remove(b); });
    b.onClick([]() {});
    f.click(b);
    f.layout();
}

TEST_CASE("Reentrancy: remove the parent of a button from its click", "[reentrancy]") {
    F f;
    auto &grand  = f.window->appContainer().add<Div>();
    auto &parent = grand.add<Div>();
    auto &b      = parent.add<Button>("x");
    f.layout();
    b.onClick([&grand, &parent]() { grand.removeAll(); (void) parent; });
    f.click(b);
    f.layout();
}

TEST_CASE("Reentrancy: remove focused text input on key press", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &t      = parent.add<TextInput>("hello");
    f.layout();
    f.click(t);
    t.onKeyPress([&parent, &t]() { parent.remove(t); });
    f.key(Key::A);
    f.key(Key::TAB);
    f.layout();
}

TEST_CASE("Reentrancy: remove focused div on blur", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &a      = parent.add<Button>("a");
    auto &b      = parent.add<Button>("b");
    (void) b;
    f.layout();
    f.key(Key::TAB);
    a.onBlur([&parent, &a]() { parent.remove(a); });
    f.key(Key::TAB);
    f.key(Key::TAB);
}

TEST_CASE("Reentrancy: close a popup from its closed handler", "[reentrancy]") {
    F f;
    auto &b = f.window->appContainer().add<Button>("a");
    f.layout();
    Menu &m = f.window->openMenu({Menu::item("One", []() {})}, b);
    f.layout();
    m.onClose([&m]() { m.close(); });
    m.close();
    f.layout();
}

TEST_CASE("Reentrancy: open another menu from a menu's closed handler", "[reentrancy]") {
    F f;
    auto &b = f.window->appContainer().add<Button>("a");
    f.layout();
    Menu &m = f.window->openMenu({Menu::item("One", []() {})}, b);
    f.layout();
    Window *w = f.window.get();
    m.onClose([w, &b]() { w->openMenu({Menu::item("Two", []() {})}, b); });
    f.key(Key::ESCAPE);
    f.layout();
    f.key(Key::ESCAPE);
}

TEST_CASE("Reentrancy: a menu item callback removes the anchor and opens a menu", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("a");
    f.layout();
    Window *w = f.window.get();
    w->openMenu({Menu::item("One", [&parent, &b, w]() {
        parent.remove(b);
        w->openMenu({Menu::item("x", []() {})}, 10, 10);
    })}, b);
    f.layout();
    f.key(Key::DOWN);
    f.key(Key::ENTER);
    f.layout();
    f.layout();
}

TEST_CASE("Reentrancy: a menu item callback closes the menu itself and the window menu", "[reentrancy]") {
    F f;
    auto &b = f.window->appContainer().add<Button>("a");
    f.layout();
    Window     *w = f.window.get();
    const Menu &m = w->openMenu({Menu::item("One", [w]() { w->closeMenu(); w->overlay().closeAll<Popup>(); })}, b);
    (void) m;
    f.layout();
    auto *row = m.rows().at(0); (void) row;
    f.click(*row);
    f.layout();
}

TEST_CASE("Reentrancy: submenu chosen item closes everything", "[reentrancy]") {
    F f;
    auto &b = f.window->appContainer().add<Button>("a");
    f.layout();
    Window *w = f.window.get();
    w->openMenu({Menu::item("Sub", {Menu::item("Leaf", [w]() { w->overlay().closeAll<Popup>(); })})}, b);
    f.layout();
    f.key(Key::DOWN);
    f.key(Key::RIGHT);
    f.key(Key::ENTER);
    f.layout();
}

TEST_CASE("Reentrancy: destroy window while popups are open", "[reentrancy]") {
    auto f = std::make_unique<F>();
    auto &b = f->window->appContainer().add<Button>("a").tooltip("tip");
    auto &s = f->window->appContainer().add<Select>(std::vector<std::string>{"A", "B"}, 0);
    f->layout();
    f->window->openMenu({Menu::item("Sub", {Menu::item("Leaf", []() {})})}, b);
    f->layout();
    f->key(Key::DOWN);
    f->key(Key::RIGHT);
    f->window->overlay().open<Dialog>("Hi").action("OK", []() {});
    f->window->toast({.title = "t"});
    f->layout();
    (void) s;
    f->sys.reset();
    f->window.reset();
    f.reset();
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
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("a").tooltip("tip");
    f.layout();
    auto [x, y] = F::center(b);
    f.move(x, y);
    f.window->updatePopups(std::chrono::steady_clock::now().time_since_epoch());
    f.window->updatePopups(std::chrono::steady_clock::now().time_since_epoch() + 1s);
    f.layout();
    parent.remove(b);
    f.layout();
    f.window->updatePopups(std::chrono::steady_clock::now().time_since_epoch() + 2s);
    f.layout();
    f.move(x + 1, y);
    f.layout();
}

TEST_CASE("Reentrancy: remove a tooltip's div before its delay ends", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("a").tooltip("tip");
    f.layout();
    auto [x, y] = F::center(b);
    f.move(x, y);
    auto t0 = std::chrono::steady_clock::now().time_since_epoch();
    f.window->updatePopups(t0);
    parent.remove(b);
    f.window->updatePopups(t0 + 1s);
    f.layout();
}

TEST_CASE("Reentrancy: remove the anchor of an open menu and select", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &s      = parent.add<Select>(std::vector<std::string>{"A", "B"}, 0);
    f.layout();
    f.click(s);
    f.layout();
    parent.remove(s);
    f.layout();
    f.key(Key::DOWN);
    f.key(Key::ENTER);
    f.layout();
}

TEST_CASE("Reentrancy: select removes itself on change", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &s      = parent.add<Select>(std::vector<std::string>{"A", "B"}, 0);
    f.layout();
    s.onChange([&parent, &s]() { parent.remove(s); });
    f.click(s);
    f.key(Key::DOWN);
    f.key(Key::ENTER);
    f.layout();
}

TEST_CASE("Reentrancy: remove a tree node holding the selection / collapse during change", "[reentrancy]") {
    F f;
    auto &tree = f.window->appContainer().add<Tree>();
    auto &root = tree.node("root");
    auto &kid  = root.node("kid");
    kid.node("grandkid");
    root.expanded(true);
    kid.expanded(true);
    f.layout();
    tree.onExpansionChange([&kid](TreeNode *node, bool) {
        if (node == &kid && kid.parent()) kid.parent()->remove(kid);
    });
    tree.onSelectionChange([](TreeNode *) {});
    f.key(Key::TAB);
    f.key(Key::DOWN);
    f.key(Key::DOWN);
    f.key(Key::LEFT);
    f.key(Key::LEFT);
    f.key(Key::ENTER);
    f.layout();
}

TEST_CASE("Reentrancy: sort a data grid from its selection change and remove rows", "[reentrancy]") {
    F f;
    auto &grid = f.window->appContainer().add<DataGrid>();
    grid.columns({{"Name"}, {.title = "Size", .width = 80}});
    for (int i = 0; i < 6; ++i) grid.row({"n" + std::to_string(i), std::to_string(10 - i)});
    f.layout();
    grid.onSelectionChange([&grid](DataGridRow *) { grid.sort(1, SortOrder::Ascending); });
    f.key(Key::TAB);
    f.key(Key::DOWN);
    f.key(Key::DOWN);
    f.layout();
}

TEST_CASE("Reentrancy: change theme and skin set inside styleUpdated / mid event", "[reentrancy]") {
    F f;
    auto &b = f.window->appContainer().add<Button>("a");
    auto &t = f.window->appContainer().add<TextInput>("hello");
    auto &sw = f.window->appContainer().add<Switch>();
    (void) sw;
    f.layout();
    Window *w = f.window.get();
    int     n = 0;
    b.onClick([w, &n]() {
        if (n++ % 2) w->skinSet<FloatSkins>(); else w->skinSet<PivotalSkins>();
        w->theme(themes::pivotalLight);
    });
    f.click(b);
    f.click(b);
    f.click(t);
    t.onKeyPress([w]() { w->skinSet<FloatSkins>(); });
    f.key(Key::A);
    f.key(Key::BACKSPACE);
    f.layout();
}

namespace {
    struct ThemeFlipper : Div {
        Window *w;
        int     n{0};
        explicit ThemeFlipper(Window *w) : w(w) {}
        void styleUpdated() override {
            Div::styleUpdated();
            if (n++ < 3) w->theme(n % 2 ? themes::pivotalLight : themes::pivotalDark);
        }
    };
    struct SkinFlipper : Div {
        Window *w;
        int     n{0};
        explicit SkinFlipper(Window *w) : w(w) {}
        void styleUpdated() override {
            Div::styleUpdated();
            if (n++ < 3) { if (n % 2) w->skinSet<FloatSkins>(); else w->skinSet<PivotalSkins>(); }
        }
    };
}

TEST_CASE("Reentrancy: theme switched inside styleUpdated", "[reentrancy]") {
    F f;
    f.window->appContainer().add<Button>("a");
    f.window->appContainer().add<ThemeFlipper>(f.window.get());
    f.window->appContainer().add<TextInput>("t");
    f.layout();
    f.layout();
}

TEST_CASE("Reentrancy: skin set switched inside styleUpdated", "[reentrancy]") {
    F f;
    f.window->appContainer().add<Button>("a");
    f.window->appContainer().add<SkinFlipper>(f.window.get());
    f.window->appContainer().add<TextInput>("t");
    f.window->appContainer().add<Switch>();
    f.layout();
    f.layout();
}

TEST_CASE("Reentrancy: re-hatch a button's skin from its own press", "[reentrancy]") {
    F f;
    auto &b = f.window->appContainer().add<Button>("a");
    f.layout();
    b.onMousePress([&b]() { b.skin("primary"); });
    b.onClick([&b]() { b.skin(""); });
    f.click(b);
    f.layout();
    f.click(b);
    f.layout();
}

TEST_CASE("Reentrancy: toast action removes all toasts and opens a dialog", "[reentrancy]") {
    F f;
    Window *w = f.window.get();
    auto &t   = w->toast({.title = "t", .actions = {{.label = "Go", .callback = [w]() {
        w->overlay().closeAll<Popup>();
        w->overlay().open<Dialog>("x");
    }}}});
    w->toast({.title = "u"});
    f.layout();
    Div *btn = nullptr;
    std::function<void(Div &)> find = [&](Div &d) {
        for (const auto &c: d.children()) {
            if (auto *bb = dynamic_cast<Button *>(c.get()); bb && !btn) btn = bb;
            find(*c);
        }
    };
    find(t);
    REQUIRE(btn);
    f.click(*btn);
    f.layout();
    f.key(Key::ESCAPE);
}

TEST_CASE("Reentrancy: toasts time out while one is being clicked away; more than the limit", "[reentrancy]") {
    F f;
    Window *w = f.window.get();
    for (int i = 0; i < 8; ++i) {
        w->toast({.title = "t" + std::to_string(i), .timeout = 1ms});
        f.layout();
    }
    auto t0 = std::chrono::steady_clock::now().time_since_epoch();
    w->updatePopups(t0);
    w->updatePopups(t0 + 1s);
    f.layout();
    w->toast({.title = "z"});
    w->updatePopups(t0 + 2s);
    w->updatePopups(t0 + 10s);
    f.layout();
}

TEST_CASE("Reentrancy: dialog action closes and removes the opener", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("open");
    f.layout();
    f.key(Key::TAB);
    const Window *w = f.window.get();
    auto   &d = w->overlay().open<Dialog>("Hi");
    d.action("OK", [&parent, &b]() { parent.remove(b); });
    d.body().add<TextInput>("x");
    f.layout();
    f.key(Key::TAB);
    f.key(Key::TAB);
    f.key(Key::TAB);
    f.key(Key::ENTER);
    f.layout();
    f.key(Key::TAB);
}

TEST_CASE("Reentrancy: chip dismissed removes itself", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &c      = parent.add<Chip>("x").removable(true);
    f.layout();
    c.onDismiss([&parent, &c]() { parent.remove(c); });
    f.key(Key::TAB);
    f.key(Key::BACKSPACE);
    f.layout();
    f.key(Key::TAB);
}

TEST_CASE("Reentrancy: search field clears and removes itself on submit", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &s      = parent.add<SearchField>();
    f.layout();
    f.click(s);
    f.window->handleType("abc");
    f.layout();
    s.onSubmit([&parent, &s]() { parent.remove(s); });
    f.key(Key::ENTER);
    f.layout();
    f.key(Key::ESCAPE);
}

TEST_CASE("Reentrancy: context menu whose item removes the div with the context menu", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &d      = parent.add<Div>().size(100, 100);
    d.contextMenu({Menu::item("Delete", [&parent, &d]() { parent.remove(d); })});
    f.layout();
    auto [x, y] = F::center(d);
    f.press(x, y, MouseButton::RIGHT);
    f.release(x, y, MouseButton::RIGHT);
    f.layout();
    f.key(Key::DOWN);
    f.key(Key::ENTER);
    f.layout();
    f.press(x, y, MouseButton::RIGHT);
    f.release(x, y, MouseButton::RIGHT);
    f.layout();
}

TEST_CASE("Reentrancy: touch long press then remove target before release", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x").tooltip("tip");
    f.layout();
    auto [x, y] = F::center(b);
    auto t0     = std::chrono::steady_clock::now().time_since_epoch();
    f.window->handleTouchPress(x, y, t0);
    f.window->updateGestures(t0 + 600ms);
    f.window->updatePopups(t0 + 600ms);
    f.layout();
    parent.remove(b);
    f.window->handleTouchMove(x + 30, y, t0 + 700ms);
    f.window->handleTouchRelease(x + 30, y, t0 + 800ms);
    f.layout();
}

TEST_CASE("Reentrancy: touch pan a scroller and remove it mid-fling", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &sc     = parent.add<Div>().size(200, 200).overflow(Overflow::Scroll);
    for (int i = 0; i < 30; ++i) sc.add<Div>().height(40);
    f.layout();
    auto [x, y] = F::center(sc);
    auto t0     = std::chrono::steady_clock::now().time_since_epoch();
    f.window->handleTouchPress(x, y, t0);
    f.window->handleTouchMove(x, y - 20, t0 + 10ms);
    f.window->handleTouchMove(x, y - 60, t0 + 20ms);
    f.window->handleTouchRelease(x, y - 80, t0 + 30ms);
    f.window->updateGestures(t0 + 40ms);
    parent.remove(sc);
    f.window->updateGestures(t0 + 60ms);
    f.window->updateGestures(t0 + 200ms);
    f.layout();
}

TEST_CASE("Reentrancy: press on a button, remove it, release elsewhere", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    f.layout();
    auto [x, y] = F::center(b);
    f.press(x, y);
    parent.remove(b);
    f.move(x + 50, y + 50);
    f.release(x + 50, y + 50);
    f.layout();
}

TEST_CASE("Reentrancy: hovered div removed then mouse moves", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    f.layout();
    auto [x, y] = F::center(b);
    f.move(x, y);
    b.onMouseExit([]() {});
    parent.remove(b);
    f.move(x + 1, y);
    f.move(500, 500);
    f.layout();
}

TEST_CASE("Reentrancy: mouseExited removes the div being exited", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &b      = parent.add<Button>("x");
    f.layout();
    auto [x, y] = F::center(b);
    f.move(x, y);
    b.onMouseExit([&parent, &b]() { parent.remove(b); });
    f.move(700, 500);
    f.layout();
}

TEST_CASE("Reentrancy: mouseEntered removes siblings on the path", "[reentrancy]") {
    F f;
    auto &outer = f.window->appContainer().add<Div>();
    auto &inner = outer.add<Div>().size(100, 100);
    auto &b     = inner.add<Button>("x");
    f.layout();
    inner.onMouseEnter([&outer]() { outer.removeAll(); });
    auto [x, y] = F::center(b);
    f.move(x, y);
    f.layout();
}

TEST_CASE("Reentrancy: menu radio callback closes the overlay and switches skin set", "[reentrancy]") {
    F f;
    auto &b = f.window->appContainer().add<Button>("a");
    f.layout();
    Window *w     = f.window.get();
    auto    group = std::make_shared<MenuRadioGroup>("0", [w](const std::string &) {
        w->skinSet<FloatSkins>();
        w->overlay().closeAll<Popup>();
    });
    w->openMenu({Menu::radio(group, "A", "0"), Menu::radio(group, "B", "1")}, b);
    f.layout();
    f.key(Key::DOWN);
    f.key(Key::DOWN);
    f.key(Key::ENTER);
    f.layout();
}

TEST_CASE("Reentrancy: data grid removes the selected row on selection change (keys)", "[reentrancy]") {
    F f;
    auto &grid = f.window->appContainer().add<DataGrid>();
    grid.columns({{"Name"}});
    for (int i = 0; i < 5; ++i) grid.row({"n" + std::to_string(i)});
    f.layout();
    grid.onSelectionChange([](DataGridRow *r) { if (r && r->parent()) r->parent()->remove(*r); });
    f.key(Key::TAB);
    f.key(Key::DOWN);
    f.key(Key::DOWN);
    f.layout();
}

TEST_CASE("Reentrancy: list removes itself on selection change (click)", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    auto &list   = parent.add<List>();
    auto &a      = list.row("a");
    list.row("b");
    f.layout();
    list.onSelectionChange([&parent, &list](ListRow *) { if (list.parent()) parent.remove(list); });
    f.click(a);
    f.layout();
}

TEST_CASE("Reentrancy: menu bar removed by its own item", "[reentrancy]") {
    F f;
    auto &parent = f.window->appContainer().add<Div>();
    MenuBar *bar = nullptr;
    bar = &parent.add<MenuBar>(std::vector<std::shared_ptr<MenuItem>>{
        Menu::item("File", {Menu::item("Quit", [&parent, &bar]() { parent.remove(*bar); })}),
        Menu::item("Edit", {Menu::item("Copy", []() {})})});
    f.layout();
    const Div *trigger = bar->children().size() > 1 ? bar->children()[1].get() : bar->children()[0].get();
    f.click(*trigger);
    f.key(Key::DOWN);
    f.key(Key::ENTER);
    f.layout();
    f.layout();
}

TEST_CASE("Reentrancy: window destroyed with focus and hover inside", "[reentrancy]") {
    auto f = std::make_unique<F>();
    auto &t = f->window->appContainer().add<TextInput>("x");
    auto &b = f->window->appContainer().add<Button>("b").tooltip("tip");
    f->layout();
    f->click(t);
    auto [x, y] = F::center(b);
    f->move(x, y);
    f->press(x, y);
    f.reset();
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
