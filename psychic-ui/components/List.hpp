#pragma once

#include <string>
#include "psychic-ui/signals/Signal.hpp"
#include "Rows.hpp"
#include "Label.hpp"

namespace psychic_ui {
    class Avatar;
    class Badge;
    class Kbd;

    /**
     * @class ListRow
     *
     * A row of a List: what leads it (the `.leading` part: an icon or an avatar), its text (`.text`: the `.title`
     * and, under it, the `.secondary` text), then what trails it (`.trailing`: `.meta` text, badges, keyboard
     * hints, anything added to `trailing()`). Parts are made when first set. Hovering raises it; the selection is
     * raised with an accent bar at its start.
     *
     *     list.row("Ada Lovelace", "ada@example.com").avatar("Ada Lovelace").meta("2h");
     */
    class ListRow : public SelectableRow {
    public:
        explicit ListRow(const std::string &title = "", const std::string &secondary = "");

        std::string title() const { return _title->text(); }

        template<class Self>
        Self &title(this Self &self, const std::string &title) {
            static_cast<ListRow &>(self)._title->text(title);
            return self;
        }

        std::string secondary() const { return _secondary->text(); }

        /**
         * The text under the title, smaller and muted; "" for none
         */
        template<class Self>
        Self &secondary(this Self &self, const std::string &secondary) {
            static_cast<ListRow &>(self).updateSecondary(secondary);
            return self;
        }

        /**
         * Lead with an icon, by its registered name (replacing what led it)
         */
        template<class Self>
        Self &icon(this Self &self, detail::StringArg icon) {
            static_cast<ListRow &>(self).updateIcon(icon.value);
            return self;
        }

        /**
         * Lead with someone's avatar (replacing what led it)
         */
        template<class Self>
        Self &avatar(this Self &self, const std::string &name) {
            static_cast<ListRow &>(self).updateAvatar(name);
            return self;
        }

        /**
         * Trail with small muted text (a time, a count, a size), before what else trails it
         */
        template<class Self>
        Self &meta(this Self &self, const std::string &meta) {
            static_cast<ListRow &>(self).updateMeta(meta);
            return self;
        }

        /**
         * The `.leading` part, for anything else to lead the row
         */
        Div &leading();

        /**
         * The `.trailing` part: `row.trailing().add<Badge>("New", Tone::Accent)`
         */
        Div &trailing();

    protected:
        // Children: our child list owns them; the leading and trailing parts are made when first needed
        Div   *_leading{nullptr};
        Label *_title{nullptr};
        Label *_secondary{nullptr};
        Div   *_trailing{nullptr};
        Label *_meta{nullptr};

        void updateSecondary(const std::string &secondary);
        void updateIcon(std::string_view icon);
        void updateAvatar(const std::string &name);
        void updateMeta(const std::string &meta);
    };

    /**
     * @class List
     *
     * Rows separated by hairlines, edge to edge on the page's gutter, with a single selection (see RowGroup): click a
     * row or move with Up, Down, Home and End. Rows are ListRows (`row(...)`), or any SelectableRow.
     *
     * Its event is `onSelectionChange`/`selectionChanged`, not `onSelect`/`selected`: `selected` is the rows' state
     * getter, and the name follows `Text`'s `selectionChanged`.
     *
     *     auto &list = add<List>();
     *     list.row("Inbox", "3 unread").icon("inbox");
     *     list.onSelectionChange([](ListRow *row) { ... });
     */
    class List : public RowGroup {
    public:
        List();

        /**
         * Add a row: `add<ListRow>(title, secondary)`
         */
        ListRow &row(const std::string &title, const std::string &secondary = "") {
            return add<ListRow>(title, secondary);
        }

        /**
         * The selected row if it is a ListRow, or nullptr
         */
        ListRow *selectedRow() const;

        /**
         * The selection changed, to this row (nullptr for none; a row that isn't a ListRow is in `selection()`)
         */
        Signal<ListRow *> selectionChanged{};
        PSYCHIC_UI_EVENT(onSelectionChange, selectionChanged)

    protected:
        void selectionUpdated() override;
    };
}
