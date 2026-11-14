#pragma once

#include "psychic-ui/skins/base/ColorSwatchesSkinBase.hpp"
#include "psychic-ui/skins/base/NumberStepperSkinBase.hpp"
#include "psychic-ui/skins/base/SearchFieldSkinBase.hpp"
#include "psychic-ui/skins/base/SegmentedControlSkinBase.hpp"
#include "psychic-ui/skins/base/SwitchSkinBase.hpp"

/**
 * The default skins of the inputs, Pivotal's. Their parts carry the whole look, from the skin set's rules:
 * these only name themselves, so rules can bind to the set (`Switch DefaultSwitchSkin`).
 */
namespace psychic_ui {
    /**
     * A 30 × 18 pill track in a strong hairline, the accent when on, a white 12 px thumb with the thumb shadow
     */
    class DefaultSwitchSkin : public SwitchSkinBase {
    public:
        DefaultSwitchSkin();
    };

    /**
     * A pill on the surface color in a hairline, its selected segment raised, in the text color and medium weight
     */
    class DefaultSegmentedControlSkin : public SegmentedControlSkinBase {
    public:
        DefaultSegmentedControlSkin();
    };

    /**
     * 20 px round chips 4 px apart, wrapping; the selected one ringed in the text color, the accent with the focus
     */
    class DefaultColorSwatchesSkin : public ColorSwatchesSkinBase {
    public:
        DefaultColorSwatchesSkin();
    };

    /**
     * A raised field in a hairline, four controls wide, − and + at its ends behind hairlines, the value centered
     */
    class DefaultNumberStepperSkin : public NumberStepperSkinBase {
    public:
        DefaultNumberStepperSkin();
    };

    /**
     * A text input's field with a muted search icon before the text and a clear button after it
     */
    class DefaultSearchFieldSkin : public SearchFieldSkinBase {
    public:
        DefaultSearchFieldSkin();
    };
}
