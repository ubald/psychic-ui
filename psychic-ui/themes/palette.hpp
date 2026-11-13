#pragma once

#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

/**
 * Named colors: a base16 gray scale and accents, and the One Dark palette
 */
namespace palette {
    inline constexpr Color base00 = argb(0xFF151515);
    inline constexpr Color base01 = argb(0xFF202020);
    inline constexpr Color base02 = argb(0xFF303030);
    inline constexpr Color base03 = argb(0xFF505050);
    inline constexpr Color base04 = argb(0xFFB0B0B0);
    inline constexpr Color base05 = argb(0xFFD0D0D0);
    inline constexpr Color base06 = argb(0xFFE0E0E0);
    inline constexpr Color base07 = argb(0xFFF5F5F5);

    inline constexpr Color baseRed       = argb(0xFFAC4142);
    inline constexpr Color baseOrange    = argb(0xFFD28445);
    inline constexpr Color baseYellow    = argb(0xFFF4BF75);
    inline constexpr Color baseGreen     = argb(0xFF90A959);
    inline constexpr Color baseTurquoise = argb(0xFF75B5AA);
    inline constexpr Color baseBlue      = argb(0xFF6A9FB5);
    inline constexpr Color baseViolet    = argb(0xFFAA759F);
    inline constexpr Color baseBrown     = argb(0xFF8F5536);

    inline constexpr Color oneLight      = argb(0xFFABB2BF);
    inline constexpr Color oneMedium     = argb(0xFF828997);
    inline constexpr Color oneDark       = argb(0xFF5C6370);
    inline constexpr Color oneCyan       = argb(0xFF56B6C2);
    inline constexpr Color oneBlue       = argb(0xFF61AFEF);
    inline constexpr Color onePurple     = argb(0xFFC678DD);
    inline constexpr Color oneGreen      = argb(0xFF98C379);
    inline constexpr Color oneRedFaded   = argb(0xFFE06C75);
    inline constexpr Color oneRed        = argb(0xFFBE5046);
    inline constexpr Color oneOrange     = argb(0xFFD19A66);
    inline constexpr Color oneYellow     = argb(0xFFE5C07B);
    inline constexpr Color oneForeground = argb(0xFFABB2BF);
    inline constexpr Color oneBackground = argb(0xFF282C34);
    inline constexpr Color oneGutter     = argb(0xFF636D83);
    inline constexpr Color oneBlueAccent = argb(0xFF528BFF);
}

}
