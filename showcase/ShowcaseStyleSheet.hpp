#pragma once

#include <psychic-ui/style/StyleSheet.hpp>

namespace psychic_ui::showcase {
    /**
     * The showcase's own rules, over the skin set's and from the same theme: the backdrop the scenes sit on, the
     * panels floating over it, and the captions in them. Only layout and color: the components look as their skin
     * set draws them.
     */
    class ShowcaseStyleSheet : public StyleSheet {
    public:
        void load(StyleManager *manager) override;
    };
}
