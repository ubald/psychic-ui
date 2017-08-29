#include <iostream>
#include <algorithm>
#include "StyleManager.hpp"
#include "utils/StringUtils.hpp"

namespace psychicui {

    void StyleManager::parseStyleName(const std::string &styleName, std::string &type, std::vector<std::string> &classes, std::string &pseudo) {
        auto parts     = split(styleName, '.', false);
        auto partCount = parts.size();
        if (partCount > 0) {
            {
                // Find pseudp
                auto pseudoParts = split(parts.back(), ':', false);
                if (pseudoParts.size() > 1) {
                    pseudo = pseudoParts.back();
                    parts.back() = pseudoParts.front();
                }
            }

            // Might be empty but we don't care
            // It also can just hold garbage if the styleName is not properly formatted
            // but we don't care about it for now
            type = parts[0];

            // Get classes
            classes = std::vector<std::string>(parts.begin() + 1, parts.end());
        }
    }

    Style *StyleManager::getStyle(std::string styleName) {
        std::transform(styleName.begin(), styleName.end(), styleName.begin(), ::tolower);
        auto it = _declarations.find(styleName);
        if (it != _declarations.end()) {
            return it->second->style();
        } else {
            std::string              type;
            std::vector<std::string> classes;
            std::string              pseudo;
            parseStyleName(styleName, type, classes, pseudo);
            std::unique_ptr<StyleDeclaration> declaration = std::make_unique<StyleDeclaration>(type, classes, pseudo);
            Style                             *style      = declaration->style();
            _declarations[styleName] = std::move(declaration);
            return style;
        }
    }

    Style *StyleManager::computeStyle(std::string styleName) {
        std::string              type;
        std::vector<std::string> classes;
        std::string              pseudo;
        parseStyleName(styleName, type, classes, pseudo);
        return Style::getDefaultStyle();
    }

    Style *StyleManager::computeStyle(const Component *component) {
        Style * style{nullptr};
        if (component->_parent) {
            std::cout << "PARENT" << std::endl;
            style = computeStyle(component->_parent);
        } else {
            std::cout << "DEFAULT" << std::endl;
            style = Style::getDefaultStyle();
        }

        // Here we have access to all the class names higher up the hierarchy, meaning that a class added to a Label for
        // example is still considered when finding the style for the base Component
//        for (const auto &typeName: component->_componentType) {
//            style->overlay(getTypeStyle(typeName));
//            for (const auto &className: component->_classNames) {
//                style->overlay(getTypeStyle(typeName, className));
//                style->overlay(getClassStyle(className));
//            }
//        }
        return style;
    }

    std::shared_ptr<Style> StyleManager::getTypeStyle(const std::string &typeName) {
        try {
            return typeStyles.at(typeName).at("*");
        } catch (const std::out_of_range &exception) {
            return nullptr;
        }
    }

    std::shared_ptr<Style> StyleManager::getTypeStyle(const std::string &typeName, const std::string &className) {
        try {
            return typeStyles.at(typeName).at(className);
        } catch (const std::out_of_range &exception) {
            return nullptr;
        }
    }

    std::shared_ptr<Style> StyleManager::getClassStyle(const std::string &className) {
        try {
            return classStyles.at(className);
        } catch (const std::out_of_range &exception) {
            return nullptr;
        }
    }
}