#pragma once

#include "Theme.hpp"

namespace psychic_ui::themes {
    /**
     * The Ubuntu typeface (res/fonts/Ubuntu, UFL) on Pivotal's scale: the One and Psychic themes' type
     */
    inline constexpr Typography ubuntuType{
        .regular  = {"Ubuntu Regular", "res/fonts/Ubuntu/Ubuntu-Regular.ttf"},
        .medium   = {"Ubuntu Medium", "res/fonts/Ubuntu/Ubuntu-Medium.ttf"},
        .semiBold = {"Ubuntu Bold", "res/fonts/Ubuntu/Ubuntu-Bold.ttf"},
    };
}
