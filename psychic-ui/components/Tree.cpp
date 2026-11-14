#include <algorithm>
#include "Tree.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "psychic-ui/Window.hpp"

namespace psychic_ui {

    // region Row

    TreeRow::TreeRow() :
        SelectableRow() {
        tag("TreeRow");
        addClassName("row");

        _indent = &add<Div>();
        _indent->addClassName("indent")
               .shrink(0);
        _disclosure = &add<Icon>();
        _disclosure->addClassName("disclosure")
                   .shrink(0);
        _disclosure->onClick(
            [this]() {
                if (TreeNode *n = node(); n && n->hasChildren()) {
                    n->toggle();
                }
            }
        );
        _disclosure->onDoubleClick([](unsigned int /*count*/) {}); // Two clicks on it are two toggles, not three
        _icon = &add<Icon>();
        _icon->addClassName("icon")
             .display(Display::None);
        _label = &add<Label>();
        _label->addClassName("label")
              .grow(1)
              .shrink(1);
        _trailing = &add<Label>();
        _trailing->addClassName("trailing")
                 .display(Display::None);

        doubleClicked.subscribe(
            [this](unsigned int /*count*/) {
                if (TreeNode *n = node(); n && n->hasChildren()) {
                    n->toggle();
                }
            }
        );
    }

    TreeNode *TreeRow::node() const {
        return dynamic_cast<TreeNode *>(const_cast<Div *>(parent()));
    }

    // endregion

    // region Node

    TreeNode::TreeNode(const std::string &label, const std::string &icon) :
        Div() {
        tag("TreeNode");
        _defaultStyle.direction(Direction::Column)
            .shrink(0);
        _row = &add<TreeRow>();
        _children = &add<Div>();
        _children->addClassName("children")
                 .direction(Direction::Column)
                 .display(Display::None);
        updateLabel(label);
        updateIcon(icon);
        updateDisclosure();
    }

    TreeNode &TreeNode::node(const std::string &label, const std::string &icon) {
        auto &child = _children->add<TreeNode>(label, icon);
        updateDisclosure();
        return child;
    }

    std::vector<TreeNode *> TreeNode::nodes() const {
        std::vector<TreeNode *> nodes{};
        for (const auto &child: _children->children()) {
            if (auto node = std::dynamic_pointer_cast<TreeNode>(child)) {
                nodes.push_back(node.get());
            }
        }
        return nodes;
    }

    bool TreeNode::hasChildren() const {
        return !_children->children().empty();
    }

    TreeNode *TreeNode::parentNode() const {
        // In its parent node's `.children`
        const Div *container = parent();
        return container ? dynamic_cast<TreeNode *>(const_cast<Div *>(container->parent())) : nullptr;
    }

    Tree *TreeNode::tree() const {
        for (const Div *div = parent(); div; div = div->parent()) {
            if (auto t = dynamic_cast<const Tree *>(div)) {
                return const_cast<Tree *>(t);
            }
        }
        return nullptr;
    }

    const std::string &TreeNode::label() const {
        return _row->_label->text();
    }

    void TreeNode::updateLabel(const std::string &label) {
        _row->_label->text(label);
    }

    void TreeNode::updateIcon(const std::string_view icon) {
        _row->_icon->name(icon)
                   .display(icon.empty() ? Display::None : Display::Flex);
    }

    void TreeNode::updateTrailing(const std::string &trailing) {
        _row->_trailing->text(trailing)
                       .display(trailing.empty() ? Display::None : Display::Flex);
    }

    void TreeNode::updateExpanded(const bool expanded) {
        if (expanded == _expanded) {
            return;
        }
        _expanded = expanded;
        _children->display(_expanded ? Display::Flex : Display::None);
        if (_expanded) {
            addClassName("expanded");
        } else {
            removeClassName("expanded");
        }
        updateDisclosure();
        if (Tree *t = tree()) {
            if (!_expanded) {
                t->collapsed(*this);
            }
            t->expansionChanged.emit(this, _expanded);
        }
    }

    void TreeNode::updateDisclosure() {
        _row->_disclosure->name(!hasChildren() ? "" : _expanded ? "chevron-down" : "disclosure");
    }

