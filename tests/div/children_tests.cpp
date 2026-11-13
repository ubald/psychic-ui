#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <yoga/Yoga.h>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Style.hpp>

using namespace psychic_ui;

namespace {
    // Exposes what the tests need to compare against Yoga and to lay out without a Window
    class TestDiv : public Div {
    public:
        explicit TestDiv(std::string name = "") : name(std::move(name)) {}

        using Div::_yogaNode;
        using Div::layoutUpdated;

        std::string name;
    };

    std::shared_ptr<TestDiv> make(const std::string &name) {
        return std::make_shared<TestDiv>(name);
    }

    // Child names in Div order, checking that Yoga holds the same nodes in the same order
    std::string order(TestDiv &parent) {
        REQUIRE(YGNodeGetChildCount(parent._yogaNode) == parent.childCount());
        std::string names;
        for (unsigned int i = 0; i < parent.childCount(); ++i) {
            auto child = dynamic_cast<TestDiv *>(parent.at(i));
            REQUIRE(child != nullptr);
            REQUIRE(YGNodeGetChild(parent._yogaNode, i) == child->_yogaNode);
            REQUIRE(parent.childIndex(child) == (int) i);
            REQUIRE(parent.children()[i].get() == child);
            REQUIRE(child->parent() == &parent);
            names += child->name;
        }
        return names;
    }
}

TEST_CASE("children are kept in document order, matching yoga", "[div][children]") {
    auto parent = make("p");
    auto a      = make("a");
    auto b      = make("b");
    auto c      = make("c");

    SECTION("add appends") {
        parent->add(a);
        parent->add(b);
        parent->add(c);
        REQUIRE(order(*parent) == "abc");
        REQUIRE(parent->childIndex(a) == 0);
        REQUIRE(parent->childIndex(c) == 2);
    }

    SECTION("add at index inserts") {
        parent->add(0, b);
        parent->add(0, a);
        parent->add(2, c);
        REQUIRE(order(*parent) == "abc");
        auto d = make("d");
        parent->add(1, d);
        REQUIRE(order(*parent) == "adbc");
    }

    SECTION("childIndex of a stranger is -1") {
        parent->add(a);
        REQUIRE(parent->childIndex(b) == -1);
        REQUIRE(parent->childIndex(b.get()) == -1);
    }

    SECTION("remove by index removes the right child from both lists") {
        auto d = make("d");
        parent->add(a);
        parent->add(b);
        parent->add(c);
        parent->add(d);

        parent->remove(0u);
        REQUIRE(order(*parent) == "bcd");
        REQUIRE(a->parent() == nullptr);
        REQUIRE(YGNodeGetOwner(a->_yogaNode) == nullptr);

        parent->remove(1u);
        REQUIRE(order(*parent) == "bd");
        REQUIRE(c->parent() == nullptr);

        parent->remove(1u);
        REQUIRE(order(*parent) == "b");
        REQUIRE(d->parent() == nullptr);

        parent->remove(0u);
        REQUIRE(order(*parent).empty());
        REQUIRE(b->parent() == nullptr);
    }

    SECTION("remove by pointer") {
        parent->add(a);
        parent->add(b);
        parent->add(c);
        parent->remove(b);
        REQUIRE(order(*parent) == "ac");
        REQUIRE(b->parent() == nullptr);

        // Removing a stranger leaves everything alone
        auto other = make("o");
        other->add(b);
        parent->remove(b);
        REQUIRE(order(*parent) == "ac");
        REQUIRE(b->parent() == other.get());
    }

    SECTION("removeAll") {
        parent->add(a);
        parent->add(b);
        parent->add(c);
        parent->removeAll();
        REQUIRE(parent->childCount() == 0);
        REQUIRE(YGNodeGetChildCount(parent->_yogaNode) == 0);
        for (auto &child: {a, b, c}) {
            REQUIRE(child->parent() == nullptr);
            REQUIRE(YGNodeGetOwner(child->_yogaNode) == nullptr);
        }
        // Children can be re-added after removeAll
        parent->add(c);
        parent->add(a);
        REQUIRE(order(*parent) == "ca");
    }
}

