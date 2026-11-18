#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <utility>
#include "DataGrid.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "ScrollBar.hpp"

namespace psychic_ui {
    namespace {
        /**
         * The number a text starts with, if it does
         */
        bool leadingNumber(const std::string &text, double &number) {
            const char *start = text.c_str();
            char       *end   = nullptr; // NOLINT(misc-const-correctness): std::strtod's end is a char **
            number = std::strtod(start, &end);
            return end != start;
        }

        /**
         * Negative, zero or positive as a sorts before, with or after b
         */
        int compareValues(const std::string &a, const std::string &b) {
            double na = 0;
            double nb = 0;
            if (leadingNumber(a, na) && leadingNumber(b, nb) && na != nb) {
                return na < nb ? -1 : 1;
            }
            const auto length = std::min(a.size(), b.size());
            for (std::size_t i = 0; i < length; ++i) {
                const int ca = std::tolower(static_cast<unsigned char>(a[i]));
                const int cb = std::tolower(static_cast<unsigned char>(b[i]));
                if (ca != cb) {
                    return ca < cb ? -1 : 1;
                }
            }
            if (a.size() == b.size()) {
                return 0;
            }
            return a.size() < b.size() ? -1 : 1;
        }
    }

    // region Row

    DataGridRow::DataGridRow(const std::vector<std::string> &values) :
        SelectableRow() {
        tag("DataGridRow");
        _mouseChildren = false; // The row takes the presses, not its cells
        for (std::size_t i = 0; i < values.size(); ++i) {
            updateValue(i, values[i]);
        }
    }

    Div &DataGridRow::cell(const std::size_t column) const {
        return *_cells.at(column);
    }

    std::string DataGridRow::value(const std::size_t column) const {
        return column < _values.size() ? _values[column]->text() : std::string{};
    }

    void DataGridRow::addCell() {
        auto &cell = add<Div>();
        cell.addClassName("cell");
        auto &value = cell.add<Label>();
        value.addClassName("value");
        _cells.push_back(&cell);
        _values.push_back(&value);
    }

    void DataGridRow::updateValue(const std::size_t column, const std::string &value) {
        while (_cells.size() <= column) {
            addCell();
        }
        _values[column]->text(value);
    }

    // endregion

    // region Grid

    DataGrid::DataGrid() :
        RowGroup() {
        tag("DataGrid");
        _defaultStyle.direction(Direction::Column);

        _header = &add<Div>();
        _header->addClassName("header")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .shrink(0);

        // The rows scroll under the header, with a scroll bar over their right edge, so that they keep the
        // header's width and their cells stay under its titles
        auto &viewport = add<Div>();
        viewport.addClassName("viewport")
                .position(Position::Relative)
                .grow(1)
                .shrink(1)
                .basis(0);
        _body = &viewport.add<Div>();
        _body->addClassName("body")
             .direction(Direction::Column)
             .width(100_pct)
             .height(100_pct)
             .overflow(Overflow::Scroll);
        viewport.add<ScrollBar>(*_body)
                .position(Position::Absolute)
                .top(0)
                .right(0)
                .bottom(0);
    }

    DataGridRow &DataGrid::row(const std::vector<std::string> &values) {
        auto &row = _body->add<DataGridRow>(values);
        row._order = _added++;
        for (std::size_t i = 0; i < row.cellCount(); ++i) {
            styleCell(row.cell(i), i);
        }
        if (_sortOrder != SortOrder::None) {
            applySort();
        }
        return row;
    }

    std::vector<DataGridRow *> DataGrid::dataRows() const {
        std::vector<DataGridRow *> rows{};
        for (const auto &child: _body->children()) {
            if (auto row = std::dynamic_pointer_cast<DataGridRow>(child)) {
                rows.push_back(row.get());
            }
        }
        return rows;
    }

    DataGridRow *DataGrid::selectedRow() const {
        return dynamic_cast<DataGridRow *>(_selection);
    }

    void DataGrid::selectionUpdated() {
        selectionChanged.emit(selectedRow());
    }

    void DataGrid::styleCell(Div &cell, const std::size_t column) const {
        if (column >= _columns.size()) {
            return;
        }
        const DataGridColumn &c = _columns[column];
        cell.direction(Direction::Row)
            .alignItems(Align::Center)
            .minWidth(0)
            .justify(c.align == TextAlign::End ? Justify::End : Justify::Start);
        if (c.width > 0) {
            cell.width(c.width).grow(0).shrink(0);
        } else {
            cell.basis(0).grow(1).shrink(1);
        }
        if (c.align == TextAlign::End) {
            cell.addClassName("end");
        } else {
            cell.removeClassName("end");
        }
    }

