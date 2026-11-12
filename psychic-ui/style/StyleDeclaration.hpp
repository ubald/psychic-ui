#pragma once

#include <memory>
#include <string>
#include "Style.hpp"
#include "StyleSelector.hpp"

namespace psychic_ui {
    /**
     * A stylesheet rule: a selector and the style it applies
     */
    class StyleDeclaration {
    public:
        StyleDeclaration(std::string selectorString, std::unique_ptr<StyleSelector> selector, Style::ChangedCallback onChanged = nullptr, void *owner = nullptr);

        const std::string &selectorString() const { return _selectorString; }

        const StyleSelector *selector() const { return _selector.get(); }

        Style &style() { return _style; }

        const Style &style() const { return _style; }

        /**
         * Returns the cached weight computed from the selector chain
         * @return int
         */
        int weight() const { return _weight; }

        /**
         * Whether the rule is in effect: false once its style manager was reset, until it is declared again
         */
        bool active() const { return _active; }

        void setActive(bool active) { _active = active; }

    protected:
        const std::string                    _selectorString;
        const std::unique_ptr<StyleSelector> _selector;
        Style                                _style;
        int                                  _weight{0};
        bool                                 _active{false};
    };
}
