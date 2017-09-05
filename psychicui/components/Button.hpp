#pragma once

#include <functional>
#include <utility>
#include "psychicui/psychicui.hpp"
#include "psychicui/SkinnableComponent.hpp"
#include "psychicui/Skin.hpp"
#include "Label.hpp"

namespace psychicui {
    class Button;

    class ButtonSkin : public Skin<Button> {
    public:
        virtual void setLabel(const std::string &label) {};
    };

    class Button : public SkinnableComponent<ButtonSkin> {
    public:
        Button(const std::string &label, ClickCallback onClick);

        explicit Button(const std::string &label = "") : Button(label, nullptr) {}

        explicit Button(ClickCallback onClick) : Button("", onClick) {}

        const std::string &label() const;
        void setLabel(const std::string &label);

        const bool toggle() const { return _toggle; }

        Button *setToggle(const bool toggle) {
            _toggle = toggle;
            return this;
        }

        const bool selected() const;
        void setSelected(const bool selected);

        const bool active() const override;

        Button *onChange(std::function<void(bool)> onChange) {
            _onChange = std::move(onChange);
            return this;
        }

    protected:
        std::string               _label{};
        bool                      _toggle{false};
        bool                      _selected{false};
        std::function<void(bool)> _onChange{nullptr};

        void skinChanged() override;
        void onMouseUp(int mouseX, int mouseY, int button, int modifiers) override;
        void onMouseDown(int mouseX, int mouseY, int button, int modifiers) override;
        void onClick() override;
    };
}

