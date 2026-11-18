#pragma once

#include <vector>
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    class RowGroup;

    /**
     * @class SelectableRow
     *
     * A row a RowGroup (List, DataGrid, Tree) selects: the `selected` class while it is its group's selection.
     * Clicking it selects it. Only one row of a group is in the Tab order at a time (the selection, else the first
     * row): Tab enters the group once, then the arrows move along it.
     */
    class SelectableRow : public Div {
    public:
        SelectableRow();
        ~SelectableRow();

        /**
         * Whether it is its group's selection
         */
        bool selected() const { return _selected; }

        /**
         * Select it in its group (the previous selection stops being it), or stop being the selection
         */
        template<class Self>
        Self &selected(this Self &self, detail::BoolArg selected) {
            static_cast<SelectableRow &>(self).updateSelected(selected.value);
            return self;
        }

        /**
         * The group it is in, or nullptr
         */
        RowGroup *group() const;

    protected:
        friend class RowGroup;

        bool     _selected{false};
        RowGroup *_owner{nullptr}; // The group it joined, while it is in it

        void updateSelected(bool selected);
        void added() override;

        /**
         * Whether a click selects it (true): a row that acts instead (a sidebar item opening a menu) says no
         */
        virtual bool selectsOnClick() const { return true; }

        /**
         * The class it has while it is the selection: `selected` (a sidebar's item says `current`)
         */
        virtual const char *selectionClass() const { return "selected"; }
        void removed() override;

        /**
         * Only the state and the class: the group keeps a single selection
         */
        void setSelected(bool selected);
    };

    /**
     * @class RowGroup
     *
     * What List, DataGrid, Tree and Sidebar share: a single selection among their rows (SelectableRow descendants),
     * and the keyboard, with a single row in the Tab order (the selection, else the first). Up and Down move the
     * focus and the selection to the previous or next row, Home and End to the first and last, Space selects the
     * focused row; the row moved to is scrolled into view in the nearest scrolling ancestor. A group whose
     * selection doesn't follow the focus (`selectionFollowsFocus`, a sidebar) only moves the focus, and its Tab stop
     * goes back to the selection once the focus leaves it. A subclass emits its own typed signal from
     * `selectionUpdated` and may handle more keys (`handleRowKey`).
     */
    class RowGroup : public Div {
    public:
        RowGroup();
        ~RowGroup();

        /**
         * The rows the keyboard moves along, in order: every row, but a tree leaves out the collapsed ones
         */
        virtual std::vector<SelectableRow *> rows() const;

        /**
         * The selected row, or nullptr
         */
        SelectableRow *selection() const { return _selection; }

        /**
         * The selection's index in `rows()`, or -1
         */
        int selectedIndex() const;

        /**
         * Select a row (nullptr for none), emitting the subclass's selection signal when it changes
         */
        template<class Self>
        Self &select(this Self &self, SelectableRow *row) {
            static_cast<RowGroup &>(self).updateSelection(row);
            return self;
        }

        /**
         * Select the row at an index of `rows()` (none when out of range)
         */
        template<class Self>
        Self &select(this Self &self, int index) {
            static_cast<RowGroup &>(self).updateSelection(index);
            return self;
        }

    protected:
        friend class SelectableRow;

        SelectableRow *_selection{nullptr}; // One of our descendants
        SelectableRow *_tabStop{nullptr};   // The row in the Tab order, one of our descendants
        bool          _reordering{false};  // Rows leave and come back (a sort): they stay selected

        void updateSelection(SelectableRow *row);
        void updateSelection(int index);

        /**
         * Whether moving along the rows with the keyboard selects them (true); a sidebar only moves the focus
         */
        virtual bool selectionFollowsFocus() const { return true; }

        /**
         * The selection changed (to `selection()`): emit the subclass's signal
         */
        virtual void selectionUpdated() {}

        /**
         * A key pressed while the focus is in the group, on `row` (the focused row, else the selection; may be
         * nullptr). Return whether it was handled.
         */
        virtual bool handleRowKey(SelectableRow *row, Key key, Mod mod);

        /**
         * Focus a row (as the keyboard does: with a visible ring), select it (when the selection follows the focus)
         * and scroll it into view
         */
        void moveTo(SelectableRow *row);

        /**
         * A row joined the group: the first one is the Tab stop until there is a selection. A subclass that adds
         * rows inside containers already built calls it for them (they joined nothing when they were added).
         */
        void rowAdded(SelectableRow *row);

        /**
         * Put a single row in the Tab order: this one, else the selection, else the first row
         */
        void updateTabStop(SelectableRow *row = nullptr);

        /**
         * A row is going away: it can't stay selected
         */
        void forget(SelectableRow *row);

        /**
         * Put rows back in a container in a new order (the same rows), keeping the selection and the focus
         */
        void reorder(Div &container, const std::vector<std::shared_ptr<Div>> &children);
    };
}
