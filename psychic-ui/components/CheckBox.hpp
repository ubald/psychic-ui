#pragma once

#include <functional>
#include <utility>
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "Label.hpp"

namespace psychic_ui {
    class CheckBox;

    class CheckBoxSkin : public Skin<CheckBox> {
    public:
        CheckBoxSkin() : Skin<CheckBox>() {
            setTag("CheckBoxSkin");
        }

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
        CheckBox *setChecked(bool checked);

        const bool active() const override;

        Signal<bool> onChange{};

    protected:
        std::string _label{};
        bool        _checked{false};

        void skinChanged() override;
    };
}

