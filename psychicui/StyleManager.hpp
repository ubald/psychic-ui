#pragma once

#include <map>
#include <string>
#include <memory>
#include <psychicui/psychicui.hpp>
#include "Style.hpp"
#include "Component.hpp"
#include "StyleDeclaration.hpp"

namespace psychicui {
    class StyleManager {
    public:
        static void parseStyleName(const std::string &styleName, std::string &type, std::vector<std::string> &classes, std::string &pseudo);
        Style *getStyle(std::string styleName);
        Style *computeStyle(std::string styleName);
        Style *computeStyle(const Component *component);
        std::shared_ptr<Style> getTypeStyle(const std::string &typeName);
        std::shared_ptr<Style> getTypeStyle(const std::string &typeName, const std::string &className);
        std::shared_ptr<Style> getClassStyle(const std::string &className);
    protected:
        std::map<std::string, std::unique_ptr<StyleDeclaration>>             _declarations;
        std::map<std::string, std::map<std::string, std::shared_ptr<Style>>> typeStyles{};
        std::map<std::string, std::shared_ptr<Style>>                        classStyles{};
    };
}
