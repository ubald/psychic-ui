#pragma once

#include <string>
#include "psychic-ui/Window.hpp"
#include "psychic-ui/text/TextEditing.hpp"

namespace psychic_ui {

    /**
     * A window's clipboard, for the editing model; none without a window
     */
    class WindowClipboard final : public TextEditing::Clipboard {
    public:
        explicit WindowClipboard(Window *window) : _window(window) {}

        std::string clipboardText() const override {
            return _window ? _window->clipboardText() : std::string{};
        }

        void setClipboardText(const std::string &text) override {
            if (_window) {
                _window->setClipboardText(text);
            }
        }

    private:
        Window *_window;
    };
}
