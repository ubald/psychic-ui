#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "psychic-ui/signals/Signal.hpp"
#include "Rows.hpp"

namespace psychic_ui {
    class DataGrid;
    class Icon;
    class Label;

    /**
     * The order a DataGrid's rows are sorted in by a column; `None` is the order they were added in
     */
    enum class SortOrder : std::uint8_t {
        None,
        Ascending,
        Descending
    };

    /**
     * A DataGrid's column: its title, its width (0 shares what the fixed ones leave), its alignment (`End` for
     * figures: their header's arrow goes before the title, which stays aligned with them) and whether clicking its
     * header sorts the rows by it
     */
    struct DataGridColumn {
        std::string title{};
        float       width{0};
        TextAlign   align{TextAlign::Start};
        bool        sortable{true};
    };

    /**
     * @class DataGridRow
     *
     * A row of a DataGrid: one `.cell` per column (with the `end` class for an `End` column), each showing a
     * `.value` label. A cell is a Div, so it can hold more than its text (`cell(2).add<Badge>(...)`).
     */
    class DataGridRow : public SelectableRow {
    public:
        explicit DataGridRow(const std::vector<std::string> &values = {});

        /**
         * The cell of a column (asserted in range)
         */
        Div &cell(std::size_t column) const;

        std::size_t cellCount() const { return _values.size(); }

        /**
         * A cell's text, which sorting compares ("" out of range)
         */
        const std::string &value(std::size_t column) const;

        /**
         * Set a cell's text, adding cells up to its column
         */
        template<class Self>
        Self &value(this Self &self, std::size_t column, const std::string &value) {
            static_cast<DataGridRow &>(self).updateValue(column, value);
            return self;
        }

    protected:
        friend class DataGrid;

        std::vector<Div *>   _cells{};  // Children, which we own
        std::vector<Label *> _values{}; // The cells' labels
        std::size_t          _order{0}; // Where it was added among the grid's rows, for `SortOrder::None`

        void updateValue(std::size_t column, const std::string &value);
        void addCell();
    };

    /**
     * @class DataGrid
     *
     * Rows of values in columns under a header row (`.header`, a tool bar's height, so that its line meets the bars
     * beside the grid), with a single selection (see RowGroup). The rows scroll inside (the `.body` part, with an
     * overlaid scroll bar) under the header, which stays in view. Clicking a sortable column's header sorts by it,
     * ascending, then descending, then back to the order the rows were added in; its `.sort-icon` arrow shows the
     * order (faintly, the next one, under the mouse); the title and the arrow are the header cell's
     * `.sort` button, which takes the focus (Enter or Space sorts). Sorting compares the cells' text, as numbers when both start
     * with one, otherwise without case.
     *
     * Its events: `onSelectionChange`/`selectionChanged` (not `onSelect`/`selected`, the rows' state getter) and
     * `onSortChange`/`sortChanged`.
     *
     *     auto &grid = add<DataGrid>().columns({{"Name"}, {.title = "Size", .width = 80, .align = TextAlign::End}});
     *     grid.row({"Cue 1", "12"});
     */
    class DataGrid : public RowGroup {
    public:
        DataGrid();

        /**
         * Replace the columns
         */
        template<class Self>
        Self &columns(this Self &self, std::vector<DataGridColumn> columns) {
            static_cast<DataGrid &>(self).updateColumns(std::move(columns));
            return self;
        }

        const std::vector<DataGridColumn> &columns() const { return _columns; }

        /**
         * Add a column
         */
        template<class Self>
        Self &column(this Self &self, DataGridColumn column) {
            auto &grid = static_cast<DataGrid &>(self);
            auto all   = grid._columns;
            all.push_back(std::move(column));
            grid.updateColumns(std::move(all));
            return self;
        }

        /**
         * Add a row of values, one per column, in the current sort
         */
        DataGridRow &row(const std::vector<std::string> &values);

        /**
         * The rows, in their current order
         */
        std::vector<DataGridRow *> dataRows() const;

        /**
         * The rows' container, which scrolls
         */
        Div &body() const { return *_body; }

        /**
         * The header row
         */
        Div &header() const { return *_header; }

        /**
         * The selected row, or nullptr
         */
        DataGridRow *selectedRow() const;

        int sortColumn() const { return _sortColumn; }
        SortOrder sortOrder() const { return _sortOrder; }

        /**
         * Sort the rows by a column (-1 or `SortOrder::None` for the order they were added in), emitting
         * `sortChanged` when it changes
         */
        template<class Self>
        Self &sort(this Self &self, int column, SortOrder order) {
            static_cast<DataGrid &>(self).updateSort(column, order);
            return self;
        }

        /**
         * The selection changed, to this row (nullptr for none)
         */
        Signal<DataGridRow *> selectionChanged{};
        PSYCHIC_UI_EVENT(onSelectionChange, selectionChanged)

        /**
         * The rows were sorted by this column (-1 for none), in this order
         */
        Signal<int, SortOrder> sortChanged{};
        PSYCHIC_UI_EVENT(onSortChange, sortChanged)

    protected:
        std::vector<DataGridColumn> _columns{};
        Div                         *_header{nullptr}; // Children, which we own
        Div                         *_body{nullptr};
        std::vector<Div *>          _sortButtons{}; // In the header's cells, nullptr for a column that doesn't sort
        int                         _sortColumn{-1};
        SortOrder                   _sortOrder{SortOrder::None};
        std::size_t                 _added{0}; // Rows added so far, for their order

        void updateColumns(std::vector<DataGridColumn> columns);
        void updateSort(int column, SortOrder order);
        void selectionUpdated() override;

        /**
         * The next sort when a column's header is clicked
         */
        void cycleSort(int column);

        /**
         * Size and align a cell as its column says
         */
        void styleCell(Div &cell, std::size_t column) const;

        /**
         * Put the rows in the current order
         */
        void applySort();
        void updateHeader();
    };
}
