#pragma once

#include <string>
#include <vector>
#include "psychic-ui/Div.hpp"
#include "Rows.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "WindowControls.hpp"

namespace psychic_ui {
    class Sidebar;

    /**
     * @class SidebarItem
     *
     * A row of a sidebar's navigation: an icon, a label and a count after it (parts `.icon`, `.label`, `.count`).
     * Clicking it (or Enter or Space while it has the focus) makes it its sidebar's current item (its selection, as
     * a SelectableRow), which has the `current` class; the arrows move the focus along the sidebar's items.
     *
     *     section.add<SidebarItem>("Inbox", "inbox").count("3").onClick([] { ... });
     */
    class SidebarItem : public SelectableRow {
    public:
        explicit SidebarItem(const std::string &label = "", const std::string &icon = "");

        std::string label() const { return _label->text(); }

        template<class Self>
        Self &label(this Self &self, const std::string &label) {
            static_cast<SidebarItem &>(self)._label->text(label);
            return self;
        }

        const std::string &icon() const { return _icon->name(); }

        /**
         * The icon before the label, by its registered name; "" for none
         */
        template<class Self>
        Self &icon(this Self &self, detail::StringArg icon) {
            static_cast<SidebarItem &>(self).updateIcon(icon.value);
            return self;
        }

        std::string count() const { return _count->text(); }

        /**
         * What is after the label, in small muted figures (a count, a shortcut); "" for nothing
         */
        template<class Self>
        Self &count(this Self &self, const std::string &count) {
            static_cast<SidebarItem &>(self).updateCount(count);
            return self;
        }

        /**
         * Whether it is its sidebar's current item
         */
        bool current() const { return _selected; }

        /**
         * Make it its sidebar's current item (the other one stops being it), or stop being it
         */
        template<class Self>
        Self &current(this Self &self, detail::BoolArg current) {
            static_cast<SidebarItem &>(self).updateSelected(current.value);
            return self;
        }

        /**
         * Whether clicking it makes it current (true): false for an item that acts instead of navigating
         * (opening a menu, a dialog), which only emits `clicked`
         */
        bool selectable() const { return _selectable; }

        template<class Self>
        Self &selectable(this Self &self, detail::BoolArg selectable) {
            static_cast<SidebarItem &>(self)._selectable = selectable.value;
            return self;
        }

        /**
         * The sidebar it is in, or nullptr
         */
        Sidebar *sidebar();

    protected:
        // Children: our child list owns them
        Icon  *_icon{nullptr};
        Label *_label{nullptr};
        Label *_count{nullptr};
        bool  _selectable{true};

        void updateIcon(std::string_view icon);
        void updateCount(const std::string &count);
        bool selectsOnClick() const override { return _selectable; }
        const char *selectionClass() const override { return "current"; }
    };

    /**
     * @class SidebarSection
     *
     * A group of a sidebar's items, under an optional heading (the `.heading` part). Sections after the first are
     * set apart by a hairline.
     */
    class SidebarSection : public Div {
    public:
        explicit SidebarSection(const std::string &heading = "");

        /**
         * Add an item: `add<SidebarItem>(label, icon)`
         */
        SidebarItem &item(const std::string &label, const std::string &icon = "") {
            return add<SidebarItem>(label, icon);
        }

    protected:
        Label *_heading{nullptr}; // A child: our child list owns it
    };

    /**
     * @class SidebarHeader
     *
     * The sidebar's top band: the window controls, then the app's mark and name (the `.brand` part, with `.mark`,
     * `.title` and `.chevron`). It is the window's drag area.
     */
    class SidebarHeader : public Div {
    public:
        SidebarHeader();

        WindowControls &controls() const { return *_controls; }

        std::string title() const { return _title->text(); }

        void setTitle(const std::string &title);
        void setMark(const std::string &mark);

    protected:
        // Children: our child list owns them
        WindowControls *_controls{nullptr};
        Div            *_brand{nullptr};
        Label          *_mark{nullptr};
        Label          *_title{nullptr};

        void addedToRender() override;
    };

    /**
     * @class Sidebar
     *
     * An app's navigation, the full height of the window on the left of an AppShell: its header (window controls and
     * the app's name, the drag area), then sections of items. One item at a time is current. It is a RowGroup whose
     * selection doesn't follow the focus: one item is in the Tab order (the current one, else the first), Up, Down,
     * Home and End move the focus along every section's items, Enter or Space clicks the focused one.
     *
     *     auto &sidebar = shell.sidebar().title("Lights").mark("L");
     *     auto &show    = sidebar.section("Show");
     *     show.item("Cues", "list-detail").count("42");
     *     sidebar.onChange([](SidebarItem *item) { ... });
     */
    class Sidebar : public RowGroup {
    public:
        Sidebar();

        SidebarHeader &header() const { return *_header; }

        /**
         * The app's name, in the header
         */
        template<class Self>
        Self &title(this Self &self, const std::string &title) {
            static_cast<Sidebar &>(self)._header->setTitle(title);
            return self;
        }

        /**
         * The app's mark: a letter or two on the accent, before its name
         */
        template<class Self>
        Self &mark(this Self &self, const std::string &mark) {
            static_cast<Sidebar &>(self)._header->setMark(mark);
            return self;
        }

        /**
         * Add a section of items, under an optional heading
         */
        SidebarSection &section(const std::string &heading = "");

        /**
         * Every item, in order
         */
        std::vector<SidebarItem *> items() const;

        /**
         * The current item, or nullptr
         */
        SidebarItem *current() const;

        // Make an item current (nullptr for none) with `select(item)`, emitting `changed` when it changes

        /**
         * The current item changed, to this one (nullptr for none)
         */
        Signal<SidebarItem *> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        SidebarHeader *_header{nullptr};   // A child: our child list owns it
        Div           *_sections{nullptr}; // A child: our child list owns it

        bool selectionFollowsFocus() const override { return false; }
        void selectionUpdated() override;
        bool handleRowKey(SelectableRow *row, Key key, Mod mod) override;
    };
}
