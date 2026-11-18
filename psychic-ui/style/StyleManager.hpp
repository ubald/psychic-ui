#pragma once

#include <filesystem>
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
#include "psychic-ui/skins/SkinSet.hpp"
#include "psychic-ui/themes/Theme.hpp"
#include "psychic-ui/themes/pivotal.hpp"
#include "psychic-ui/utils/ExecutablePath.hpp"
#include "Style.hpp"
#include "StyleSelector.hpp"
#include "StyleSheet.hpp"
#include "StyleDeclaration.hpp"

namespace psychic_ui {
    class Div;

    class StyleManager {
    public:
        /**
         * The style manager windows use unless given another one
         */
        static std::shared_ptr<StyleManager> getInstance();

        StyleManager() = default;
        StyleManager(const StyleManager &) = delete;
        StyleManager &operator=(const StyleManager &) = delete;
        StyleManager(StyleManager &&) = delete;
        StyleManager &operator=(StyleManager &&) = delete;
        ~StyleManager() = default;

        /**
         * Forget the fonts, the skin set, the loaded stylesheets and every rule.
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
            static_assert(std::is_base_of_v<StyleSheet, T>, "T must extend StyleSheet");
            if (reset) {
                this->reset();
            }
            if constexpr (std::is_base_of_v<SkinSet, T>) {
                skinSet<T>();
            } else {
                loadStyleSheet(std::type_index(typeid(T)), [] { return std::make_unique<T>(); });
            }
        }

        /**
         * Use a skin set, with its default theme: its skins (and its fallback sets') replace the current set's,
         * its rules load first, under the other stylesheets, and every component hatches a new skin on the next
         * style pass. Loading a skin set with `loadStyleSheet` does the same.
         */
        template<class S>
        StyleManager &skinSet() {
            static_assert(std::is_base_of_v<SkinSet, S>, "S must extend SkinSet");
            return skinSet(std::make_unique<S>());
        }

        StyleManager &skinSet(std::unique_ptr<SkinSet> skinSet, ColorScheme scheme = ColorScheme::Unknown);

        /**
         * Use a skin set with that theme rather than its own
         */
        StyleManager &skinSet(std::unique_ptr<SkinSet> skinSet, Theme theme);

        /**
         * The skin set in use, nullptr before one is set
         */
        const SkinSet *skinSet() const { return _skinSets.empty() ? nullptr : _skinSets.back().get(); }

        /**
         * A new skin implementing SkinT from the skin set: the variant (from the set, else its fallback sets),
         * else SkinT's default. nullptr only without a skin set, or when no set in the chain implements SkinT.
         */
        template<class SkinT>
        std::shared_ptr<SkinT> skin(std::string_view variant = "") const {
            return _skins.make<SkinT>(variant);
        }

        /**
         * Bumped every time the skin set changes: components hatch a new skin when it does
         */
        std::size_t skinsGeneration() const { return _skinsGeneration; }

        /**
         * Bumped every time every style is to be computed again: a rule set or changed, a stylesheet or a font loaded,
         * the rules loaded again (a theme, a density or a skin set). Styles computed in such a pass change at once,
         * without transitions: those follow a div's own state (classes, pseudo-classes, its inline style).
         */
        std::size_t rulesGeneration() const { return _rulesGeneration; }

        /**
         * The theme stylesheets read (`manager->theme().palette.accent`), Pivotal Dark until another is set
         */
        const Theme &theme() const { return _theme; }

        /**
         * Switch themes: the rules are cleared and the loaded stylesheets load again, in the same order, with the
         * new values. Fonts and skins stay; the next style pass recomputes every div.
         */
        StyleManager &theme(const Theme &theme);

        /**
         * The density the stylesheets size controls for: `Pointer` until a window goes to touch mode
         */
        Density density() const { return _density; }

        /**
         * Switch densities, like themes: the stylesheets load again and read the new `sizes()`.
         * Nothing happens when the density doesn't change.
         * @return Whether it changed
         */
        bool density(Density density);

        /**
         * The theme's sizes for the current density
         */
        const Sizes &sizes() const { return _theme.metrics.sizes(_density); }

        bool valid() const { return _valid; };

        void setValid() { _valid = true; }

        /**
         * Recompute every div's style on the next style pass, without touching the rules
         */
        void invalidate() {
            _valid = false;
            ++_rulesGeneration;
        }

        /**
         * The platform's font manager (Fontconfig on Linux, CoreText on macOS), shared by every
         * style manager: it loads font files and finds the system's fonts
         */
        static sk_sp<SkFontMgr> fontManager();

        /**
         * Loads a font file under a name, which `fontFamily` styles use. A relative path is resolved against
         * `resourceRoot()`, an absolute one is used as it is
         */
        StyleManager &loadFont(const std::string &name, const std::string &path);
        /**
         * The directory relative resource paths (fonts) resolve against: the executable's directory by default,
         * where `psychic_ui_copy_resources` copies `res/`. Set it before installing a skin set: a font is loaded once
         */
        StyleManager &resourceRoot(std::filesystem::path root);
        const std::filesystem::path &resourceRoot() const;
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
        std::filesystem::path                                              _resourceRoot{executableDirectory()};
        std::unordered_map<std::string, sk_sp<SkTypeface>>                 _fonts{};
        sk_sp<skia::textlayout::FontCollection>                            _fontCollection{nullptr};
        /**
         * The skin set in use, last, after the sets it falls back on
         */
        std::vector<std::unique_ptr<SkinSet>>                              _skinSets{};
        SkinRegistry                                                       _skins{};
        std::size_t                                                        _skinsGeneration{0};
        std::size_t                                                        _rulesGeneration{0};
        bool                                                               _valid{false};
        Theme                                                              _theme{themes::pivotalDark};
        Density                                                            _density{Density::Pointer};
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
         * Clear the rules, then load the skin sets' rules and the stylesheets', in order
         */
        void reloadRules();

        /**
         * Replace the skin set chain and the skins, bumping the generation; the rules are reloaded by the caller
         */
        void installSkinSet(std::unique_ptr<SkinSet> skinSet);

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
