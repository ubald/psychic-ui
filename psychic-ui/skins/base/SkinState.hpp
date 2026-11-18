#pragma once

#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    /**
     * The interaction state a skin paints, read from its component. Colors and sizes per state come from the
     * computed style instead (rules on `:hover`, `:active`, `:pressed`, `.selected`, `:focus`, `:disabled`).
     */
    struct SkinState {
        bool hover{false};
        bool active{false};   // `:active`: pressed, or selected (a toggle button that is on)
        bool pressed{false};  // `:pressed`: held down
        bool selected{false}; // The `selected` class: a toggle button that is on, a selected row or chip
        bool focus{false};
        bool disabled{false};

        static SkinState of(const Div *component) {
            if (!component) {
                return {};
            }
            return {
                .hover    = component->mouseOver(),
                .active   = component->active(),
                .pressed  = component->pressed(),
                .selected = component->classNames().contains("selected"),
                .focus    = component->hasFocus(),
                .disabled = !component->enabled()
            };
        }
    };
}
