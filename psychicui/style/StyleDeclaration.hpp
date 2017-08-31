#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Style.hpp"
#include "StyleRule.hpp"

namespace psychicui {
    class StyleDeclaration {
    public:
        explicit StyleDeclaration(std::unique_ptr<Rule> rule);
        const Rule *rule() const;
        Style *style() const;

        /**
         * Returns the cached weight computed from the rule chain
         * @return int
         */
        const int weight() const;
    protected:
        const std::unique_ptr<Rule> _rule{nullptr};
        std::unique_ptr<Style>      _style;
        int                         _weight{0};

        #ifdef DEBUG_STYLES
    public:
        std::string selector{};
        #endif
    };
}
