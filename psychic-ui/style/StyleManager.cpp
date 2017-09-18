#include <iostream>
#include <algorithm>
#include "psychic-ui/Div.hpp"
#include "StyleManager.hpp"
#include "psychic-ui/utils/StringUtils.hpp"

namespace psychic_ui {

    std::shared_ptr<StyleManager> StyleManager::instance{nullptr};

    std::shared_ptr<StyleManager> StyleManager::getInstance() {
        if (!instance) {
            instance = std::make_shared<StyleManager>();
        }
        return instance;
    }

    StyleManager::StyleManager() :
        _declarations{&_styleSheetDeclarations} {}

    void StyleManager::setRestorePoint() {
        _declarations = &_runtimeDeclarations;
    }

    StyleManager *StyleManager::loadFont(const std::string &name, const std::string &path) {
        _fonts.insert({name, SkTypeface::MakeFromFile(path.c_str())});
        _valid = false;
        return this;
    }

    const sk_sp<SkTypeface> StyleManager::font(const std::string &name) const {
        auto font = _fonts.find(name);
        return font != _fonts.cend() ? font->second : nullptr;
    }

    StyleManager *StyleManager::registerSkin(const std::string &name, SkinMaker hatcher) {
        _skins.insert(std::make_pair(name, std::move(hatcher)));
        _valid = false;
        return this;
    }

    std::shared_ptr<internal::SkinBase> StyleManager::skin(const std::string &name) {
        auto skin = _skins.find(name);
        return skin != _skins.cend() ? skin->second->hatch() : nullptr;
    }

    Style *StyleManager::style(std::string selectorString) {
        std::transform(selectorString.begin(), selectorString.end(), selectorString.begin(), ::tolower);
        auto it = _declarations->find(selectorString);
        if (it != _declarations->cend()) {
            return it->second->style();
        } else {
            auto selector = StyleSelector::fromSelector(selectorString);
            if (!selector) {
                std::cerr << "Invalid selector: \"" << selectorString << "\", returning dummy style.";
                return Style::dummyStyle.get();
            }

            (*_declarations)[selectorString] = std::make_unique<StyleDeclaration>(
                std::move(selector),
                [this]() { _valid = false; }
            );

            #ifdef DEBUG_STYLES
            (*_declarations)[selectorString]->selectorString = selectorString;
            #endif

            return (*_declarations)[selectorString]->style();
        }
    }

    std::unique_ptr<Style> StyleManager::computeStyle(const Div *component) {
        std::vector<std::pair<int, StyleDeclaration *>> directMatches;

        // Start with global values
        auto s = std::make_unique<Style>(style("*"));

        // Apply inherited values
        if (component->_parent) {
            s->overlayInheritable(component->_parent->_computedStyle.get(), component);

            #ifdef DEBUG_STYLES
            s->declarations.insert(
                s->declarations.cend(),
                component->_parent->_computedStyle->declarations.cbegin(),
                component->_parent->_computedStyle->declarations.cend()
            );
            #endif
        }

        // Get Direct matches
        for (const auto &declaration: _styleSheetDeclarations) {
            if (declaration.second->selector()->matches(component)) {
                directMatches.emplace_back(declaration.second->weight(), declaration.second.get());
            }
        }

        //for (const auto &declaration: _runtimeDeclarations) {
        //    if (declaration.second->selector()->matches(component)) {
        //        directMatches.emplace_back(declaration.second->weight(), declaration.second.get());
        //    }
        //}

        // Sort direct matches by weight, heaviest overlaid last
        std::sort(
            directMatches.begin(), directMatches.end(), [](const auto &a, const auto &b) {
                return a.first < b.first;
            }
        );

        // Apply direct matches
        for (const auto &directMatch: directMatches) {
            s->overlay(directMatch.second->style());

            #ifdef DEBUG_STYLES
            s->declarations
             .push_back("[weight: " + std::to_string(directMatch.first) + "] " + directMatch.second->selectorString);
            #endif
        }

        // Overlay "inline" style
        s->overlay(component->_inlineStyle.get());

        // Set defaults (only after everything else, this way we only fill in the blanks)
        s->defaults(component->_defaultStyle.get());

        return s;
    }

}
