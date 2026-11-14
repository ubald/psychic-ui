#include "DefaultInputSkins.hpp"

namespace psychic_ui {
    DefaultSwitchSkin::DefaultSwitchSkin() :
        SwitchSkinBase() {
        tag("DefaultSwitchSkin");
    }

    DefaultSegmentedControlSkin::DefaultSegmentedControlSkin() :
        SegmentedControlSkinBase() {
        tag("DefaultSegmentedControlSkin");
    }

    DefaultColorSwatchesSkin::DefaultColorSwatchesSkin() :
        ColorSwatchesSkinBase() {
        tag("DefaultColorSwatchesSkin");
    }

    DefaultNumberStepperSkin::DefaultNumberStepperSkin() :
        NumberStepperSkinBase() {
        tag("DefaultNumberStepperSkin");
    }

    DefaultSearchFieldSkin::DefaultSearchFieldSkin() :
        SearchFieldSkinBase() {
        tag("DefaultSearchFieldSkin");
    }
}
