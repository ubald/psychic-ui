#pragma once

#include <functional>
#include <utility>
#include "psychicui/psychicui.hpp"
#include "psychicui/Component.hpp"
#include "Label.hpp"

namespace psychicui {
    class Button : public Component {
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
        std::shared_ptr<Label>    _label{nullptr};
        bool                      _toggle{false};
        bool                      _selected{false};
        std::function<void(bool)> _onChange{nullptr};

        void onMouseUp(int mouseX, int mouseY, int button, int modifiers) override;
        void onMouseDown(int mouseX, int mouseY, int button, int modifiers) override;
        void onClick() override;
    };
}

