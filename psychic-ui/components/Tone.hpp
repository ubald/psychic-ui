#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace psychic_ui {

    /**
     * What a badge, a status dot, a chip or an avatar's status means, which the skin set colors from the theme:
     * `Neutral` (secondary text, a strong hairline), `Accent`, `Success`, `Warning` and `Danger`. A component shows
     * it as the class `toneClass` names (`Badge.success`).
     */
    enum class Tone : std::uint8_t {
        Neutral,
        Accent,
        Success,
        Warning,
        Danger
    };

    /**
     * The class a component has for its tone: "neutral", "accent", "success", "warning" or "danger"
     */
    constexpr std::string_view toneClass(const Tone tone) {
        switch (tone) {
            case Tone::Accent: return "accent";
            case Tone::Success: return "success";
            case Tone::Warning: return "warning";
            case Tone::Danger: return "danger";
            case Tone::Neutral: break;
        }
        return "neutral";
    }

    inline constexpr std::array allTones{Tone::Neutral, Tone::Accent, Tone::Success, Tone::Warning, Tone::Danger};

    class Div;

    /**
     * Give a div the class of a tone, removing the others'
     */
    void applyTone(Div &div, Tone tone);
}
