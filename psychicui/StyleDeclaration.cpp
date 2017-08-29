#include "StyleDeclaration.hpp"

#include <utility>

namespace psychicui {

    StyleDeclaration::StyleDeclaration(std::string type, std::vector<std::string> classes, std::string pseudo) :
        _type(std::move(type)),
        _classes(std::move(classes)),
        _pseudo(std::move(pseudo)),
        _style(std::move(Style::getDefaultStyle())) {

    }

    const std::string StyleDeclaration::type() const {
        return _type;
    }

    const std::vector<std::string> StyleDeclaration::classes() const {
        return _classes;
    }

    const std::string StyleDeclaration::pseudo() const {
        return _pseudo;
    }

    Style *StyleDeclaration::style() const {
        return _style.get();
    }
}