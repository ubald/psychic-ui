#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "psychic-ui/Popup.hpp"
#include "Button.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "Text.hpp"

namespace psychic_ui {

    /**
     * What kind of news a toast brings: its icon's tile and color
     */
    enum class ToastVariant : std::uint8_t { Info, Success, Warning, Danger };

    /**
     * A button along a toast's bottom: answering, opening, putting off
     */
    struct ToastAction {
        std::string           label{};
        std::function<void()> callback{nullptr};
        bool                  primary{false};  // The one most likely; secondary otherwise
        bool                  keepOpen{false}; // The toast stays after the click; it closes by default
    };

    /**
     * What a toast says and how long it stays
     */
    struct ToastOptions {
        std::string              title{};
        std::string              description{};
        ToastVariant             variant{ToastVariant::Info};
        std::string              icon{};         // On a tile before the text; "" for the variant's own
        bool                     showIcon{true};
        std::chrono::nanoseconds timeout{std::chrono::seconds(5)}; // 0 keeps it until closed
        std::vector<ToastAction> actions{};
    };

    class ToastStack;

    /**
     * @class Toast
     *
     * Short news in the toasts' stack (Pivotal's): an icon on a tile in its variant's color (`.icon-tile`), a title
     * (`.title`) and a description (`.description`) with action buttons under them (`.actions`), and a close button
     * (`.close`). Its variant is a class: `info`, `success`, `warning` or `danger`. It closes after its timeout, which
     * doesn't run while the mouse is over the stack, when one of its actions is clicked, or with its close button.
     * Show one with `Window::toast(options)`.
     */
    class Toast : public Div {
    public:
        explicit Toast(const ToastOptions &options);

        const ToastOptions &options() const { return _options; }

        /**
         * How long it has been showing, the time the mouse was over the stack left out
         */
        std::chrono::nanoseconds elapsed() const { return _elapsed; }

        /**
         * Take it out of its stack: it leaves once its exit transitions (the `exiting` class) are over, and no longer
         * counts meanwhile (`ToastStack::toasts`, its limit). It leaves the stack's flow at once, where it is (absolute),
         * so that the others slide into its place as it goes (the stack's `layoutTransition`); one with a toast still
         * under it also has the `in-place` class (it fades where it is rather than sliding over that one).
         */
        void dismiss();

        /**
         * When it was dismissed, as it starts leaving its stack
         */
        Signal<> dismissed{};

        PSYCHIC_UI_EVENT(onDismiss, dismissed)

    protected:
        friend class ToastStack;

        ToastOptions                            _options{};
        std::chrono::nanoseconds                _elapsed{0};
        std::optional<std::chrono::nanoseconds> _last{};

        /**
         * Advance its timer to `now`; paused, the time passes without counting
         */
        void advance(std::chrono::nanoseconds now, bool paused);

        /**
         * When its timeout runs out, if it isn't paused
         */
        std::optional<std::chrono::nanoseconds> deadline() const;
    };

    /**
     * @class ToastStack
     *
     * The corner the toasts stack in, bottom right in the overlay: the newest at the bottom, at most `limit` (adding
     * one more closes the oldest, with the `limited` class, and `in-place`). Non-modal: it only takes the mouse on the toasts. The
     * window makes it (`Window::toasts()`). A toast enters with the `entering` class (`Div::startingClass`) and
     * leaves with `exiting`; those leaving aren't in `toasts()`.
     */
    class ToastStack : public Popup {
    public:
        ToastStack();

        static constexpr std::size_t limit = 3;

        Toast &add(const ToastOptions &options);

        /**
         * The toasts shown, oldest first (not those leaving)
         */
        std::vector<Toast *> toasts() const;

        std::optional<std::chrono::nanoseconds> nextUpdate() const override;

    protected:
        std::vector<std::shared_ptr<Toast>> _toasts{}; // Its children, typed (`childAdded`), oldest first, those leaving too
        bool                                _paused{false}; // The mouse was over the stack at the last update

        void update(std::chrono::nanoseconds now) override;
        void childAdded(Div &child) override;
        void childRemoved(Div &child) override;
    };
}
