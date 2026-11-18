#include <cstddef>
#include <limits>
#include <utility>
#include <vector>
#include "include/core/SkCanvas.h"
#include "psychic-ui/components/Icon.hpp"
#include "psychic-ui/components/Label.hpp"
#include "SegmentedControlSkinBase.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/components/SegmentedControl.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    SegmentedControlSkinBase::SegmentedControlSkinBase() :
        SegmentedControlSkin() {
        tag("SegmentedControlSkinBase");
    }

    void SegmentedControlSkinBase::setSegments(const std::vector<Segment> &segments) {
        for (const Div *segment: _segments) {
            remove(*segment);
        }
        _segments.clear();
        for (const Segment &s: segments) {
            Div &segment = add<Div>();
            segment.addClassName("segment");
            if (!s.icon.empty()) {
                segment.add<Icon>(s.icon).addClassName("icon").mouseEnabled(false);
            }
            if (!s.label.empty()) {
                segment.add<Label>(s.label).addClassName("label").mouseEnabled(false);
            }
            _segments.push_back(&segment);
        }
        const int selected = _selected;
        _selected = -1;
        setSelected(selected);
    }

    void SegmentedControlSkinBase::setSelected(const int index) {
        if (_selected >= 0 && std::cmp_less(_selected, _segments.size())) {
            _segments[static_cast<std::size_t>(_selected)]->removeClassName("selected");
        }
        _selected = index;
        if (_selected >= 0 && std::cmp_less(_selected, _segments.size())) {
            _segments[static_cast<std::size_t>(_selected)]->addClassName("selected");
        }
    }

    int SegmentedControlSkinBase::segmentAt(const int x, const int /*y*/) const {
        // The segments are our children, in a row: the nearest horizontally (zero inside one)
        int   nearest  = -1;
        float distance = std::numeric_limits<float>::max();
        for (std::size_t i = 0; i < _segments.size(); ++i) {
            const Div  *segment = _segments[i];
            const auto  left    = static_cast<float>(segment->x());
            const float right   = left + static_cast<float>(segment->getWidth());
            const auto  px      = static_cast<float>(x);
            float       d       = 0.0f; // Inside it
            if (px < left) {
                d = left - px;
            } else if (px >= right) {
                d = px - right + 1;
            }
            if (d < distance) {
                distance = d;
                nearest  = static_cast<int>(i);
            }
        }
        return nearest;
    }

    void SegmentedControlSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void SegmentedControlSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
