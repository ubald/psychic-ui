#include "StyleDeclaration.hpp"

namespace psychicui {

    StyleDeclaration::StyleDeclaration(std::unique_ptr<Rule> rule) :
        _rule(std::move(rule)),
        _style(std::make_unique<Style>()) {

    }

    const Rule *StyleDeclaration::rule() const {
        return _rule.get();
    }

    Style *StyleDeclaration::style() const {
        return _style.get();
    }
}