#include "StyleDeclaration.hpp"

namespace psychic_ui {

    StyleDeclaration::StyleDeclaration(std::string selectorString, std::unique_ptr<StyleSelector> selector, Style::ChangedCallback onChanged, void *owner) :
        _selectorString(std::move(selectorString)),
        _selector(std::move(selector)),
        _style(onChanged, owner),
        _weight(_selector->weight()) {}

}
