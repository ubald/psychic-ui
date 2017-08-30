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
    protected:
        const std::unique_ptr<Rule> _rule{nullptr};
        std::unique_ptr<Style> _style;
    };
}