    void DataGrid::updateColumns(std::vector<DataGridColumn> columns) {
        _columns = std::move(columns);
        if (std::cmp_greater_equal(_sortColumn, _columns.size())) {
            _sortColumn = -1;
            _sortOrder  = SortOrder::None;
        }

        _header->removeAll();
        _sortButtons.clear();
        for (std::size_t i = 0; i < _columns.size(); ++i) {
            const DataGridColumn &column = _columns[i];
            auto &cell = _header->add<Div>();
            cell.addClassName("cell");
            styleCell(cell, i);
            if (!column.sortable) {
                cell.add<Label>(column.title).addClassName("title");
                _sortButtons.push_back(nullptr);
                continue;
            }
            // A button as wide as its title and arrow, so that the pointer and the focus ring fit them
            auto &sort = cell.add<Div>();
            sort.addClassName("sort")
                .direction(column.align == TextAlign::End ? Direction::RowReverse : Direction::Row) // The arrow first
                .alignItems(Align::Center)
                .shrink(1)
                .minWidth(0)
                .mouseChildren(false)
                .focusEnabled(true);
            sort.add<Label>(column.title).addClassName("title");
            sort.add<Icon>("arrow-up").addClassName("sort-icon");
            const int index = static_cast<int>(i);
            sort.onClick([this, index]() { cycleSort(index); });
            sort.onKeyPress(
                [this, index](const Key key, Mod) {
                    if (key == Key::ENTER || key == Key::KP_ENTER || key == Key::SPACE) {
                        cycleSort(index);
                    }
                }
            );
            _sortButtons.push_back(&sort);
        }
        for (const DataGridRow *row: dataRows()) {
            for (std::size_t i = 0; i < row->cellCount(); ++i) {
                styleCell(row->cell(i), i);
            }
        }
        updateHeader();
    }

    void DataGrid::cycleSort(const int column) {
        if (column != _sortColumn || _sortOrder == SortOrder::None) {
            updateSort(column, SortOrder::Ascending);
        } else if (_sortOrder == SortOrder::Ascending) {
            updateSort(column, SortOrder::Descending);
        } else {
            updateSort(-1, SortOrder::None);
        }
    }

    void DataGrid::updateSort(int column, SortOrder order) {
        if (column < 0 || std::cmp_greater_equal(column, _columns.size()) || order == SortOrder::None) {
            column = -1;
            order  = SortOrder::None;
        }
        if (column == _sortColumn && order == _sortOrder) {
            return;
        }
        _sortColumn = column;
        _sortOrder  = order;
        applySort();
        updateHeader();
        sortChanged.emit(_sortColumn, _sortOrder);
    }

    void DataGrid::applySort() {
        std::vector<std::shared_ptr<Div>> others{};
        std::vector<std::shared_ptr<DataGridRow>> rows{};
        for (const auto &child: _body->children()) {
            if (auto row = std::dynamic_pointer_cast<DataGridRow>(child)) {
                rows.push_back(row);
            } else {
                others.push_back(child);
            }
        }
        const auto column = static_cast<std::size_t>(std::max(_sortColumn, 0));
        std::ranges::stable_sort(
            rows, [this, column](const auto &a, const auto &b) {
                if (_sortOrder == SortOrder::None) {
                    return a->_order < b->_order;
                }
                const int order = compareValues(a->value(column), b->value(column));
                if (order == 0) {
                    return a->_order < b->_order;
                }
                return _sortOrder == SortOrder::Ascending ? order < 0 : order > 0;
            }
        );
        std::vector<std::shared_ptr<Div>> children{rows.begin(), rows.end()};
        children.insert(children.end(), others.begin(), others.end());
        if (children != _body->children()) {
            reorder(*_body, children);
        }
    }

    void DataGrid::updateHeader() {
        for (std::size_t i = 0; i < _sortButtons.size(); ++i) {
            if (!_sortButtons[i]) {
                continue;
            }
            Div        &sort   = *_sortButtons[i];
            const bool sorted = std::cmp_equal(i, _sortColumn);
            if (sorted) {
                sort.addClassName("sorted");
            } else {
                sort.removeClassName("sorted");
            }
            for (const auto &child: sort.children()) {
                if (auto icon = std::dynamic_pointer_cast<Icon>(child)) {
                    // The current order, else the one a click gives
                    icon->name(sorted && _sortOrder == SortOrder::Descending ? "arrow-down" : "arrow-up");
                }
            }
        }
    }

    // endregion
}
