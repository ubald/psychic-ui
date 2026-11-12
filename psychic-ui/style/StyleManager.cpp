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
            rule->style().clear();
        }
        _rules.clear();
        _rulesById.clear();
        _rulesByClass.clear();
        _rulesByTag.clear();
        _otherRules.clear();
        _universal = nullptr;
        _valid     = false;
    }

    StyleManager &StyleManager::loadFont(const std::string &name, const std::string &path) {
        _fonts.insert({name, SkTypeface::MakeFromFile(path.c_str())});
        _valid = false;
        return *this;
    }

    sk_sp<SkTypeface> StyleManager::font(const std::string &name) const {
        auto font = _fonts.find(name);
        return font != _fonts.cend() ? font->second : nullptr;
    }

    StyleManager &StyleManager::registerSkin(const std::string &name, SkinMaker hatcher) {
        _skins.insert(std::make_pair(name, std::move(hatcher)));
        _valid = false;
        return *this;
    }

    std::shared_ptr<internal::SkinBase> StyleManager::skin(const std::string &name) {
        auto skin = _skins.find(name);
        return skin != _skins.cend() ? skin->second->hatch() : nullptr;
    }

    Style &StyleManager::style(std::string selectorString) {
        std::transform(selectorString.begin(), selectorString.end(), selectorString.begin(), ::tolower);
        auto it = _declarations.find(selectorString);
        if (it == _declarations.cend()) {
            auto selector = StyleSelector::fromSelector(selectorString);
            if (!selector) {
                throw std::invalid_argument("Invalid style selector: \"" + selectorString + "\"");
            }
            auto declaration = std::make_unique<StyleDeclaration>(
                selectorString, std::move(selector),
                [](void *manager) { static_cast<StyleManager *>(manager)->_valid = false; }, this
            );
            it = _declarations.emplace(selectorString, std::move(declaration)).first;
        }

        StyleDeclaration *declaration = it->second.get();
        if (!declaration->active()) {
            // New, or dropped by reset(): it starts empty, after the rules declared so far
            declaration->style().clear();
            declaration->setActive(true);
            _rules.push_back(declaration);
            if (selectorString == "*") {
                _universal = declaration;
            } else {
                // Indexed by the rarest thing its rightmost part requires
                const StyleSelector *selector = declaration->selector();
                const std::size_t   index     = _rules.size() - 1;
                if (!selector->id().empty()) {
                    _rulesById[selector->id()].push_back(index);
                } else if (!selector->classes().empty()) {
                    _rulesByClass[selector->classes().front()].push_back(index);
                } else if (!selector->tag().empty()) {
                    _rulesByTag[selector->tag()].push_back(index);
                } else {
                    _otherRules.push_back(index);
                }
            }
            _valid = false;
        }
        return declaration->style();
    }

    void StyleManager::matchCandidates(const std::vector<std::size_t> &candidates, const Div *component) {
        for (std::size_t i: candidates) {
            if (_rules[i]->selector()->matches(component)) {
                _matches.emplace_back(_rules[i]->weight(), i);
            }
        }
    }

    void StyleManager::matchCandidates(const std::unordered_map<std::string, std::vector<std::size_t>> &index, const std::string &key, const Div *component) {
        auto it = index.find(key);
        if (it != index.cend()) {
            matchCandidates(it->second, component);
        }
    }

    void StyleManager::match(const Div *component) {
        _matches.clear();
        // Only the rules that can match: each is indexed once, under what its rightmost part requires
        matchCandidates(_otherRules, component);
        if (!_rulesById.empty()) {
            if (!component->id().empty()) {
                matchCandidates(_rulesById, component->id(), component);
            }
            matchCandidates(_rulesById, component->internalId(), component);
        }
        for (const auto &className: component->classNames()) {
            matchCandidates(_rulesByClass, className, component);
        }
        for (const auto &tag: component->tags()) {
            matchCandidates(_rulesByTag, tag, component);
        }
        // Heaviest overlaid last; on equal weight the rule declared last wins, like CSS's order of appearance.
        // (weight, declaration index) pairs are unique once the duplicates (a tag set twice) are dropped.
        std::sort(_matches.begin(), _matches.end());
        _matches.erase(std::unique(_matches.begin(), _matches.end()), _matches.end());
    }

    Style StyleManager::computeStyle(const Div *component) {
        Style s;

        // Start with global values
        if (_universal) {
            s.overlay(_universal->style());
        }

        // Apply inherited values
        if (component->_parent) {
            s.overlay(component->_parent->_computedStyle, component->inheritedProperties());
        }

        // Apply the matching rules
        match(component);
        for (const auto &[weight, index]: _matches) {
            s.overlay(_rules[index]->style());
        }

        // Overlay "inline" style
        s.overlay(component->_inlineStyle);

        // Set defaults (only after everything else, this way we only fill in the blanks)
        s.defaults(component->_defaultStyle);

        return s;
    }

    std::vector<std::string> StyleManager::matchedSelectors(const Div *component) {
        std::vector<std::string> selectors;
        if (_universal) {
            selectors.push_back("*");
        }
        match(component);
        for (const auto &[weight, index]: _matches) {
            selectors.push_back("[weight: " + std::to_string(weight) + "] " + _rules[index]->selectorString());
        }
        return selectors;
    }

}
