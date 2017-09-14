#pragma once

#include <string>
#include <algorithm>
#include <yoga/Yoga.h>

YGFlexDirection YogaDirectionFromString(std::string direction, YGFlexDirection fallback) {
    std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);
    if (direction == "column") { return YGFlexDirectionColumn; }
    else if (direction == "columnreverse") { return YGFlexDirectionColumnReverse; }
    else if (direction == "row") { return YGFlexDirectionRow; }
    else if (direction == "rowreverse") { return YGFlexDirectionRowReverse; }
    else { return YGFlexDirectionColumn; };
};

YGJustify YogaJustifyFromString(std::string justify, YGJustify fallback) {
    std::transform(justify.begin(), justify.end(), justify.begin(), ::tolower);
    if (justify == "start") { return YGJustifyFlexStart; }
    else if (justify == "center") { return YGJustifyCenter; }
    else if (justify == "end") { return YGJustifyFlexEnd; }
    else if (justify == "spacebetween") { return YGJustifySpaceBetween; }
    else if (justify == "spacearound") { return YGJustifySpaceAround; }
    else { return fallback; };
};

YGAlign YogaAlignFromString(std::string align, YGAlign fallback) {
    std::transform(align.begin(), align.end(), align.begin(), ::tolower);
    if (align == "auto") { return YGAlignAuto; }
    else if (align == "start") { return YGAlignFlexStart; }
    else if (align == "center") { return YGAlignCenter; }
    else if (align == "end") { return YGAlignFlexEnd; }
    else if (align == "stretch") { return YGAlignStretch; }
    else if (align == "baseline") { return YGAlignBaseline; }
    else if (align == "spacebetween") { return YGAlignSpaceBetween; }
    else if (align == "spacearound") { return YGAlignSpaceAround; }
    else { return fallback; };
};

YGPositionType YogaPositionFromString(std::string position, YGPositionType fallback) {
    std::transform(position.begin(), position.end(), position.begin(), ::tolower);
    if (position == "relative") { return YGPositionTypeRelative; }
    else if (position == "absolute") { return YGPositionTypeAbsolute; }
    else { return fallback; };
};

YGWrap YogaWrapFromString(std::string wrap, YGWrap fallback) {
    std::transform(wrap.begin(), wrap.end(), wrap.begin(), ::tolower);
    if (wrap == "nowrap") { return YGWrapNoWrap; }
    else if (wrap == "wrap") { return YGWrapWrap; }
    else if (wrap == "wrapreverse") { return YGWrapWrapReverse; }
    else { return fallback; };
};

YGOverflow YogaOverflowFromString(std::string overflow, YGOverflow fallback) {
    std::transform(overflow.begin(), overflow.end(), overflow.begin(), ::tolower);
    if (overflow == "visible") { return YGOverflowVisible; }
    else if (overflow == "hidden") { return YGOverflowHidden; }
    else if (overflow == "scroll") { return YGOverflowScroll; }
    else { return fallback; };
};

float YogaPercent(float value) {
    return !isnan(value) ? value * 100.f : value;
}
