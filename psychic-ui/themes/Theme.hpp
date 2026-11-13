#pragma once

#include <string_view>
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    /**
     * Color roles, not colors: a skin set maps them to its states and never picks a color itself
     */
    struct Palette {
        Color background{};    // Window, input wells, scroll track
        Color surface{};       // Title bar, tab container, panels
        Color muted{};         // Toolbars, tab strip, disabled controls, scroll thumb
        Color control{};       // Buttons
        Color controlHover{};  // Buttons under the mouse
        Color separator{};     // Borders between regions
        Color text{};          // Labels
        Color textMuted{};     // Secondary text
        Color accent{};        // Active, selected, focused
        Color onAccent{};      // Text on the accent
        Color onAccentMuted{}; // Secondary text on the accent (shortcuts in a highlighted menu item)
        Color danger{};
        Color warning{};
        Color success{};
    };

    /**
     * A font file and the name `fontFamily` uses for it
     */
    struct Font {
        std::string_view name{};
        std::string_view path{}; // Loaded by the skin set (StyleManager::loadFont)
    };

    struct Typography {
        Font  regular{"Ubuntu Regular", "../res/fonts/Ubuntu/Ubuntu-Regular.ttf"};
        Font  light{"Ubuntu Light", "../res/fonts/Ubuntu/Ubuntu-Light.ttf"};
        float small{10};
        float text{12};
        float medium{14};
        float large{18};
        float big{24};
        float huge{36};
        float lineHeight{16}; // Of body text
    };

    struct Metrics {
        float radius{7};
        float border{1};
        float focusWidth{2};
        float scrollBar{12};
        float control{24}; // Thickness of small controls: a slider, a check box, a scroll thumb's minimum length
        float spaceSm{6};
        float spaceMd{12};
        float spaceLg{18};
    };

    /**
     * What a skin set and app stylesheets draw with: values only, so any theme works with any skin set.
     * Read it with `manager->theme()` in `StyleSheet::load`; switching themes replays the loaded stylesheets.
     * Groups mix with designated initializers: `Theme{.palette = themes::oneLight.palette}`.
     */
    struct Theme {
        std::string_view name{};
        Palette          palette{};
        Typography       type{};
        Metrics          metrics{};
    };

}
