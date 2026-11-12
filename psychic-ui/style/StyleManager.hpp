#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include <utility>
#include <SkTypeface.h>
#include "psychic-ui/utils/Hatcher.hpp"
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
         * Forget the fonts, the skins and every rule.
         * The rules' styles are emptied but stay allocated, so a `Style &` obtained from `style()` stays valid:
         * declaring the same selector again (usually by loading a stylesheet) brings that rule back.
         */
        void reset();

        template<typename T>
        void loadStyleSheet(bool reset = false) {
            static_assert(std::is_base_of<StyleSheet, T>::value, "T must extend StyleSheet");
            if (reset) {
                this->reset();
            }
            std::make_unique<T>()->load(this);
            _valid = false;
        }

        bool valid() const { return _valid; };

        void setValid() { _valid = true; }

        StyleManager &loadFont(const std::string &name, const std::string &path);
        sk_sp<SkTypeface> font(const std::string &name) const;

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
         * Weight and index in _rules of the rules matching the Div being computed, kept to reuse its memory
         */
        std::vector<std::pair<int, std::size_t>>                           _matches{};
        std::unordered_map<std::string, sk_sp<SkTypeface>>                 _fonts{};
        std::unordered_map<std::string, SkinMaker>                         _skins{};
        bool                                                               _valid{false};

        /**
         * Fill _matches with the rules matching a Div, in the order they apply
         */
        void match(const Div *component);
    };
}
