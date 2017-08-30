#pragma once

namespace psychicui {
    class StyleManager;
    class StyleSheet {
    public:
        virtual void load(StyleManager *manager) = 0;
    };
}

