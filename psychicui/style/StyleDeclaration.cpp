#include "StyleDeclaration.hpp"

namespace psychicui {

    StyleDeclaration::StyleDeclaration(std::unique_ptr<StyleSelector> selector) :
        StyleDeclaration(std::move(selector), nullptr) {}

    StyleDeclaration::StyleDeclaration(std::unique_ptr<StyleSelector> selector, const std::function<void()> &onChanged) :
        _selector(std::move(selector)),
        _style(std::make_unique<Style>(onChanged)),
        _onChanged(onChanged) {

        // Compute weight
        _weight = _selector->weight();
    }

    const StyleSelector *StyleDeclaration::selector() const {
        return _selector.get();
    }

    Style *StyleDeclaration::style() const {
        return _style.get();
    }

    const int StyleDeclaration::weight() const {
        return _weight;
    }
}