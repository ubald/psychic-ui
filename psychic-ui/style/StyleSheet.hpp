#pragma once

#include "../utils/Hatcher.hpp"

namespace psychic_ui {
    class StyleManager;
    namespace internal {
        class SkinBase;
    }

    class StyleSheet {
    public:
        virtual void load(StyleManager *manager) = 0;
    };
}

