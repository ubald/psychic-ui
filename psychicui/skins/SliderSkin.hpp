#pragma once

#include "../Skin.hpp"
#include "../components/Label.hpp"

namespace psychicui {
    namespace internal {
        class SliderBase;
    }

    class SliderSkin : public Skin<internal::SliderBase> {
    public:
        SliderSkin();

        Component *track() const { return _track.get(); };

        Component *range() const { return _range.get(); };

        void setValue(const float value);

        void componentChanged() override;

    protected:
        void sendMouseValue(const int x, const int y);

        bool                       _dragging{false};
        std::shared_ptr<Label> _value{nullptr};
        std::shared_ptr<Component> _track{nullptr};
        std::shared_ptr<Component> _range{nullptr};
    };
}

