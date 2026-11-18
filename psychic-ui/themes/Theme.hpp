#pragma once

#include <chrono>
#include <cstdint>
#include <string_view>
#include "psychic-ui/animation/Timing.hpp"
#include "psychic-ui/style/Color.hpp"

namespace psychic_ui {

    /**
     * Whether a theme is light or dark, and what the system asks for (`Unknown` when it doesn't say)
     */
    enum class ColorScheme : std::uint8_t {
        Unknown,
        Light,
        Dark
    };

    /**
     * Color roles, not colors: a skin set maps them to its states and never picks a color itself.
     * The roles are Pivotal's (packages/ui/src/styles/tokens.css): layers that separate by a step of grey, hairlines
     * instead of dividers, three levels of text and one accent spent deliberately.
     */
    struct Palette {
        // Layers. In a dark theme base, surface and raised go darkest to lightest; overlay sits dark.
        Color base{};          // The window's background, input wells, a check box's box
        Color surface{};       // Panels, cards, the inspector, tool bars
        Color raised{};        // Hover, inputs, the selection, secondary buttons
        Color overlay{};       // Menus, dialogs, tooltips: set apart by their edge and shadow, not lightness
        Color frame{};         // The app's frame: the sidebar and the window around the page panel
        Color page{};          // The page panel, inset in the frame
        Color hover{};         // A row or nav item under the mouse, lighter than `raised`

        // Hairlines, translucent so they sit on any layer
        Color line{};       // Between regions, under bars, around fields
        Color lineStrong{}; // Around popups and check boxes, a secondary button's hover

        // Text, by descending importance
        Color text{};          // Primary: titles, values, what was chosen
        Color textSecondary{}; // Body text, labels, controls at rest
        Color textMuted{};     // Metadata, placeholders, icons at rest

        // The one accent
        Color accent{};      // Primary buttons, check marks' boxes, the slider's fill, the focus ring
        Color accentHover{}; // A primary button under the mouse
        Color accentText{};  // The accent as text (links, counts), readable on every layer
        Color accentSoft{};  // Tinted surfaces, the text selection
        Color onAccent{};    // Text and marks drawn on the accent

        // Semantic, reserved for meaning
        Color danger{};
        Color dangerText{}; // As text on controls: readable on every layer
        Color warning{};
        Color warningText{};
        Color success{};

        Color scrollThumb{};
        Color scrollThumbHover{};
        Color scrim{}; // Behind a dialog
        Color switchOff{}; // A switch's track when off
        Color canvasDot{}; // The dots of a canvas's grid (`dotGridColor`), over the frame color
    };

    /**
     * A shadow, like a layer of CSS's box-shadow (see the `shadow*` style properties)
     */
    struct Shadow {
        Color color{};
        float blur{0};
        float offsetX{0};
        float offsetY{0};
        float spread{0};
    };

    /**
     * What floats: one elevation for everything above the page, in two layers (a soft drop and a close contact)
     */
    struct Elevation {
        Shadow overlay{};        // Menus, dialogs, tooltips
        Shadow overlayContact{}; // Their second layer, close under them
        Shadow thumb{};          // A slider's thumb
        Shadow panel{};          // Panels floating over a canvas (the Float set's sidebar and page), lower than overlays
        Shadow panelContact{};   // Their second layer, a hairline ring or a close contact
    };

    /**
     * A font file and the name `fontFamily` uses for it
     */
    struct Font {
        std::string_view name{};
        std::string_view path{}; // Loaded by the skin set (StyleManager::loadFont), relative to its resource root
    };

    /**
     * A step of the type scale: a size and its line height
     */
    struct TextStep {
        float size{12};
        float leading{18};
    };

    /**
     * Inter 4.1 (res/fonts/Inter, OFL 1.1), Pivotal's typeface, in the weights it uses (400, 500, 600), and its scale:
     * body text is `sm`, inputs and menu rows `base`, labels and metadata `xs`
     */
    struct Typography {
        Font     regular{.name = "Inter", .path = "res/fonts/Inter/Inter-Regular.ttf"};
        Font     medium{.name = "Inter Medium", .path = "res/fonts/Inter/Inter-Medium.ttf"};
        Font     semiBold{.name = "Inter SemiBold", .path = "res/fonts/Inter/Inter-SemiBold.ttf"}; // Brand and app titles only
        TextStep xxs{.size = 10, .leading = 14};
        TextStep xs{.size = 11, .leading = 16};
        TextStep sm{.size = 12, .leading = 18};
        TextStep base{.size = 13, .leading = 20};
        TextStep md{.size = 14, .leading = 20};
        TextStep xl{.size = 18, .leading = 26};
        TextStep xxl{.size = 20, .leading = 28};
    };

    /**
     * How dense the controls are: for a mouse, trackpad or pen (`Pointer`), or for fingers (`Touch`), which the style
     * manager switches to while the window is in touch mode
     */
    enum class Density : std::uint8_t {
        Pointer,
        Touch
    };

