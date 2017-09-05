#pragma once

#include "../utils/Hatcher.hpp"

namespace psychicui {
    class StyleManager;
    namespace internal {
        class SkinBase;
    }

    class StyleSheet {
    public:
        virtual void load(StyleManager *manager) = 0;
    };
}

