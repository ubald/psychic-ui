#include <algorithm>
#include <cmath>
#include <string>
#include <unicode/unistr.h>
#include "include/core/SkRect.h"
#include "psychic-ui/Window.hpp"
#include "TextField.hpp"
#include "psychic-ui/Div.hpp"
#include "Text.hpp"
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/text/TextEditing.hpp"

namespace psychic_ui {

    namespace {
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

    TextFieldBase::TextFieldBase() {
        _focusEnabled = true;

        keyPressed.subscribe([this](const Key key, const Mod mod) { handleKey(key, mod); });
        keyRepeated.subscribe([this](const Key key, const Mod mod) { handleKey(key, mod); });
        typed.subscribe(
            [this](const icu::UnicodeString &str) {
                if (_enabled) {
                    apply(_editing.insert(str));
                }
            }
        );
        focused.subscribe(
            [this] {
                if (Window *window = this->window(); window && editable()) {
                    window->startTextInput();
                }
                show();
                revealCaret();
            }
        );
        blurred.subscribe(
            [this] {
                _onMouseMove.disconnect();
                if (Window *window = this->window(); window && editable()) {
                    window->stopTextInput();
                }
                show();
            }
        );
        mousePressed.subscribe([this](const MouseEvent &event) { handlePress(event); });
        mouseReleased.subscribe([this] { _onMouseMove.disconnect(); });
    }

    bool TextFieldBase::editable() const {
        return _enabled && !_editing.readOnly();
    }

    void TextFieldBase::updateText(const std::string &text) {
        if (has(_editing.setText(icu::UnicodeString::fromUTF8(text)), TextEditing::Edit::Text)) {
            show();
            textChanged();
            _revealCaret = true; // At its start
        }
    }

    void TextFieldBase::updateReadOnly(const bool readOnly) {
        if (_editing.readOnly() != readOnly) {
            _editing.setReadOnly(readOnly);
            if (Window *window = this->window(); window && hasFocus() && _enabled) {
                readOnly ? window->stopTextInput() : window->startTextInput();
            }
            show();
        }
    }

    void TextFieldBase::handleKey(const Key key, const Mod mod) {
        const Text *view = this->view();
        if (!_enabled || !view) {
            return;
        }
        const auto keepAlive = weak_from_this().lock(); // What a key does may remove this field from its parent
        if (fieldKey(key, mod)) {
            return;
        }
        WindowClipboard clipboard{window()};
        apply(_editing.handleKey(key, mod, *view, clipboard));
    }

    void TextFieldBase::handlePress(const MouseEvent &event) {
        const Text *view = this->view();
        if (!_enabled || !view || event.button != MouseButton::LEFT) {
            return;
        }
        const auto keepAlive = weak_from_this().lock(); // What a press does may remove this field from its parent
        if (fieldPress(event)) {
            return;
        }
        // In the view's coordinates: a press beside the text goes to its nearest index
        const auto indexAt = [view](const float windowX, const float windowY) {
            int x = 0;
            int y = 0;
            view->globalToLocal(x, y, static_cast<int>(windowX), static_cast<int>(windowY));
            return view->indexAt(static_cast<float>(x), static_cast<float>(y));
        };
        // 1 click: the caret, 2: the word, 3: the sentence, 4: everything
        apply(_editing.selectAt(indexAt(event.windowX, event.windowY), std::max(1u, event.clickCount), event.modifiers.shift));
        _onMouseMove = window()->mouseMoved.subscribe(
            [this, indexAt](const MouseEvent &move) {
                apply(_editing.dragTo(indexAt(move.windowX, move.windowY)));
            }
        );
    }

    void TextFieldBase::apply(const TextEditing::Edit edit) {
        if (edit == TextEditing::Edit::None) {
            return;
        }
        show();
        const bool edited = has(edit, TextEditing::Edit::Text);
        if (!edited && isValid()) {
            revealCaret(); // Laid out already: the text is the same
        } else {
            _revealCaret = true;
        }
        if (edited) {
            textChanged();
            const auto keepAlive = weak_from_this().lock(); // A handler may remove this field from its parent
            textEdited();
        }
    }

    void TextFieldBase::show() {
        Text *view = this->view();
        if (!view) {
            return;
        }
        view->text(_editing.utf8()); // Laid out again only when it changed
        view->selection(_editing.anchor(), _editing.caret());
        view->caretVisible(_focused && editable());
    }

    void TextFieldBase::viewChanged() {
        if (Text *view = this->view()) {
            view->multiline(_editing.multiline());
        }
        show();
        textChanged();
        _revealCaret = true; // The new view is laid out with the next layout
    }

    void TextFieldBase::revealCaret() {
        _revealCaret = false;
        Text *view = this->view();
        if (!view) {
            return;
        }
        // One line scrolls itself; several lines scroll the view's viewport (the skin's, the nearest scrolling ancestor)
        view->reveal(_editing.caret());
        const SkRect caret = view->caretRect(_editing.caret());
        if (_editing.multiline()) {
            view->scrollIntoView(caret);
        }

        // Where the caret's line is, across the view, in window coordinates
        Window *window = this->window();
        if (!window || !_focused || !editable()) {
            return;
        }
        int x = 0;
        int y = 0;
        view->localToGlobal(x, y, 0, static_cast<int>(caret.fTop));
        window->setTextInputArea(
            x, y,
            view->getWidth(),
            static_cast<int>(std::ceil(caret.height())),
            static_cast<int>(caret.fLeft)
        );
    }

    void TextFieldBase::enabledChanged() {
        Div::enabledChanged();
        if (!_enabled) {
            _onMouseMove.disconnect();
        }
        show();
    }

    void TextFieldBase::layoutUpdated() {
        Div::layoutUpdated(); // The view first
        if (_revealCaret) {
            revealCaret();
        }
    }

    void TextFieldBase::removedFromRender() {
        Div::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-selection: the mouse up won't reach us
    }
}
