#include <iostream>
#include "StyleManager.hpp"
#include "psychicui/utils/StringUtils.hpp"

namespace psychicui {

    std::shared_ptr<StyleManager> StyleManager::instance{nullptr};

    std::shared_ptr<StyleManager> StyleManager::getInstance() {
        if (!instance) {
            instance = std::make_shared<StyleManager>();
        }
        return instance;
    }

    void StyleManager::loadFont(const std::string &name, const std::string &path) {
        _fonts[name] = SkTypeface::MakeFromFile(path.c_str());
    }

    sk_sp<SkTypeface> StyleManager::getFont(const std::string &name) const {
        auto font = _fonts.find(name);
        return font != _fonts.end() ? font->second : nullptr;
    }

    Style *StyleManager::getStyle(std::string selector) {
        std::transform(selector.begin(), selector.end(), selector.begin(), ::tolower);
        auto it = _declarations.find(selector);
        if (it != _declarations.end()) {
            return it->second->style();
        } else {
            auto rule = Rule::fromSelector(selector);
            if (!rule) {
                std::cerr << "Invalid selector: \"" << selector << "\", returning dummy style.";
                return Style::dummyStyle.get();
            }

            _declarations[selector] = std::make_unique<StyleDeclaration>(std::move(rule));
            return _declarations[selector]->style();
        }
    }

    std::unique_ptr<Style> StyleManager::computeStyle(const Component *component) {
        std::vector<StyleDeclaration*> inheritedMatches;
        std::vector<StyleDeclaration*> directMatches;
        for (const auto &declaration: _declarations) {
            switch (declaration.second->rule()->matches(component)) {
                case direct:
                    directMatches.push_back(declaration.second.get());
                    break;
                case inherited:
                    inheritedMatches.push_back(declaration.second.get());
                    break;
                default:
                    break;
            }
        }

        // TODO: Somehow sort matching styles

        auto style = std::make_unique<Style>();
        // Computed inherited values
        for (const auto &inheritedMatch: inheritedMatches) {
            style->overlayInheritable(inheritedMatch->style());
        }
        for (const auto &directMatch: directMatches) {
            style->overlay(directMatch->style());
        }
        // Fill holes with default style
        style->defaults(getStyle("*"));

        return std::move(style);
    }

}