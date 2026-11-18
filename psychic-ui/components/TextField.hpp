#pragma once

#include <string>
#include <utility>
#include "psychic-ui/Component.hpp"
#include "psychic-ui/components/Text.hpp"
#include "psychic-ui/text/TextEditing.hpp"

namespace psychic_ui {

    /**
     * @class TextFieldBase
     *
     * What every text field shares, written once: a Div that takes the focus, the keys, typing and the mouse itself
     * and edits its text through a `TextEditing`, shown by a view (`view()`, a `Text` part its skin creates and
     * owns). After every edit it shows the text, the selection and the caret in the view, reveals the caret once laid
     * out (scrolling one line, or the viewport around several lines), tells the window where the caret is (for the
     * on-screen keyboard and an input method's candidates) and, when the text changed, calls `textEdited`. A new view
     * gets the whole state. While disabled it ignores input (and doesn't take the focus); read-only
     * (`readOnly(true)`), it refuses edits but keeps the selection, the caret's moves and copying, without showing
     * the caret.
     *
     * Subclasses see the keys and the presses first (`fieldKey`, `fieldPress`), show what follows the text
     * (`textChanged`) and declare their own `changed` signal, with what changes for them (`textEdited`): the text for
     * `TextInput`, `TextArea` and `SearchField`, the value for `NumberStepper`. They are components, through
     * `TextField<SkinT>`.
     */
    class TextFieldBase : public Div {
    public:
        /**
         * The text, in UTF-8, as it was set or edited
         */
        std::string text() const { return _editing.utf8(); }

        /**
         * Set the text, the caret at its start (`changed` is for edits only)
         */
        template<class Self>
        Self &text(this Self &self, const std::string &text) {
            static_cast<TextFieldBase &>(self).updateText(text);
            return self;
        }

        bool readOnly() const { return _editing.readOnly(); }

        /**
         * Whether edits are refused; selecting, moving the caret and copying still work
         */
        template<class Self>
        Self &readOnly(this Self &self, detail::BoolArg readOnly) {
            static_cast<TextFieldBase &>(self).updateReadOnly(readOnly.value);
            return self;
        }

        /**
         * The selection, sorted: equal when nothing is selected
         */
        std::pair<unsigned int, unsigned int> selection() const { return _editing.selection(); }

        /**
         * Select from the anchor to the caret (UTF-16 indices, clamped to the text)
         */
        template<class Self>
        Self &selection(this Self &self, unsigned int anchor, unsigned int caret) {
            auto &field = static_cast<TextFieldBase &>(self);
            field.apply(field._editing.select(anchor, caret));
            return self;
        }

        /**
         * Where the caret is (the selection's moving end), in UTF-16 units
         */
        unsigned int caret() const { return _editing.caret(); }

    protected:
        TextFieldBase();

        TextEditing _editing{};
        bool        _revealCaret{false}; // Until the next layout: an edit changed the text, the view isn't laid out yet
        ScopedSlot  _onMouseMove{};      // On the window, only while selecting with the mouse

        /**
         * The skin's view, if there is a skin
         */
        virtual Text *view() const = 0;

        /**
         * A key, before editing has it; the field is kept alive meanwhile
         * @return Whether it was used (then editing doesn't see it)
         */
        virtual bool fieldKey(Key /*key*/, Mod /*mod*/) { return false; }

        /**
         * A left press, before it places the caret or starts a selection (a button part of the skin's); the field is
         * kept alive meanwhile
         * @return Whether it was used (then the text doesn't see it)
         */
        virtual bool fieldPress(const MouseEvent &/*event*/) { return false; }

        /**
         * The text changed (edited or set), shown in the view, before `textEdited`: show what follows it
         */
        virtual void textChanged() {}

        /**
         * An edit (typed, erased, cut or pasted) changed the text, once the field is consistent: tell the world. A
         * handler may remove the field from its parent; it is kept alive meanwhile.
         */
        virtual void textEdited() {}

        /**
         * Show what an edit changed, then tell (`textEdited`) when the text changed, once the field is consistent
         */
        void apply(TextEditing::Edit edit);

        /**
         * Push the whole state to the view
         */
        void show();

        /**
         * Push the whole state to a new view, revealing the caret once it is laid out
         */
        void viewChanged();

        /**
         * Scroll the view to the caret and tell the window where it is
         */
        void revealCaret();

        void updateText(const std::string &text);
        void updateReadOnly(bool readOnly);
        void handleKey(Key key, Mod mod);
        void handlePress(const MouseEvent &event);
        bool editable() const;

        void enabledChanged() override;
        void layoutUpdated() override;
        void removedFromRender() override;
    };

    /**
     * @class TextField
     *
     * A text field drawn by a skin implementing `SkinT`, whose `textView()` is the field's view
     */
    template<class SkinT>
    class TextField : public Component<SkinT, TextFieldBase> {
    protected:
        TextField(const std::string &text, const bool multiline) {
            this->_editing.setMultiline(multiline);
            this->_editing.setText(icu::UnicodeString::fromUTF8(text));
        }

        Text *view() const override { // NOLINT(portability-template-virtual-member-function): it compiles for every T, whether instantiated or not
            return this->_skin ? this->_skin->textView() : nullptr;
        }

        void skinChanged() override { // NOLINT(portability-template-virtual-member-function): it compiles for every T, whether instantiated or not
            Component<SkinT, TextFieldBase>::skinChanged();
            this->viewChanged();
        }
    };
}
