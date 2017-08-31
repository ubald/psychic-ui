#include "StyleDeclaration.hpp"

namespace psychicui {

    StyleDeclaration::StyleDeclaration(std::unique_ptr<Rule> rule) :
        _rule(std::move(rule)),
        _style(std::make_unique<Style>()) {
        // Compute weight
        _weight = _rule->weight();
    }

    const Rule *StyleDeclaration::rule() const {
        return _rule.get();
    }

    Style *StyleDeclaration::style() const {
        return _style.get();
    }

    const int StyleDeclaration::weight() const {
        return _weight;
    }
}