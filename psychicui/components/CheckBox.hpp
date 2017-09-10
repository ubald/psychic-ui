#pragma once

#include <functional>
#include <utility>
#include "psychicui/psychicui.hpp"
#include "psychicui/Component.hpp"
#include "psychicui/Skin.hpp"
#include "psychicui/signals/Signal.hpp"
#include "Label.hpp"

namespace psychicui {
    class CheckBox;

    class CheckBoxSkin : public Skin<CheckBox> {
    public:
        virtual void setLabel(const std::string &label) {};
    };

    class CheckBox : public Component<CheckBoxSkin> {
    public:
        CheckBox(const std::string &label, std::function<void(bool)> onChangeCallback);

        explicit CheckBox(const std::string &label = "") : CheckBox(label, nullptr) {}

        explicit CheckBox(std::function<void(bool)> onChangeCallback) : CheckBox("", std::move(onChangeCallback)) {}

        const std::string &label() const;
        void setLabel(const std::string &label);

        const bool checked() const;
        void setChecked(bool checked);

        const bool active() const override;

        Signal<bool> onChange{};

    protected:
        std::string               _label{};
        bool                      _checked{false};

        void skinChanged() override;
    };
}

