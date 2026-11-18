#include <algorithm>
#include <stdexcept>
#include <vector>
#include "StyleManager.hpp"
#include "../Div.hpp"
#include "modules/skparagraph/include/TypefaceFontProvider.h"

#if defined(__APPLE__)
#include "include/ports/SkFontMgr_mac_ct.h"
#elif defined(_WIN32)
#include "include/ports/SkTypeface_win.h"
#else
#include "include/ports/SkFontMgr_fontconfig.h"
#include "include/ports/SkFontScanner_FreeType.h"
#endif

namespace psychic_ui {

    std::shared_ptr<StyleManager> StyleManager::getInstance() {
        // Built on first use, so it doesn't depend on the initialization order of other translation units
        static const std::shared_ptr<StyleManager> instance = std::make_shared<StyleManager>();
        return instance;
    }

    void StyleManager::reset() {
        _fonts.clear();
        _fontCollection = nullptr;
        _skinSets.clear();
        _skins.clear();
        _styleSheets.clear();
        clearRules();
    }

    void StyleManager::clearRules() {
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
        invalidate();
    }

    void StyleManager::loadStyleSheet(const std::type_index type, const std::function<std::unique_ptr<StyleSheet>()> &make) {
        auto it = std::find_if(
            _styleSheets.begin(), _styleSheets.end(),
            [type](const auto &loaded) { return loaded.first == type; }
        );
        StyleSheet *sheet = it != _styleSheets.end()
                            ? it->second.get()
                            : _styleSheets.emplace_back(type, make()).second.get();
        sheet->load(this);
        invalidate();
    }

    StyleManager &StyleManager::theme(const Theme &theme) {
        _theme = theme;
        reloadRules();
        return *this;
    }

    bool StyleManager::density(const Density density) {
        if (density == _density) {
            return false;
        }
        _density = density;
        reloadRules();
        return true;
    }

    StyleManager &StyleManager::skinSet(std::unique_ptr<SkinSet> skinSet, const ColorScheme scheme) {
        installSkinSet(std::move(skinSet));
        if (!_skinSets.empty()) {
            _theme = _skinSets.back()->theme(scheme);
        }
        reloadRules();
        return *this;
    }

    StyleManager &StyleManager::skinSet(std::unique_ptr<SkinSet> skinSet, const Theme theme) {
        installSkinSet(std::move(skinSet));
        _theme = theme;
        reloadRules();
        return *this;
    }

    void StyleManager::installSkinSet(std::unique_ptr<SkinSet> skinSet) {
        // The chain, from the last fallback to the set itself: a set's skins and rules go over its fallback's
        _skinSets.clear();
        while (skinSet) {
            std::unique_ptr<SkinSet> fallback = skinSet->fallback();
            _skinSets.insert(_skinSets.begin(), std::move(skinSet));
            skinSet = std::move(fallback);
        }
        _skins.clear();
        for (const auto &set: _skinSets) {
            set->registerSkins(_skins);
        }
        ++_skinsGeneration;
    }

    void StyleManager::reloadRules() {
        clearRules();
        for (const auto &set: _skinSets) {
            set->load(this);
        }
        for (const auto &[type, sheet]: _styleSheets) {
            sheet->load(this);
        }
        invalidate();
    }

    sk_sp<SkFontMgr> StyleManager::fontManager() {
        static const sk_sp<SkFontMgr> manager =
            #if defined(__APPLE__)
            SkFontMgr_New_CoreText(nullptr);
            #elif defined(_WIN32)
            SkFontMgr_New_DirectWrite();
            #else
            SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());
            #endif
        return manager;
    }

    StyleManager &StyleManager::loadFont(const std::string &name, const std::string &path) {
        if (_fonts.contains(name)) {
            return *this; // Stylesheets load their fonts again on every replay: once is enough
        }
        std::filesystem::path file{path};
        if (file.is_relative()) {
            file = _resourceRoot / file;
        }
        _fonts.insert({name, fontManager()->makeFromFile(file.string().c_str())});
        _fontCollection = nullptr;
        invalidate();
        return *this;
    }

    StyleManager &StyleManager::resourceRoot(std::filesystem::path root) {
        _resourceRoot = std::move(root);
        return *this;
    }

    const std::filesystem::path &StyleManager::resourceRoot() const {
        return _resourceRoot;
    }

    sk_sp<SkTypeface> StyleManager::font(const std::string &name) const {
        auto font = _fonts.find(name);
        if (font != _fonts.cend() && font->second) {
            return font->second;
        }
        // Not loaded (or its file didn't load): the system's font of that family, or its default font. Skia draws nothing
        // with a null typeface.
        if (auto system = fontManager()->matchFamilyStyle(name.c_str(), SkFontStyle())) {
            return system;
        }
        return fontManager()->legacyMakeTypeface(nullptr, SkFontStyle());
    }

    sk_sp<skia::textlayout::FontCollection> StyleManager::fontCollection() {
        if (!_fontCollection) {
            auto provider = sk_make_sp<skia::textlayout::TypefaceFontProvider>();
            for (const auto &[name, typeface]: _fonts) {
                if (typeface) {
                    provider->registerTypeface(typeface, SkString(name.c_str()));
                }
            }
            _fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
            _fontCollection->setAssetFontManager(std::move(provider));
            // An unknown family falls back to the system's default font: "sans-serif" (SkParagraph's default family,
            // which Fontconfig knows), then the font manager's own default family, since DirectWrite and CoreText have
            // no generic names. Without it SkParagraph would take the first loaded font.
            std::vector<SkString> defaultFamilies{SkString{"sans-serif"}};
            if (auto typeface = fontManager()->legacyMakeTypeface(nullptr, SkFontStyle())) {
                SkString family;
                typeface->getFamilyName(&family);
                defaultFamilies.push_back(family);
            }
            _fontCollection->setDefaultFontManager(fontManager(), defaultFamilies);
            _fontCollection->enableFontFallback();
        }
        return _fontCollection;
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
                [](void *manager) { static_cast<StyleManager *>(manager)->invalidate(); }, this
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
            invalidate();
        }
        return declaration->style();
    }

    void StyleManager::matchCandidates(const std::vector<std::size_t> &candidates, const Div *component) {
        for (const std::size_t i: candidates) {
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
