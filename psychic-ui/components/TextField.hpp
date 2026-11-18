#pragma once

#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <unicode/unistr.h>
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/text/TextEditing.hpp"
#include "psychic-ui/text/WindowClipboard.hpp"

namespace psychic_ui {

    /**
     * @class TextField
     *
     * What every text field shares, written once: a component that takes the focus, the keys, typing and the mouse
     * itself and edits its text through a `TextEditing`, shown by its skin's view (`SkinT::textView()`, a `Text`
     * part the skin creates and owns). After every edit it shows the text, the selection and the caret in the view,
     * reveals the caret once laid out (scrolling one line), tells the window where the caret is (for the on-screen
     * keyboard and an input method's candidates) and emits `changed` when the text changed. A new skin gets the whole
     * state. While disabled it ignores input; read-only (`readOnly(true)`), it refuses edits but keeps the
     * selection, the caret's moves and copying, without showing the caret.
     *
     * Subclasses see the keys first (`fieldKey`) and show what follows the text (`textChanged`).
     */
    template<class SkinT>
    class TextField : public Component<SkinT> {
    public:
        /**
         * The text, in UTF-8, as it was set or edited
         */
        std::string text() const {
            return _editing.utf8();
        }

        /**
         * Set the text, the caret at its start (`changed` is for edits only)
         */
        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<TextField &>(self).updateText(text);
            return self;
        }

        bool readOnly() const {
            return _editing.readOnly();
        }

        /**
         * Whether edits are refused; selecting, moving the caret and copying still work
         */
        template<class Self>
        Self &readOnly(this Self &self, detail::BoolArg readOnly) {
            static_cast<TextField &>(self).updateReadOnly(readOnly.value);
            return self;
        }

        /**
         * The selection, sorted: equal when nothing is selected
         */
        std::pair<unsigned int, unsigned int> selection() const {
            return _editing.selection();
        }

        /**
         * Select from the anchor to the caret (UTF-16 indices, clamped to the text)
         */
        template<class Self>
        Self &selection(this Self &self, unsigned int anchor, unsigned int caret) {
            auto &field = static_cast<TextField &>(self);
            field.apply(field._editing.select(anchor, caret));
            return self;
        }

        /**
         * Where the caret is (the selection's moving end), in UTF-16 units
         */
        unsigned int caret() const {
            return _editing.caret();
        }

        /**
         * The text changed by an edit (typed, erased, cut or pasted), with the new text
         */
        Signal<std::string> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        explicit TextField(const std::string &text, bool multiline);

        TextEditing _editing{};
        bool        _revealCaret{false}; // Until the next layout: an edit changed the text, the view isn't laid out yet
        ScopedSlot  _onMouseMove{};      // On the window, only while selecting with the mouse

        /**
         * The skin's view, if there is a skin
         */
        Text *view() const {
            return this->_skin ? this->_skin->textView() : nullptr;
        }

        /**
         * A key, before editing has it
         * @return Whether it was used (then editing doesn't see it)
         */
        virtual bool fieldKey(Key /*key*/, Mod /*mod*/) { return false; }

        /**
         * The text changed (edited or set), shown in the view, before `changed`: show what follows it
         */
        virtual void textChanged() {}

        /**
         * Show what an edit changed, then tell: `changed` when the text changed, once the field is consistent
         */
        void apply(TextEditing::Edit edit);

        /**
         * Push the whole state to the view
         */
        void show();

        /**
         * Scroll the view to the caret and tell the window where it is
         */
        void revealCaret();

        void updateText(const std::string &text);
        void updateReadOnly(bool readOnly);
        void handleKey(Key key, Mod mod);
        void handlePress(const MouseEvent &event);
        bool editable() const;

