#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace psychic_ui {

    /**
     * How an undecorated window's controls look and are ordered: `Mac`, three dots in the order close, minimize,
     * maximize, at the start of a bar; `Windows` (Windows and Linux), three glyph buttons in the order minimize,
     * maximize, close, at the end of a bar. A `WindowControls` shows it as the class `windowControlsStyleClass`
     * names (`WindowControls.mac`, `WindowControls.windows`), which the skin set's rules key off.
     */
    enum class WindowControlsStyle : std::uint8_t {
        Mac,
        Windows
    };

    /**
     * The style of the platform the library was built for: `Mac` on macOS, `Windows` elsewhere
     */
    inline constexpr WindowControlsStyle platformWindowControlsStyle =
        #ifdef __APPLE__
        WindowControlsStyle::Mac;
        #else
        WindowControlsStyle::Windows;
        #endif

    /**
     * The class a `WindowControls` has for its style: "mac" or "windows"
     */
    constexpr std::string_view windowControlsStyleClass(const WindowControlsStyle style) {
        switch (style) {
            case WindowControlsStyle::Mac: return "mac";
            case WindowControlsStyle::Windows: break;
        }
        return "windows";
    }

    inline constexpr std::array allWindowControlsStyles{WindowControlsStyle::Mac, WindowControlsStyle::Windows};

    /**
     * Whether the controls sit at the start of their bar (macOS) rather than at its end
     */
    constexpr bool windowControlsLead(const WindowControlsStyle style) {
        return style == WindowControlsStyle::Mac;
    }
}
