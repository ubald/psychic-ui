#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "psychic-ui/Overlay.hpp"
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
     * Short news in the toasts' stack (Pivotal's): an icon on a tile in its variant's color (`.icon`), a title
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
         * Take it out of its stack
         */
        void dismiss();

        /**
         * After it left its stack
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
    };

    /**
     * @class ToastStack
     *
     * The corner the toasts stack in, bottom right in the overlay: the newest at the bottom, at most `limit` (adding
     * one more closes the oldest). Non-modal: it only takes the mouse on the toasts. The window makes it
     * (`Window::toasts()`).
     */
    class ToastStack : public Popup {
    public:
        ToastStack();

        static constexpr std::size_t limit = 3;

        Toast &add(const ToastOptions &options);

        std::vector<Toast *> toasts() const;

    protected:
        void update(std::chrono::nanoseconds now) override;
    };
}