    /**
     * The sizes that follow the density: a skin set reads the current ones with `manager->sizes()`.
     * One scale of heights on the 4 px grid: controls sit inside rows and bars.
     */
    struct Sizes {
        float controlXs{20}; // Small icon buttons, avatars, badges
        float controlSm{24}; // Small buttons, fields in a tool bar, icon buttons
        float controlMd{28}; // Buttons, text inputs
        float rowSm{32};     // Menu, list and nav rows
        float rowMd{40};     // List rows, tabs
        float textStep{0};   // Added to the text sizes
        float hitSlop{0};    // How far around a control a press still reaches it (the `hitSlop` style)
        float iconStep{0};   // Added to the icon sizes (`Metrics::iconSm`, `iconMd`)
    };

    /**
     * Corner radii by role, so a role changes without touching the others
     */
    struct Radii {
        float control{8};  // Inputs, selects
        float button{999}; // Buttons, chips, segmented controls: pills
        float surface{12}; // Cards, panels
        float popup{16};   // Menus, dialogs, toasts
        float row{8};      // Menu, list and nav rows
        float page{16};    // The page panel
        float check{4};    // A check box's box
        float small{6};    // Keyboard hints, small marks
    };

    /**
     * How things move, by role: Pivotal's durations (`--duration-fast`, `--duration-medium`) and curve
     * (`--ease-out-quick`), "quick and barely noticeable, or it reads as sluggish", and a spring for what is thrown or
     * dragged. A window that reduces motion makes them all instant.
     */
    struct Motion {
        std::chrono::milliseconds fast{100};   // Hover, press and focus: colors, borders, the focus ring
        std::chrono::milliseconds medium{150}; // A switch's track, a selection moving, a slider's thumb on a key
        std::chrono::milliseconds slow{240};   // What travels further: the default set's grid rows as they're sorted; not Pivotal's, which has two steps
        Ease                      standard{Ease::Standard};                              // CSS's default curve, Pivotal's color transitions'
        Ease                      out{.x1 = 0.16f, .y1 = 1.0f, .x2 = 0.3f, .y2 = 1.0f}; // Things arriving (--ease-out-quick)
        Spring                    spring{.stiffness = 500.0f, .damping = 40.0f};          // Things thrown or dragged: a switch's thumb, barely overshooting

        [[nodiscard]] constexpr Timing quick() const { return {fast, standard}; }

        [[nodiscard]] constexpr Timing moderate() const { return {medium, standard}; }

        [[nodiscard]] constexpr Timing arrive() const { return {medium, out}; }

        [[nodiscard]] constexpr Timing enter() const { return {medium, out}; } // A popup appearing (Pivotal's floating motion)

        [[nodiscard]] constexpr Timing leave() const { return {fast, out}; } // A popup going: a little quicker than it came
    };

    struct Metrics {
        Radii radius{};
        float border{1};
        float focusRing{2};       // The focus ring's width (`outlineWidth`)
        float focusRingOffset{1}; // Its distance from the control (`outlineOffset`)
        float spaceUnit{4};       // The spacing scale's step: `space(2.5)` is 10 px, as Pivotal's `--space-2-5`
        float gutter{20};         // Page panes and tool bars
        float gutterCard{16};     // Cards and dialogs
        float frameInset{8};      // The page panel's inset from the window and the sidebar
        float barToolbar{44};     // A strip under a header: tabs, filters, a grid's header row
        float barHeader{52};      // A page's title bar, the sidebar's header
        float sidebarWidth{224};
        float menuWidth{192};     // A menu's least width
        float popoverWidth{320};  // A tooltip's most width
        float toastWidth{352};
        float dialogSm{400};      // A dialog's width, and a wide one's
        float dialogMd{560};
        float viewportMargin{16}; // What floats keeps this far from the window's edges (the toasts' corner)
        float scrollBar{10};      // Thin, self-effacing scroll bars
        float scrollThumbInset{3};
        float iconSm{12};
        float iconMd{14};
        float iconStroke{1.3f};
        Sizes pointer{};
        Sizes touch{.hitSlop = 10}; // A 24 px control reaches 44 px for a finger, and looks the same
        Motion motion{};

        [[nodiscard]] constexpr float space(float steps) const { return steps * spaceUnit; }

        [[nodiscard]] constexpr const Sizes &sizes(Density density) const {
            return density == Density::Touch ? touch : pointer;
        }
    };

    /**
     * What a skin set and app stylesheets draw with: values only, so any theme works with any skin set.
     * Read it with `manager->theme()` in `StyleSheet::load` (and the sizes of the current density with
     * `manager->sizes()`); switching themes or densities replays the loaded stylesheets.
     * Groups mix with designated initializers: `Theme{.palette = themes::pivotalLight.palette}`.
     */
    struct Theme {
        std::string_view name{};
        ColorScheme      scheme{ColorScheme::Dark};
        Palette          palette{};
        Elevation        elevation{};
        Typography       type{};
        Metrics          metrics{};
    };

}
