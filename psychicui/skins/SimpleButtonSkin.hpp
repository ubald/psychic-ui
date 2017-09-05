#pragma once

#include "../Skin.hpp"
#include "../components/Button.hpp"

namespace psychicui {
    class SimpleButtonSkin : public ButtonSkin {
    public:
        SimpleButtonSkin();
        void setLabel(const std::string &label) override;
    protected:
        std::shared_ptr<Label> _label{nullptr};
    };
}

