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
            tag("CheckBoxSkin");
        }

        virtual void setLabel(const std::string &/*label*/) {};
    };

    class CheckBox : public Component<CheckBoxSkin> {
    public:
        CheckBox(const std::string &label, std::function<void(bool)> onChangeCallback);

        explicit CheckBox(const std::string &label = "") : CheckBox(label, nullptr) {}

        explicit CheckBox(std::function<void(bool)> onChangeCallback) : CheckBox("", std::move(onChangeCallback)) {}

        const std::string &label() const;

        template<class Self>
        Self &label(this Self &self, const std::string &label) {
            static_cast<CheckBox &>(self).updateLabel(label);
            return self;
        }

        bool checked() const;

        template<class Self>
        Self &checked(this Self &self, detail::BoolArg checked) {
            static_cast<CheckBox &>(self).updateChecked(checked.value);
            return self;
        }

        bool active() const override;

        /**
         * The checked state changed, with the new state
         */
        Signal<bool> changed{};
        PSYCHIC_UI_EVENT(onChange, changed)

    protected:
        std::string _label{};
        bool        _checked{false};

        void updateLabel(const std::string &label);
        void updateChecked(bool checked);
        void skinChanged() override;
    };
}

