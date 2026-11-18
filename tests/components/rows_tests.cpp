// Known bugs from review-2026-11.md, tagged [!shouldfail]: remove the tag in the commit that fixes one.
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/DataGrid.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Tree.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

// C4: DataGrid.cpp compares cells with strtod, which reads "Nancy" as NaN and "Infra" as infinity
TEST_CASE("C4: DataGrid sorts words that start like nan or inf as text", "[components][datagrid][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    const FakeSystemWindow sys{window, 800, 600};
    auto            &grid = window->appContainer().add<DataGrid>().columns({{"Name"}});
    grid.row({"Nancy"});
    grid.row({"Infra"});
    grid.row({"Bob"});
    grid.sort(0, SortOrder::Ascending);
    std::vector<std::string> order{};
    for (const DataGridRow *row: grid.dataRows()) {
        order.push_back(row->value(0));
    }
    CHECK(order == std::vector<std::string>{"Bob", "Infra", "Nancy"});
}

// C8: Rows.cpp's forget drops the Tab stop with the removed row and gives it to no other
TEST_CASE("C8: removing the row that holds the list's Tab stop leaves the list reachable by Tab", "[components][list][focus][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    auto            &before = window->appContainer().add<Button>("Before");
    auto            &list   = window->appContainer().add<List>();
    auto            &first  = list.row("One");
    auto            &second = list.row("Two");
    list.row("Three");
    window->appContainer().add<Button>("After");
    sys.layout();
    list.remove(first);
    sys.layout();
    window->requestFocus(&before, true);
    window->focusNext(false);
    CHECK(window->focusedDiv() == &second);
}

// M8: removed() fires only on the root of a removed subtree, so a TreeNode's row never tells its tree it left
TEST_CASE("M8: a selected tree node moved to another tree stops being the first tree's selection", "[components][tree][!shouldfail]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow sys{window, 800, 600};
    auto            &tree1 = window->appContainer().add<Tree>();
    auto            &tree2 = window->appContainer().add<Tree>();
    auto            &node  = tree1.node("a");
    sys.layout();
    tree1.select(0);
    REQUIRE(tree1.selection() != nullptr);
    auto keep = node.shared_from_this();
    node.parent()->remove(node);
    tree2.add(keep);
    sys.layout();
    CHECK(tree1.selection() == nullptr);
    // Free the row while tree1 lives: its destructor tells the group that still holds it (the hidden asan test
    // lets tree1 die first)
    tree2.removeAll();
    keep.reset();
}