    void TreeNode::added() {
        Div::added();
        if (TreeNode *parentNode = this->parentNode()) {
            _level = parentNode->_level + 1;
            parentNode->updateDisclosure(); // Added to its children by hand
        } else {
            _level = 0;
        }
        Tree *t = tree();
        updateLevel(t ? t->indent() : 16.0f);
        if (t) {
            t->adopt(*this);
        }
    }

    void TreeNode::updateLevel(const float indent) {
        if (TreeNode *parentNode = this->parentNode()) {
            _level = parentNode->_level + 1;
        }
        _row->_indent->width(static_cast<float>(_level) * indent);
        for (TreeNode *child: nodes()) {
            child->updateLevel(indent);
        }
    }

    // endregion

    // region Tree

    Tree::Tree() :
        RowGroup() {
        tag("Tree");
        _defaultStyle.direction(Direction::Column);
    }

    TreeNode &Tree::node(const std::string &label, const std::string &icon) {
        return add<TreeNode>(label, icon);
    }

    std::vector<TreeNode *> Tree::nodes() const {
        std::vector<TreeNode *> nodes{};
        for (const auto &child: _children) {
            if (auto node = std::dynamic_pointer_cast<TreeNode>(child)) {
                nodes.push_back(node.get());
            }
        }
        return nodes;
    }

    namespace {
        void visibleRows(const std::vector<TreeNode *> &nodes, std::vector<SelectableRow *> &rows) {
            for (TreeNode *node: nodes) {
                rows.push_back(&node->row());
                if (node->expanded()) {
                    visibleRows(node->nodes(), rows);
                }
            }
        }

        void allNodes(TreeNode &node, std::vector<TreeNode *> &nodes) {
            nodes.push_back(&node);
            for (TreeNode *child: node.nodes()) {
                allNodes(*child, nodes);
            }
        }
    }

    std::vector<SelectableRow *> Tree::rows() const {
        std::vector<SelectableRow *> rows{};
        visibleRows(nodes(), rows);
        return rows;
    }

    TreeNode *Tree::selectedNode() const {
        auto row = dynamic_cast<TreeRow *>(_selection);
        return row ? row->node() : nullptr;
    }

    void Tree::selectionUpdated() {
        selectionChanged.emit(selectedNode());
    }

    void Tree::updateIndent(const float indent) {
        _indent = indent;
        for (TreeNode *node: nodes()) {
            node->updateLevel(_indent);
        }
    }

    void Tree::adopt(TreeNode &node) {
        std::vector<TreeNode *> subtree{};
        allNodes(node, subtree);
        for (TreeNode *n: subtree) {
            rowAdded(n->_row);
        }
        if (!_tabStop || !_selection) {
            updateTabStop(); // The first visible row, which may have changed
        }
    }

    void Tree::collapsed(TreeNode &node) {
        // Whether a row is hidden in the node: one of its descendants
        const auto inside = [&node](const Div *div) {
            for (const Div *d = div ? div->parent() : nullptr; d; d = d->parent()) {
                if (d == node._children) {
                    return true;
                }
            }
            return false;
        };
        bool refocus = false;
        for (const SelectableRow *row: RowGroup::rows()) {
            refocus = refocus || (row->hasFocus() && inside(row));
        }
        if (inside(_selection)) {
            updateSelection(node._row);
        }
        if (Window *win = window(); win && refocus) {
            win->requestFocus(node._row, win->focusVisible());
        }
        if (inside(_tabStop)) {
            updateTabStop(node._row);
        }
    }

    bool Tree::handleRowKey(SelectableRow *row, const Key key, const Mod mod) {
        auto     treeRow = dynamic_cast<TreeRow *>(row);
        TreeNode *node   = treeRow ? treeRow->node() : nullptr;
        if (node) {
            switch (key) {
                case Key::RIGHT:
                    if (node->hasChildren()) {
                        if (!node->expanded()) {
                            node->expanded(true);
                        } else {
                            moveTo(&node->nodes().front()->row());
                        }
                    }
                    return true;
                case Key::LEFT:
                    if (node->hasChildren() && node->expanded()) {
                        node->expanded(false);
                    } else if (TreeNode *up = node->parentNode()) {
                        moveTo(&up->row());
                    }
                    return true;
                case Key::ENTER:
                case Key::KP_ENTER:
                    if (node->hasChildren()) {
                        node->toggle();
                    }
                    return true;
                default:
                    break;
            }
        }
        return RowGroup::handleRowKey(row, key, mod);
    }

    // endregion
}
