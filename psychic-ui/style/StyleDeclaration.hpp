#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "Style.hpp"
#include "StyleSelector.hpp"

namespace psychic_ui {
    class StyleDeclaration {
    public:
        explicit StyleDeclaration(std::unique_ptr<StyleSelector> selector);
        StyleDeclaration(std::unique_ptr<StyleSelector> selector, const std::function<void()> &onChanged);
        const StyleSelector *selector() const;
        Style *style() const;

        /**
         * Returns the cached weight computed from the selector chain
         * @return int
         */
        int weight() const;

        /**
         * Whether the rule is in effect: false once its style manager was reset, until it is declared again
         */
        bool active() const { return _active; }

        void setActive(bool active) { _active = active; }

    protected:
        const std::unique_ptr<StyleSelector> _selector{nullptr};
        std::unique_ptr<Style>               _style{nullptr};
        int                                  _weight{0};
        bool                                 _active{false};

        #ifdef DEBUG_STYLES
    public:
        std::string selectorString{};
        #endif
    };
}
