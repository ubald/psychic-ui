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
                .selectable(false)
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
        if (Div *stack = parent()) {
            const auto keepAlive = shared_from_this();
            stack->remove(*this);
            dismissed.emit();
        }
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
        while (_children.size() >= limit) {
            static_cast<Toast *>(_children.front().get())->dismiss();
        }
        return Div::add<Toast>(options);
    }

    std::vector<Toast *> ToastStack::toasts() const {
        std::vector<Toast *> list{};
        for (const auto &child: _children) {
            list.push_back(static_cast<Toast *>(child.get()));
        }
        return list;
    }

    void ToastStack::update(const std::chrono::nanoseconds now) {
        // The mouse over the stack holds every toast (to read it, or reach its actions)
        const bool paused = mouseOver();
        auto       children = _children; // A toast that times out leaves
        for (auto &child: children) {
            if (child->parent() == this) {
                static_cast<Toast *>(child.get())->advance(now, paused);
            }
        }
    }

    // endregion
}