        void skinChanged() override;
        void enabledChanged() override;
        void layoutUpdated() override;
        void removedFromRender() override;
    };

    template<class SkinT>
    TextField<SkinT>::TextField(const std::string &text, const bool multiline) :
        Component<SkinT>() {
        this->_focusEnabled = true;
        _editing.setMultiline(multiline);
        _editing.setText(icu::UnicodeString::fromUTF8(text));

        this->keyPressed.subscribe([this](const Key key, const Mod mod) { handleKey(key, mod); });
        this->keyRepeated.subscribe([this](const Key key, const Mod mod) { handleKey(key, mod); });
        this->typed.subscribe(
            [this](const icu::UnicodeString &str) {
                if (this->_enabled) {
                    apply(_editing.insert(str));
                }
            }
        );
        this->focused.subscribe(
            [this] {
                if (Window *window = this->window(); window && editable()) {
                    window->startTextInput();
                }
                show();
                revealCaret();
            }
        );
        this->blurred.subscribe(
            [this] {
                _onMouseMove.disconnect();
                if (Window *window = this->window(); window && editable()) {
                    window->stopTextInput();
                }
                show();
            }
        );
        this->mousePressed.subscribe([this](const MouseEvent &event) { handlePress(event); });
        this->mouseReleased.subscribe([this] { _onMouseMove.disconnect(); });
    }

    template<class SkinT>
    bool TextField<SkinT>::editable() const {
        return this->_enabled && !_editing.readOnly();
    }

    template<class SkinT>
    void TextField<SkinT>::updateText(const std::string &text) {
        if (has(_editing.setText(icu::UnicodeString::fromUTF8(text)), TextEditing::Edit::Text)) {
            show();
            textChanged();
            _revealCaret = true; // At its start
        }
    }

    template<class SkinT>
    void TextField<SkinT>::updateReadOnly(const bool readOnly) {
        if (_editing.readOnly() != readOnly) {
            _editing.setReadOnly(readOnly);
            if (Window *window = this->window(); window && this->hasFocus() && this->_enabled) {
                readOnly ? window->stopTextInput() : window->startTextInput();
            }
            show();
        }
    }

    template<class SkinT>
    void TextField<SkinT>::handleKey(const Key key, const Mod mod) {
        const Text *view = this->view();
        if (!this->_enabled || !view || fieldKey(key, mod)) {
            return;
        }
        WindowClipboard clipboard{this->window()};
        apply(_editing.handleKey(key, mod, *view, clipboard));
    }

    template<class SkinT>
    void TextField<SkinT>::handlePress(const MouseEvent &event) {
        const Text *view = this->view();
        if (!this->_enabled || !view || event.button != MouseButton::LEFT) {
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
        _onMouseMove = this->window()->mouseMoved.subscribe(
            [this, indexAt](const MouseEvent &move) {
                apply(_editing.dragTo(indexAt(move.windowX, move.windowY)));
            }
        );
    }

    template<class SkinT>
    void TextField<SkinT>::apply(const TextEditing::Edit edit) {
        if (edit == TextEditing::Edit::None) {
            return;
        }
        show();
        const bool textEdited = has(edit, TextEditing::Edit::Text);
        if (!textEdited && this->isValid()) {
            revealCaret(); // Laid out already: the text is the same
        } else {
            _revealCaret = true;
        }
        if (textEdited) {
            textChanged();
            const auto keepAlive = this->weak_from_this().lock(); // A handler may remove this field from its parent
            changed.emit(text());
        }
    }

    template<class SkinT>
    void TextField<SkinT>::show() {
        Text *view = this->view();
        if (!view) {
            return;
        }
        view->text(_editing.utf8()); // Laid out again only when it changed
        view->selection(_editing.anchor(), _editing.caret());
        view->caretVisible(this->_focused && editable());
    }

    template<class SkinT>
    void TextField<SkinT>::revealCaret() {
        _revealCaret = false;
        Text *view = this->view();
        if (!view) {
            return;
        }
        view->reveal(_editing.caret());

        // Where the caret's line is, across the view, in window coordinates
        Window *window = this->window();
        if (!window || !this->_focused || !editable()) {
            return;
        }
        const SkRect caret = view->caretRect(_editing.caret());
        int          x     = 0;
        int          y     = 0;
        view->localToGlobal(x, y, 0, static_cast<int>(caret.fTop));
        window->setTextInputArea(
            x, y,
            view->getWidth(),
            static_cast<int>(std::ceil(caret.height())),
            static_cast<int>(caret.fLeft)
        );
    }

    template<class SkinT>
    void TextField<SkinT>::skinChanged() {
        Component<SkinT>::skinChanged();
        if (Text *view = this->view()) {
            view->multiline(_editing.multiline());
        }
        show();
        textChanged();
        _revealCaret = true; // The new view is laid out with the next layout
    }

    template<class SkinT>
    void TextField<SkinT>::enabledChanged() {
        Component<SkinT>::enabledChanged();
        if (!this->_enabled) {
            _onMouseMove.disconnect();
        }
        show();
    }

    template<class SkinT>
    void TextField<SkinT>::layoutUpdated() {
        Component<SkinT>::layoutUpdated(); // The view first
        if (_revealCaret) {
            revealCaret();
        }
    }

    template<class SkinT>
    void TextField<SkinT>::removedFromRender() {
        Component<SkinT>::removedFromRender();
        _onMouseMove.disconnect(); // Removed mid-selection: the mouse up won't reach us
    }
}
