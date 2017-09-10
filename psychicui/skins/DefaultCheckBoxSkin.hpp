#pragma once

#include "../Skin.hpp"
#include "../Shape.hpp"
#include "../components/CheckBox.hpp"

namespace psychicui {
    class DefaultCheckBoxSkin : public CheckBoxSkin {
    public:
        DefaultCheckBoxSkin();
        void setLabel(const std::string &label) override;
    protected:
        std::shared_ptr<Shape> _box{nullptr};
        std::shared_ptr<Label> _label{nullptr};
        void draw(SkCanvas *canvas) override ;
    };
}

