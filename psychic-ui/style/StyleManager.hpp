#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <utility>
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "psychic-ui/utils/Hatcher.hpp"
#include "psychic-ui/themes/Theme.hpp"
#include "psychic-ui/themes/one-dark.hpp"
#include "Style.hpp"
#include "StyleSelector.hpp"
#include "StyleSheet.hpp"
#include "StyleDeclaration.hpp"

namespace psychic_ui {
    class Div;

    namespace internal {
        class SkinBase;
    }

    using SkinType = Hatcher<std::shared_ptr<internal::SkinBase>>;
    using SkinMaker = std::shared_ptr<SkinType>;

    class StyleManager {
    public:
        /**
         * The style manager windows use unless given another one
         */
        static std::shared_ptr<StyleManager> getInstance();

        StyleManager() = default;
        StyleManager(const StyleManager &) = delete;
        StyleManager &operator=(const StyleManager &) = delete;

        /**
         * Forget the fonts, the skins, the loaded stylesheets and every rule.
         * The rules' styles are emptied but stay allocated, so a `Style &` obtained from `style()` stays valid:
         * declaring the same selector again (usually by loading a stylesheet) brings that rule back.
         */
        void reset();

        /**
         * Load a stylesheet: its rules apply after (and over, on equal weight) those loaded before it.
         * The style manager keeps it, to load it again when the theme changes; loading a stylesheet type
         * that is already loaded runs it again in its place.
         */
        template<typename T>
        void loadStyleSheet(bool reset = false) {
            static_assert(std::is_base_of<StyleSheet, T>::value, "T must extend StyleSheet");
            if (reset) {
                this->reset();
            }
            loadStyleSheet(std::type_index(typeid(T)), [] { return std::make_unique<T>(); });
        }

        /**
         * The theme stylesheets read (`manager->theme().palette.accent`), One Dark until another is set
         */
        const Theme &theme() const { return _theme; }

        /**
         * Switch themes: the rules are cleared and the loaded stylesheets load again, in the same order, with the
         * new values. Fonts and skins stay; the next style pass recomputes every div.
         */
        StyleManager &theme(const Theme &theme);

        bool valid() const { return _valid; };

        void setValid() { _valid = true; }

        /**
         * The platform's font manager (Fontconfig on Linux, CoreText on macOS), shared by every
         * style manager: it loads font files and finds the system's fonts
         */
        static sk_sp<SkFontMgr> fontManager();

        /**
         * Loads a font file under a name, which `fontFamily` styles use
         */
        StyleManager &loadFont(const std::string &name, const std::string &path);
        /**
         * The font loaded under that name, otherwise the system font of that family, otherwise
         * the system default
         */
        sk_sp<SkTypeface> font(const std::string &name) const;

        /**
         * What text layout finds fonts in: the loaded fonts by name, then the system's fonts,
         * which also fill in the characters a font lacks. Remade after a font is loaded.
         */
        sk_sp<skia::textlayout::FontCollection> fontCollection();

        StyleManager &registerSkin(const std::string &name, SkinMaker hatcher);
        std::shared_ptr<internal::SkinBase> skin(const std::string &name);

        /**
         * Get the rule for a selector, declaring it if needed. Selectors are case insensitive.
         * Rules apply like CSS: by specificity (weight), and on equal specificity the rule declared last wins.
         *
         *     manager->style("Button:hover").backgroundColor(rgb(0x3C414F));
         *
         * @throws std::invalid_argument when the selector can't be parsed
         */
        Style &style(std::string selector);

        /**
         * The style a Div gets, from (lowest priority first) the `*` rule, what it inherits from its parent's
         * computed style, the rules matching it, its inline style, and its default style for what is still unset
         */
        Style computeStyle(const Div *component);

        /**
         * The selectors of the rules matching a Div, in the order they apply. For debugging.
         */
        std::vector<std::string> matchedSelectors(const Div *component);

    protected:
        /**
         * Every rule ever declared, by lowercase selector. Never shrinks, so that a `Style &` handed out stays valid.
         */
        std::unordered_map<std::string, std::unique_ptr<StyleDeclaration>> _declarations{};
        /**
         * The rules in effect, in declaration order: on equal weight, a later rule wins
         */
        std::vector<StyleDeclaration *>                                    _rules{};
        /**
         * The `*` rule, if declared
         */
        StyleDeclaration                                                   *_universal{nullptr};
        /**
         * The rules (indices in _rules) by what their rightmost part requires: an id, else its first class,
         * else its tag. A Div only needs testing against the rules indexed under its id, classes and tags,
         * plus those that require none of them (pseudo-classes only).
         */
        std::unordered_map<std::string, std::vector<std::size_t>>          _rulesById{};
        std::unordered_map<std::string, std::vector<std::size_t>>          _rulesByClass{};
        std::unordered_map<std::string, std::vector<std::size_t>>          _rulesByTag{};
        std::vector<std::size_t>                                           _otherRules{};
        /**
         * Weight and index in _rules of the rules matching the Div being computed, kept to reuse its memory
         */
        std::vector<std::pair<int, std::size_t>>                           _matches{};
        std::unordered_map<std::string, sk_sp<SkTypeface>>                 _fonts{};
        sk_sp<skia::textlayout::FontCollection>                            _fontCollection{nullptr};
        std::unordered_map<std::string, SkinMaker>                         _skins{};
        bool                                                               _valid{false};
        Theme                                                              _theme{themes::oneDark};
        /**
         * The loaded stylesheets, in load order, with their type so that each type is loaded once
         */
        std::vector<std::pair<std::type_index, std::unique_ptr<StyleSheet>>> _styleSheets{};

        void loadStyleSheet(std::type_index type, const std::function<std::unique_ptr<StyleSheet>()> &make);

        /**
         * Drop every rule, keeping the fonts, the skins and the stylesheets
         */
        void clearRules();

        /**
         * Fill _matches with the rules matching a Div, in the order they apply
         */
        void match(const Div *component);

        /**
         * Add the candidates of one index bucket that match the Div to _matches
         */
        void matchCandidates(const std::vector<std::size_t> &candidates, const Div *component);
        void matchCandidates(const std::unordered_map<std::string, std::vector<std::size_t>> &index, const std::string &key, const Div *component);
    };
}
