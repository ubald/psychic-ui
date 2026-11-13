#pragma once

#include <functional>
#include <utility>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "Label.hpp"

namespace psychic_ui {
    class Button;

    class ButtonSkin : public Skin<Button> {
    public:
        ButtonSkin() : Skin<Button>() {
            setTag("ButtonSkin");
        }

        virtual void setLabel(const std::string &/*label*/) {};
    };

    class Button : public Component<ButtonSkin> {
    public:
        /**
         * @param onClick Connected to `clicked`, after the button updates its selection
         */
        Button(const std::string &label, ClickCallback onClick);

        explicit Button(const std::string &label = "") : Button(label, nullptr) {}

        explicit Button(ClickCallback onClick) : Button("", std::move(onClick)) {}

        const std::string &label() const;

        template<class Self>
        Self &label(this Self &self, const std::string &label) {
            static_cast<Button &>(self).updateLabel(label);
            return self;
        }

        /**
         * Whether a click toggles the selection (instead of selecting while pressed)
         */
        bool toggle() const { return _toggle; }

        template<class Self>
        Self &toggle(this Self &self, detail::BoolArg toggle) {
            static_cast<Button &>(self)._toggle = toggle.value;
            return self;
        }

        /**
         * Whether a toggle button changes its selection itself when clicked
         */
        bool autoToggle() const { return _autoToggle; }

        template<class Self>
        Self &autoToggle(this Self &self, detail::BoolArg autoToggle) {
            static_cast<Button &>(self)._autoToggle = autoToggle.value;
            return self;
        }

        bool selected() const;

        template<class Self>
        Self &selected(this Self &self, detail::BoolArg selected) {
            static_cast<Button &>(self).updateSelected(selected.value);
            return self;
        }

        bool active() const override;

        /**
         * The selection changed, with the new state
         */
        Signal<bool> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        std::string               _label{};
        bool                      _toggle{false};
        bool                      _autoToggle{true};
        bool                      _selected{false};

        void updateLabel(const std::string &label);
        void updateSelected(bool selected);
        void skinChanged() override;
    };
}

