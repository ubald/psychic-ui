#include <iostream>
#include "Style.hpp"

namespace psychicui {
    // Load fonts before creating the default setStyle
//    sk_sp<SkTypeface> Style::defaultFont = SkTypeface::MakeFromFile("res/fonts/stan0755.ttf");
    sk_sp<SkTypeface> Style::defaultFont = SkTypeface::MakeFromFile("res/fonts/Ubuntu/Ubuntu-Light.ttf");

    // Create the default setStyle
    std::shared_ptr<Style> Style::defaultStyle = std::make_shared<Style>();

    Style::Style() :
        font(defaultFont) {
    }
}