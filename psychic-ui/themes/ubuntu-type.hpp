#pragma once

#include "Theme.hpp"

namespace psychic_ui::themes {
    /**
     * The Ubuntu typeface (res/fonts/Ubuntu, UFL) on Pivotal's scale: the One and Psychic themes' type
     */
    inline constexpr Typography ubuntuType{
        .regular  = {.name = "Ubuntu Regular", .path = "res/fonts/Ubuntu/Ubuntu-Regular.ttf"},
        .medium   = {.name = "Ubuntu Medium", .path = "res/fonts/Ubuntu/Ubuntu-Medium.ttf"},
        .semiBold = {.name = "Ubuntu Bold", .path = "res/fonts/Ubuntu/Ubuntu-Bold.ttf"},
    };
}
