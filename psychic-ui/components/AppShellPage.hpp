#pragma once

#include <memory>
#include <vector>
#include "psychic-ui/Div.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "PageHeader.hpp"

namespace psychic_ui {

    /**
     * @class AppShellPage
     *
     * An `AppShell`'s page panel (its `.page` part, `AppShell::page()`): a column that keeps track of the
     * `PageHeader`s among its children, so that the shell knows whether the page has a header to carry the window
     * controls.
     */
    class AppShellPage : public Div {
    public:
        AppShellPage();

        /**
         * Whether a `PageHeader` is one of its children
         */
        bool hasHeader() const { return !_headers.empty(); }

        /**
         * A `PageHeader` became its first, or its last one left, with whether it has one
         */
        Signal<bool> headerChanged{};
        PSYCHIC_UI_EVENT(onHeaderChange, headerChanged)

    protected:
        std::vector<std::shared_ptr<PageHeader>> _headers{}; // Its children that are page headers, in order

        void childAdded(Div &child) override;
        void childRemoved(Div &child) override;
    };
}
