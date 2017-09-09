#pragma once

#include <functional>
#include <utility>
#include "psychicui/psychicui.hpp"
#include "psychicui/Component.hpp"
#include "psychicui/Skin.hpp"
#include "Label.hpp"

namespace psychicui {
    class CheckBox;

    class CheckBoxSkin : public Skin<CheckBox> {
    public:
        virtual void setLabel(const std::string &label) {};
    };

    class CheckBox : public Component<CheckBoxSkin> {
    public:
        CheckBox(const std::string &label, ClickCallback onClick);

        explicit CheckBox(const std::string &label = "") : CheckBox(label, nullptr) {}

        explicit CheckBox(const ClickCallback &onClick) : CheckBox("", std::move(onClick)) {}

        const std::string &label() const;
        void setLabel(const std::string &label);

        const bool toggle() const { return _toggle; }

        CheckBox *setToggle(const bool toggle) {
            _toggle = toggle;
            return this;
        }

        const bool autoToggle() const { return _autoToggle; }

        CheckBox *setAutoToggle(const bool autoToggle) {
            _autoToggle = autoToggle;
            return this;
        }

        const bool selected() const;
        void setSelected(bool selected);

        const bool active() const override;

        CheckBox *onChange(std::function<void(bool)> onChange) {
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

