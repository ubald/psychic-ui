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
        // Inter 4.1 (res/fonts/Inter, OFL 1.1), Pivotal's typeface, in the weights it uses (400, 500, 600)
        Font  sans{"Inter", "../res/fonts/Inter/Inter-Regular.ttf"};
        Font  sansMedium{"Inter Medium", "../res/fonts/Inter/Inter-Medium.ttf"};
        Font  sansSemiBold{"Inter SemiBold", "../res/fonts/Inter/Inter-SemiBold.ttf"};
        float small{10};
        float text{12};
        float medium{14};
        float large{18};
        float big{24};
        float huge{36};
        float lineHeight{16}; // Of body text
    };

    /**
     * How dense the controls are: for a mouse, trackpad or pen (`Pointer`), or for fingers (`Touch`), which the style
     * manager switches to while the window is in touch mode
     */
    enum class Density {
        Pointer,
        Touch
    };

    /**
     * The sizes that follow the density: a skin set reads the current ones with `manager->sizes()`
     */
    struct Sizes {
        float control{24};  // Thickness of small controls: a slider, a check box, a scroll thumb's minimum length
        float row{24};      // Height of a row in a list or a menu
        float textStep{0};  // Added to the text sizes
        float hitSlop{0};   // How far around a control a press still reaches it (the `hitSlop` style)
    };

    struct Metrics {
        float radius{7};
        float border{1};
        float focusWidth{2};
        float scrollBar{12};
        float spaceSm{6};
        float spaceMd{12};
        float spaceLg{18};
        Sizes pointer{};
        Sizes touch{.hitSlop = 10}; // A 24 px control reaches 44 px for a finger, and looks the same

        constexpr const Sizes &sizes(Density density) const {
            return density == Density::Touch ? touch : pointer;
        }
    };

    /**
     * What a skin set and app stylesheets draw with: values only, so any theme works with any skin set.
     * Read it with `manager->theme()` in `StyleSheet::load` (and the sizes of the current density with
     * `manager->sizes()`); switching themes or densities replays the loaded stylesheets.
     * Groups mix with designated initializers: `Theme{.palette = themes::oneLight.palette}`.
     */
    struct Theme {
        std::string_view name{};
        Palette          palette{};
        Typography       type{};
        Metrics          metrics{};
    };

}
