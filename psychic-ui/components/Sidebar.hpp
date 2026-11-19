#pragma once

#include <memory>
#include <string>
#include <string_view>
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
     * Items can be nested under it (`item(label, icon)`): they go in its `.children` part, right after it, shown
     * while it is expanded (the `expanded` class); its `.disclosure` chevron, at the end of the row, expands or
     * collapses it. While one of them is current, it has the `has-current` class, and it expands when one becomes
     * current.
     *
     *     section.add<SidebarItem>("Inbox", "inbox").count("3").onClick([] { ... });
     *     auto &session = section.item("Session", "layers");
     *     session.item("Bank 1");
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

        /**
         * Add an item nested under it, at the end of its `.children` part
         */
        SidebarItem &item(const std::string &label, const std::string &icon = "");

        /**
         * The items nested right under it, in order
         */
        std::vector<SidebarItem *> items() const;

        /**
         * The item it is nested under, or nullptr
         */
        SidebarItem *parentItem() const { return _parentItem; }

        /**
         * Whether its nested items show
         */
        bool expanded() const { return _expanded; }

        /**
         * Show or hide its nested items (the `expanded` class); a current one stays current
         */
        template<class Self>
        Self &expanded(this Self &self, detail::BoolArg expanded) {
            static_cast<SidebarItem &>(self).updateExpanded(expanded.value);
            return self;
        }

        /**
         * Expand it if it is collapsed, and collapse it if it is expanded
         */
        void toggle() { updateExpanded(!_expanded); }

    protected:
        // Children: our child list owns them
        Icon  *_icon{nullptr};
        Label *_label{nullptr};
        Label *_count{nullptr};
        Icon  *_disclosure{nullptr};
        bool  _selectable{true};
        bool  _expanded{false};

        /**
         * The `.children` part, made with the first nested item: in our parent, right after us, and ours too, so
         * that it follows us when we move
         */
        std::shared_ptr<Div> _children{};
        SidebarItem          *_parentItem{nullptr};

        void updateIcon(std::string_view icon);
        void updateCount(const std::string &count);
        void updateExpanded(bool expanded);

        /**
         * Whether a click is on the chevron, which expands or collapses it instead of making it current
         */
        bool onDisclosure(const MouseEvent &click) const;
        bool selectsOnClick(const MouseEvent &click) const override { return _selectable && !onDisclosure(click); }
        const char *selectionClass() const override { return "current"; }

        /**
         * Put the `.children` part right after us in our parent, if we have one
         */
        void placeChildren();
        void added() override;

        /**
         * Take the `.children` part out of our former parent
         */
        void removed() override;
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
     * The sidebar's top band, the window's drag area: the window controls (`windowControls(false)` leaves them out,
     * for an app that puts them elsewhere), then the `.identity` column: the `.brand` row (the app's `.mark`, its
     * `.title` and a `.chevron`), and the `.details` under it. What the app adds to `mark()` takes the letters'
     * place (its own drawing), what it adds to `details()` goes under the brand (a document's name, a place), and
     * the brand becomes a menu trigger once it has a `clicked` subscriber (which the drag area leaves to it).
     *
     *     auto &header = sidebar.header().windowControls(false);
     *     header.mark().add<Logo>();
     *     header.details().add<Label>("Summer Tour");
     *     header.brand().onClick([this] { openAppMenu(); });
     */
    class SidebarHeader : public Div {
    public:
        SidebarHeader();

        /**
         * The window controls, in the header unless `windowControls(false)`
         */
        WindowControls &controls() const { return *_controls; }

        /**
         * Whether the window controls are in the header (they are at first)
         */
        bool windowControls() const;

        /**
         * Put the window controls first in the header, or take them out of it
         */
        template<class Self>
        Self &windowControls(this Self &self, detail::BoolArg shown) {
            static_cast<SidebarHeader &>(self).updateWindowControls(shown.value);
            return self;
        }

        /**
         * The `.brand` part: the app's mark, its name and a chevron
         */
        Div &brand() const { return *_brand; }

        /**
         * The `.mark` part, first in the brand, made when first set or asked for: the letters `setMark` puts in it,
         * or what the app adds to it in their place
         */
        Div &mark();

        /**
         * The `.details` part, under the brand, made when first asked for: what the app adds to it
         */
        Div &details();

        std::string title() const { return _title->text(); }

        void setTitle(const std::string &title);

        /**
         * A letter or two on the accent, in the `.mark` part; "" for none (the part goes when nothing else is in it)
         */
        void setMark(const std::string &mark);

    protected:
        std::shared_ptr<WindowControls> _controls{}; // Ours, and our child while shown
        // Children: our child list owns them
        Div   *_identity{nullptr};
        Div   *_brand{nullptr};
        Div   *_mark{nullptr};
        Label *_letters{nullptr};
        Label *_title{nullptr};
        Div   *_details{nullptr};

        void updateWindowControls(bool shown);
        void addedToRender() override;
    };

    /**
     * @class Sidebar
     *
     * An app's navigation, the full height of the window on the left of an AppShell: its header (window controls and
     * the app's name, the drag area), then sections of items, which may hold nested items. One item at a time is
     * current. It is a RowGroup whose
     * selection doesn't follow the focus: one item is in the Tab order (the current one, else the first), Up, Down,
     * Home and End move the focus along every section's items (nested ones too, unless their parent is collapsed),
     * Enter or Space clicks the focused one, Right and Left expand and collapse.
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
         * Every item, in order, nested ones after their parent
         */
        std::vector<SidebarItem *> items() const;

        /**
         * The items the keyboard moves along: those not hidden in a collapsed parent
         */
        std::vector<SelectableRow *> rows() const override;

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
        friend class SidebarItem;

        SidebarHeader *_header{nullptr};   // A child: our child list owns it
        Div           *_sections{nullptr}; // A child: our child list owns it

        bool selectionFollowsFocus() const override { return false; }

        /**
         * Expand the current item's parents and mark them (`has-current`), then emit `changed`
         */
        void selectionUpdated() override;
        SelectableRow *restingTabStop() const override;

        /**
         * Enter or Space clicks the focused item; Right expands a collapsed parent, Left collapses an expanded one
         * or moves to the parent of a nested item
         */
        bool handleRowKey(SelectableRow *row, Key key, Mod mod) override;

        /**
         * An item collapsed: the focus and the Tab stop can't stay hidden in it
         */
        void collapsed(SidebarItem &item);
    };
}
