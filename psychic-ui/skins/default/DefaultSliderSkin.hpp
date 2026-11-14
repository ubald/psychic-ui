#pragma once

#include "psychic-ui/skins/base/RangeSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default range, Pivotal's slider: a thin rounded track, the accent filling it up to the value, a round thumb
     * with a soft shadow on it, and the value after the track. The parts paint themselves from the set's rules.
     */
    class DefaultSliderSkin : public RangeSkinBase {
    public:
        DefaultSliderSkin();
    };
}
