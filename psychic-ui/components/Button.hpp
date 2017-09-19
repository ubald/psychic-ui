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

        virtual void setLabel(const std::string &label) {};
    };

    class Button : public Component<ButtonSkin> {
    public:
        Button(const std::string &label, ClickCallback onClick);

        explicit Button(const std::string &label = "") : Button(label, nullptr) {}

        explicit Button(const ClickCallback &onClick) : Button("", std::move(onClick)) {}

        const std::string &label() const;
        void setLabel(const std::string &label);

        const bool toggle() const { return _toggle; }

        Button *setToggle(const bool toggle) {
            _toggle = toggle;
            return this;
        }

        const bool autoToggle() const { return _autoToggle; }

        Button *setAutoToggle(const bool autoToggle) {
            _autoToggle = autoToggle;
            return this;
        }

        const bool selected() const;
        Button *setSelected(bool selected);

        const bool active() const override;

        Button *onChange(std::function<void(bool)> onChange) {
            _onChange = std::move(onChange);
            return this;
        }

    protected:
        std::string               _label{};
        bool                      _toggle{false};
        bool                      _autoToggle{true};
        bool                      _selected{false};
        std::function<void(bool)> _onChange{nullptr};

        void skinChanged() override;
    };
}

