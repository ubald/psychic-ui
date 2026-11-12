#pragma once

#include "../Skin.hpp"
#include "../components/Range.hpp"
#include "../components/Label.hpp"

namespace psychic_ui {
    namespace internal {
        class SliderBase;
    }

    class SliderRangeSkin : public RangeSkin {
    public:
        SliderRangeSkin();
        void setValue(float value) override;

    protected:
        void styleUpdated() override;
        void added() override;
        void removedFromRender() override;
        void sendMouseValue(int x, int y);
        void draw(SkCanvas *canvas) override;
        float                  _value{0.0f};
        Label *_valueLabel{nullptr}; // A child, which we own
        ScopedSlot             _onMouseMove{}; // On the window, only while dragging
    };
}

