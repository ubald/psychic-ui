#include <algorithm>
#include <stdexcept>
#include "StyleManager.hpp"
#include "../Div.hpp"

namespace psychic_ui {

    std::shared_ptr<StyleManager> StyleManager::getInstance() {
        // Built on first use, so it doesn't depend on the initialization order of other translation units
        static std::shared_ptr<StyleManager> instance = std::make_shared<StyleManager>();
        return instance;
    }

    void StyleManager::reset() {
        _fonts.clear();
        _skins.clear();
        for (auto *rule: _rules) {
            rule->setActive(false);
            rule->style()->clear();
        }
        _rules.clear();
        _valid = false;
    }

    StyleManager *StyleManager::loadFont(const std::string &name, const std::string &path) {
        _fonts.insert({name, SkTypeface::MakeFromFile(path.c_str())});
        _valid = false;
        return this;
    }

    sk_sp<SkTypeface> StyleManager::font(const std::string &name) const {
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
        auto it = _declarations.find(selectorString);
        if (it == _declarations.cend()) {
            auto selector = StyleSelector::fromSelector(selectorString);
            if (!selector) {
                throw std::invalid_argument("Invalid style selector: \"" + selectorString + "\"");
            }
            auto declaration = std::make_unique<StyleDeclaration>(std::move(selector), [this]() { _valid = false; });
            #ifdef DEBUG_STYLES
            declaration->selectorString = selectorString;
            #endif
            it = _declarations.emplace(selectorString, std::move(declaration)).first;
        }

        StyleDeclaration *declaration = it->second.get();
        if (!declaration->active()) {
            // New, or dropped by reset(): it starts empty, after the rules declared so far
            declaration->style()->clear();
            declaration->setActive(true);
            _rules.push_back(declaration);
            _valid = false;
        }
        return declaration->style();
    }

    std::unique_ptr<Style> StyleManager::computeStyle(const Div *component) {
        // Weight, then index in _rules: the declaration order breaks ties
        std::vector<std::pair<int, std::size_t>> directMatches;

        // Start with global values
        auto universal = _declarations.find("*");
        auto s         = std::make_unique<Style>(
            universal != _declarations.cend() && universal->second->active() ? universal->second->style() : nullptr
        );

        // Apply inherited values
        if (component->_parent) {
            s->overlayInheritable(component->_parent->_computedStyle.get(), component);

            #ifdef DEBUG_STYLES
            s->declarations.insert(
                s->declarations.end(),
                component->_parent->_computedStyle->declarations.cbegin(),
                component->_parent->_computedStyle->declarations.cend()
            );
            #endif
        }

        // Get direct matches
        for (std::size_t i = 0; i < _rules.size(); ++i) {
            if (_rules[i]->selector()->matches(component)) {
                directMatches.emplace_back(_rules[i]->weight(), i);
            }
        }

        // Heaviest overlaid last; on equal weight the rule declared last wins, like CSS's order of appearance
        std::sort(directMatches.begin(), directMatches.end());

        // Apply direct matches
        for (const auto &[weight, index]: directMatches) {
            s->overlay(_rules[index]->style());

            #ifdef DEBUG_STYLES
            s->declarations.push_back("[weight: " + std::to_string(weight) + "] " + _rules[index]->selectorString);
            #endif
        }

        // Overlay "inline" style
        s->overlay(component->_inlineStyle.get());

        // Set defaults (only after everything else, this way we only fill in the blanks)
        s->defaults(component->_defaultStyle.get());

        return s;
    }

}
