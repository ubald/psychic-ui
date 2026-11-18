#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <yoga/YGEnums.h>
#include <yoga/YGNode.h>
#include <yoga/YGValue.h>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Style.hpp>
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>

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
            auto *child = dynamic_cast<TestDiv *>(parent.at(i));
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
    auto childA = make("a");
    auto childB = make("b");
    auto childC = make("c");

    SECTION("add appends") {
        parent->add(childA);
        parent->add(childB);
        parent->add(childC);
        REQUIRE(order(*parent) == "abc");
        REQUIRE(parent->childIndex(childA) == 0);
        REQUIRE(parent->childIndex(childC) == 2);
    }

    SECTION("add at index inserts") {
        parent->add(0, childB);
        parent->add(0, childA);
        parent->add(2, childC);
        REQUIRE(order(*parent) == "abc");
        auto childD = make("d");
        parent->add(1, childD);
        REQUIRE(order(*parent) == "adbc");
    }

    SECTION("childIndex of a stranger is -1") {
        parent->add(childA);
        REQUIRE(parent->childIndex(childB) == -1);
        REQUIRE(parent->childIndex(childB.get()) == -1);
    }

    SECTION("remove by index removes the right child from both lists") {
        auto childD = make("d");
        parent->add(childA);
        parent->add(childB);
        parent->add(childC);
        parent->add(childD);

        parent->remove(0u);
        REQUIRE(order(*parent) == "bcd");
        REQUIRE(childA->parent() == nullptr);
        REQUIRE(YGNodeGetOwner(childA->_yogaNode) == nullptr);

        parent->remove(1u);
        REQUIRE(order(*parent) == "bd");
        REQUIRE(childC->parent() == nullptr);

        parent->remove(1u);
        REQUIRE(order(*parent) == "b");
        REQUIRE(childD->parent() == nullptr);

        parent->remove(0u);
        REQUIRE(order(*parent).empty());
        REQUIRE(childB->parent() == nullptr);
    }

    SECTION("remove by pointer") {
        parent->add(childA);
        parent->add(childB);
        parent->add(childC);
        parent->remove(childB);
        REQUIRE(order(*parent) == "ac");
        REQUIRE(childB->parent() == nullptr);

        // Removing a stranger leaves everything alone
        auto other = make("o");
        other->add(childB);
        parent->remove(childB);
        REQUIRE(order(*parent) == "ac");
        REQUIRE(childB->parent() == other.get());
    }

    SECTION("removeAll") {
        parent->add(childA);
        parent->add(childB);
        parent->add(childC);
        parent->removeAll();
        REQUIRE(parent->childCount() == 0);
        REQUIRE(YGNodeGetChildCount(parent->_yogaNode) == 0);
        for (const auto &child: {childA, childB, childC}) {
            REQUIRE(child->parent() == nullptr);
            REQUIRE(YGNodeGetOwner(child->_yogaNode) == nullptr);
        }
        // Children can be re-added after removeAll
        parent->add(childC);
        parent->add(childA);
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

    auto childA = make("a");
    parent->add(childA);
    REQUIRE(matched(childA) == "FL");

    auto childB = make("b");
    parent->add(childB);
    auto childC = make("c");
    parent->add(childC);
    REQUIRE(restyled(childA) == "F");
    REQUIRE(restyled(childB) == "");
    REQUIRE(restyled(childC) == "L");

    SECTION("a new child sees its position in its first style pass") {
        auto childD = make("d");
        parent->add(childD);
        REQUIRE(matched(childD) == "L");
        auto childE = make("e");
        parent->add(0, childE);
        REQUIRE(matched(childE) == "F");
        REQUIRE(restyled(childA) == "");
        REQUIRE(restyled(childC) == "");
    }

    SECTION("removing the ends moves first and last") {
        parent->remove(0u);
        REQUIRE(restyled(childB) == "F");
        parent->remove(1u);
        REQUIRE(restyled(childB) == "FL");
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
            [&hits, name]() {
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
        REQUIRE(root->handleMouseButtonChange({.x = 50, .y = 50, .button = MouseButton::LEFT, .down = true}) == MouseEventStatus::Handled);
        REQUIRE(hits == "c");
    }

    SECTION("a child inserted at the front is under the others") {
        auto childD = make("d");
        root->add(0, childD);
        childD->position(Position::Absolute).width(100).height(100);
        childD->mouseButtonChanged.subscribe([&hits]() { hits += "d"; });
        layout();
        root->handleMouseButtonChange({.x = 50, .y = 50, .button = MouseButton::LEFT, .down = true});
        REQUIRE(hits == "c");
        root->remove(children[2]);
        root->remove(children[1]);
        root->remove(children[0]);
        layout();
        root->handleMouseButtonChange({.x = 50, .y = 50, .button = MouseButton::LEFT, .down = true});
        REQUIRE(hits == "cd");
    }

    SECTION("handlers can remove and add siblings while the event is dispatched") {
        // mouseMoved visits every child, topmost first. Pass handled = true so no Window is needed.
        std::string moves;
        children[2]->mouseMoved.subscribe(
            [&]() {
                moves += "c";
                root->remove(children[1]);  // Not visited yet, must be skipped
                root->removeAll();          // Clear everything, including the node being dispatched
                root->add(make("e"));       // Must not be visited during this dispatch
            }
        );
        children[1]->mouseMoved.subscribe([&]() { moves += "b"; });
        children[0]->mouseMoved.subscribe([&]() { moves += "a"; });

        root->handleMouseMove({.x = 50, .y = 50}, true);
        REQUIRE(moves == "c");
        REQUIRE(order(*root) == "e");
    }
}
