#pragma once

#include <functional>
#include <SkCanvas.h>
#include "psychicui.hpp"
#include "Component.hpp"
#include "Label.hpp"

namespace psychicui {
    class Button : public Component {
    public:
        explicit Button(const std::string &label = "");
        Button(const std::string &label, std::function<void()> onClick);
        explicit Button(std::function<void()> onClick);

        std::string label();
        void setLabel(std::string label);

        bool selected();
        void setSelected(bool selected);

        const bool active() const override;

        void onClick(std::function<void()> callback);
    protected:
        void createChildren();

        std::shared_ptr<Label> _label{nullptr};
        bool _selected{false};
    };
}

