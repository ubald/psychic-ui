#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Style.hpp"

namespace psychicui {
    class StyleDeclaration {
    public:
        StyleDeclaration(std::string type, std::vector<std::string> classes, std::string pseudo);
        const std::string type() const;
        const std::vector<std::string> classes() const;
        const std::string pseudo() const;
        Style * style() const;
    protected:
        std::string              _type;
        std::vector<std::string> _classes;
        std::string              _pseudo;
        std::unique_ptr<Style>   _style;
    };
}
