#pragma once

#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    /**
     * The interaction state a skin paints, read from its component. Colors and sizes per state come from the
     * computed style instead (rules on `:hover`, `:active`, `:focus`, `:disabled`).
     */
    struct SkinState {
        bool hover{false};
        bool active{false};
        bool focus{false};
        bool disabled{false};

        static SkinState of(const Div *component) {
            if (!component) {
                return {};
            }
            return {component->mouseOver(), component->active(), component->hasFocus(), !component->enabled()};
        }
    };
}
