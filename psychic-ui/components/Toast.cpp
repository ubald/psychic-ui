#include <algorithm>
#include "Toast.hpp"

namespace psychic_ui {

    namespace {
        const char *variantName(const ToastVariant variant) {
            switch (variant) {
                case ToastVariant::Info: return "info";
                case ToastVariant::Success: return "success";
                case ToastVariant::Warning: return "warning";
                case ToastVariant::Danger: return "danger";
            }
            return "info";
        }

        const char *variantIcon(const ToastVariant variant) {
            switch (variant) {
                case ToastVariant::Info: return "info";
                case ToastVariant::Success: return "circle-check";
                case ToastVariant::Warning: return "warning";
                case ToastVariant::Danger: return "circle-alert";
            }
            return "info";
        }
    }

    // region Toast

    Toast::Toast(const ToastOptions &options) :
        Div(),
        _options(options) {
        tag("Toast");
        startingClass("entering"); // Its rules say where it slides in from
        addClassName(variantName(options.variant));
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Start);

        if (options.showIcon) {
            auto &tile = add<Div>();
            tile.addClassName("icon-tile")
                .alignItems(Align::Center)
                .justify(Justify::Center)
                .shrink(0);
            tile.add<Icon>(options.icon.empty() ? variantIcon(options.variant) : options.icon);
        }

        auto &text = add<Div>();
        text.addClassName("text")
            .direction(Direction::Column)
            .grow(1)
            .shrink(1)
            .basis(0);
        text.add<Label>(options.title).addClassName("title");
        if (!options.description.empty()) {
            text.add<Text>(options.description)
                .multiline(true)
                .addClassName("description");
        }
        if (!options.actions.empty()) {
            auto &actions = text.add<Div>();
            actions.addClassName("actions")
                   .direction(Direction::Row)
                   .wrap(Wrap::Wrap);
            for (const ToastAction &action: options.actions) {
                auto &button = actions.add<Button>(action.label);
                button.addClassName("small")
                      .skin(action.primary ? "primary" : "secondary");
                // The toast owns the button: a raw pointer to it
                button.onClick(
                    [this, callback = action.callback, keepOpen = action.keepOpen]() {
                        if (!keepOpen) {
                            dismiss();
                        }
                        if (callback) {
                            callback();
                        }
                    }
                );
            }
        }

        auto &close = add<Button>();
        close.icon("x")
             .addClassName("icon-button")
             .addClassName("close")
             .shrink(0);
        close.onClick([this]() { dismiss(); });
    }

    void Toast::dismiss() {
        Div *stack = parent();
        if (!stack || exiting()) {
            return;
        }
        const auto keepAlive = shared_from_this();
        // One with another under it fades where it is, rather than sliding down over it
        const auto &siblings = stack->children();
        const auto self      = std::find_if(siblings.begin(), siblings.end(), [this](const auto &c) { return c.get() == this; });
        if (std::any_of(self, siblings.end(), [this](const auto &c) { return c.get() != this && !c->exiting(); })) {
            addClassName("in-place");
        }
        // Out of the stack's flow, where it is (the stack keeps its bottom edge in the corner): the others slide into
        // its place as it leaves
        if (layoutReady) {
            position(Position::Absolute)
                .left(x())
                .width(getWidth())
                .bottom(stack->getHeight() - (y() + getHeight()));
        }
        // It leaves the stack once its exit transitions are over (at once with reduced motion)
        exit("exiting", [this]() {
            if (Div *from = parent()) {
                from->remove(*this);
            }
        });
        dismissed.emit();
    }

    void Toast::advance(const std::chrono::nanoseconds now, const bool paused) {
        if (!_last) {
            _last = now; // Shown since this frame
            return;
        }
        const auto delta = now - *_last;
        _last = now;
        if (paused || _options.timeout <= std::chrono::nanoseconds::zero()) {
            return;
        }
        _elapsed += delta;
        if (_elapsed >= _options.timeout) {
            dismiss();
        }
    }

    std::optional<std::chrono::nanoseconds> Toast::deadline() const {
        if (_options.timeout <= std::chrono::nanoseconds::zero()) {
            return std::nullopt;
        }
        if (!_last) {
            return std::chrono::nanoseconds::zero(); // Its timer starts with the next update
        }
        return *_last + (_options.timeout - _elapsed);
    }

    // endregion

    // region Stack

    ToastStack::ToastStack() :
        Popup() {
        tag("ToastStack");
        dismissOnOutsidePress(false);
        dismissOnEscape(false);
        restoreFocus(false);
        _defaultStyle.direction(Direction::Column)
            .justify(Justify::End);
    }

    Toast &ToastStack::add(const ToastOptions &options) {
        // The oldest go (fading where they are, `limited`, rather than sliding over the others)
        for (auto shown = toasts(); shown.size() >= limit; shown.erase(shown.begin())) {
            shown.front()->addClassName("limited");
            shown.front()->dismiss();
        }
        return Div::add<Toast>(options);
    }

    std::vector<Toast *> ToastStack::toasts() const {
        std::vector<Toast *> list{};
        for (const auto &child: _children) {
            if (!child->exiting()) {
                list.push_back(static_cast<Toast *>(child.get()));
            }
        }
        return list;
    }

    void ToastStack::update(const std::chrono::nanoseconds now) {
        // The mouse over the stack holds every toast (to read it, or reach its actions): the time since the last
        // update doesn't count when it is over now, or was then (frames stop while nothing changes, and the mouse
        // leaving brings the next one)
        const bool paused = mouseOver() || _paused;
        _paused = mouseOver();
        auto       children = _children; // A toast that times out leaves
        for (auto &child: children) {
            if (child->parent() == this && !child->exiting()) {
                static_cast<Toast *>(child.get())->advance(now, paused);
            }
        }
    }

    std::optional<std::chrono::nanoseconds> ToastStack::nextUpdate() const {
        if (mouseOver()) {
            return std::nullopt; // Held: the mouse leaving brings the next frame
        }
        std::optional<std::chrono::nanoseconds> next{};
        for (const auto &child: _children) {
            if (child->exiting()) {
                continue;
            }
            const auto deadline = static_cast<const Toast *>(child.get())->deadline();
            if (deadline && (!next || *deadline < *next)) {
                next = deadline;
            }
        }
        // Paused at the last update: the next one doesn't count, it only notes the time
        return next && _paused ? std::optional{std::chrono::nanoseconds::zero()} : next;
    }

    // endregion
}
