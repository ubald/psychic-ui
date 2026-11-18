#pragma once

#include <string>
#include <vector>
#include "psychic-ui/signals/Signal.hpp"
#include "Rows.hpp"

namespace psychic_ui {
    class Icon;
    class Label;
    class Tree;
    class TreeNode;

    /**
     * @class TreeRow
     *
     * A tree node's row, as the sidebar's: an `.indent` a step per level, the `.disclosure` chevron (its column kept
     * on leaves, so that icons line up across levels), the node's `.icon`, its `.label` and `.trailing` text
     */
    class TreeRow : public SelectableRow {
    public:
        TreeRow();

        TreeNode *node() const;

    protected:
        friend class TreeNode;

        // Children: our child list owns them
        Div   *_indent{nullptr};
        Icon  *_disclosure{nullptr};
        Icon  *_icon{nullptr};
        Label *_label{nullptr};
        Label *_trailing{nullptr};
    };

    /**
     * @class TreeNode
     *
     * A node of a Tree: its row, then its child nodes (the `.children` part), shown while it is expanded.
     * Clicking its chevron, or double clicking its row, expands or collapses it.
     */
    class TreeNode : public Div {
    public:
        explicit TreeNode(const std::string &label = "", const std::string &icon = "");

        /**
         * Add a child node
         */
        TreeNode &node(const std::string &label, const std::string &icon = "");

        /**
         * The child nodes, in order
         */
        std::vector<TreeNode *> nodes() const;

        /**
         * The node it is a child of, or nullptr at the top of the tree
         */
        TreeNode *parentNode() const;

        /**
         * The tree it is in, or nullptr
         */
        Tree *tree() const;

        /**
         * How deep it is: 0 at the top
         */
        int level() const { return _level; }

        TreeRow &row() const { return *_row; }

        const std::string &label() const;

        template<class Self>
        Self &label(this Self &self, const std::string &label) {
            static_cast<TreeNode &>(self).updateLabel(label);
            return self;
        }

        /**
         * The icon after the chevron, by its registered name; "" for none
         */
        template<class Self>
        Self &icon(this Self &self, detail::StringArg icon) {
            static_cast<TreeNode &>(self).updateIcon(icon.value);
            return self;
        }

        /**
         * Small muted text at the end of the row (a count); "" for none
         */
        template<class Self>
        Self &trailing(this Self &self, const std::string &trailing) {
            static_cast<TreeNode &>(self).updateTrailing(trailing);
            return self;
        }

        bool expanded() const { return _expanded; }

        /**
         * Show or hide its children (the `expanded` class), emitting the tree's `expansionChanged` when it changes
         */
        template<class Self>
        Self &expanded(this Self &self, detail::BoolArg expanded) {
            static_cast<TreeNode &>(self).updateExpanded(expanded.value);
            return self;
        }

        /**
         * Expand it if it is collapsed, and collapse it if it is expanded
         */
        void toggle() { updateExpanded(!_expanded); }

        bool hasChildren() const;

    protected:
        friend class Tree;

        TreeRow *_row{nullptr};      // Children: our child list owns them
        Div     *_children{nullptr};
        int     _level{0};
        bool    _expanded{false};

        void updateLabel(const std::string &label);
        void updateIcon(std::string_view icon);
        void updateTrailing(const std::string &trailing);
        void updateExpanded(bool expanded);

        /**
         * Show the chevron of its state, or none for a leaf
         */
        void updateDisclosure();

        /**
         * Take its level from its parent node, then its children theirs, and join the tree with its rows
         */
        void added() override;
        void updateLevel(float indent);
    };

    /**
     * @class Tree
     *
     * Nested rows that expand and collapse, with a single selection (see RowGroup). The keyboard follows WAI-ARIA's
     * tree view: Up and Down move along the visible rows, Home and End to the first and last; Right expands a
     * collapsed node, or moves to an expanded one's first child; Left collapses an expanded node, or moves to its
     * parent; Enter expands or collapses. Collapsing a node that holds the selection selects it.
     *
     * Its events: `onSelectionChange`/`selectionChanged` (not `onSelect`/`selected`, the rows' state getter) and
     * `onExpansionChange`/`expansionChanged` (not `onExpand`/`expanded`, the nodes' state getter).
     *
     *     auto &src = tree.node("src", "folder").expanded(true);
     *     src.node("main.cpp", "file");
     */
    class Tree : public RowGroup {
    public:
        Tree();

        /**
         * Add a node at the top
         */
        TreeNode &node(const std::string &label, const std::string &icon = "");

        /**
         * The nodes at the top, in order
         */
        std::vector<TreeNode *> nodes() const;

        /**
         * The visible rows: those of the expanded nodes' children
         */
        std::vector<SelectableRow *> rows() const override;

        /**
         * The selected node, or nullptr
         */
        TreeNode *selectedNode() const;

        float indent() const { return _indent; }

        /**
         * How far each level is indented, in window units (16, Pivotal's `--space-4`)
         */
        template<class Self>
        Self &indent(this Self &self, detail::FloatArg indent) {
            static_cast<Tree &>(self).updateIndent(indent.value);
            return self;
        }

        /**
         * The selection changed, to this node (nullptr for none)
         */
        Signal<TreeNode *> selectionChanged{};
        PSYCHIC_UI_EVENT(onSelectionChange, selectionChanged)

        /**
         * A node was expanded (true) or collapsed (false)
         */
        Signal<TreeNode *, bool> expansionChanged{};
        PSYCHIC_UI_EVENT(onExpansionChange, expansionChanged)

    protected:
        friend class TreeNode;

        float _indent{16};

        void selectionUpdated() override;
        bool handleRowKey(SelectableRow *row, Key key, Mod mod) override;
        void updateIndent(float indent);

        /**
         * A node joined the tree: its rows join the group
         */
        void adopt(TreeNode &node);

        /**
         * A node collapsed: the selection, and the focus, can't stay hidden in it
         */
        void collapsed(TreeNode &node);
    };
}
