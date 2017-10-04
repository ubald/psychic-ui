#pragma once

#include "psychic-ui/components/TextArea.hpp"
#include "psychic-ui/components/Scroller.hpp"

namespace psychic_ui {
    class DefaultTextAreaSkin : public TextAreaSkin {
    public:
        DefaultTextAreaSkin();
    private:
        void addedToComponent() override;
        void removedFromComponent() override;
        void draw(SkCanvas *canvas) override;
    };
}