TEST_CASE(":first-child and :last-child match the document order", "[div][children][style]") {
    auto styleManager = std::make_shared<StyleManager>();
    styleManager->style("div").fontFamily("").skin("");
    styleManager->style("div:first-child").fontFamily("F");
    styleManager->style("div:last-child").skin("L");

    auto parent = make("p");
    parent->setStyleManager(styleManager);

    // "F" when matching :first-child, "L" when matching :last-child, as last computed
    auto matched = [](const std::shared_ptr<TestDiv> &div) {
        return div->computedStyle().fontFamily().str() + div->computedStyle().skin().str();
    };
    auto restyled = [&matched](const std::shared_ptr<TestDiv> &div) {
        div->updateStyle();
        return matched(div);
    };

    auto a = make("a");
    parent->add(a);
    REQUIRE(matched(a) == "FL");

    auto b = make("b");
    parent->add(b);
    auto c = make("c");
    parent->add(c);
    REQUIRE(restyled(a) == "F");
    REQUIRE(restyled(b) == "");
    REQUIRE(restyled(c) == "L");

    SECTION("a new child sees its position in its first style pass") {
        auto d = make("d");
        parent->add(d);
        REQUIRE(matched(d) == "L");
        auto e = make("e");
        parent->add(0, e);
        REQUIRE(matched(e) == "F");
        REQUIRE(restyled(a) == "");
        REQUIRE(restyled(c) == "");
    }

    SECTION("removing the ends moves first and last") {
        parent->remove(0u);
        REQUIRE(restyled(b) == "F");
        parent->remove(1u);
        REQUIRE(restyled(b) == "FL");
    }
}

TEST_CASE("mouse events go to the topmost (last) child first", "[div][children][mouse]") {
    auto styleManager = std::make_shared<StyleManager>();
    auto root         = make("root");
    root->setStyleManager(styleManager);
    root->width(100).height(100);

    std::vector<std::shared_ptr<TestDiv>> children;
    std::string                           hits;
    for (const auto *name: {"a", "b", "c"}) {
        auto child = make(name);
        root->add(child);
        child->position(Position::Absolute)
             .left(0)
             .top(0)
             .width(100)
             .height(100);
        child->mouseButtonChanged.subscribe(
            [&hits, name](const int, const int, const MouseButton, const bool, const Mod) {
                hits += name;
            }
        );
        children.push_back(child);
    }

    auto layout = [&root]() {
        root->updateStyleRecursive();
        YGNodeCalculateLayout(root->_yogaNode, YGUndefined, YGUndefined, YGDirectionLTR);
        root->layoutUpdated();
    };
    layout();

    SECTION("the last child added is hit") {
        REQUIRE(root->handleMouseButton(50, 50, LEFT, true, Mod{}) == Handled);
        REQUIRE(hits == "c");
    }

    SECTION("a child inserted at the front is under the others") {
        auto d = make("d");
        root->add(0, d);
        d->position(Position::Absolute).width(100).height(100);
        d->mouseButtonChanged.subscribe([&hits](const int, const int, const MouseButton, const bool, const Mod) { hits += "d"; });
        layout();
        root->handleMouseButton(50, 50, LEFT, true, Mod{});
        REQUIRE(hits == "c");
        root->remove(children[2]);
        root->remove(children[1]);
        root->remove(children[0]);
        layout();
        root->handleMouseButton(50, 50, LEFT, true, Mod{});
        REQUIRE(hits == "cd");
    }

    SECTION("handlers can remove and add siblings while the event is dispatched") {
        // mouseMoved visits every child, topmost first. Pass handled = true so no Window is needed.
        std::string moves;
        children[2]->mouseMoved.subscribe(
            [&](const int, const int, const int, const Mod) {
                moves += "c";
                root->remove(children[1]);  // Not visited yet, must be skipped
                root->removeAll();          // Clear everything, including the node being dispatched
                root->add(make("e"));       // Must not be visited during this dispatch
            }
        );
        children[1]->mouseMoved.subscribe([&](const int, const int, const int, const Mod) { moves += "b"; });
        children[0]->mouseMoved.subscribe([&](const int, const int, const int, const Mod) { moves += "a"; });

        root->handleMouseMoved(50, 50, 0, Mod{}, true);
        REQUIRE(moves == "c");
        REQUIRE(order(*root) == "e");
    }
}
