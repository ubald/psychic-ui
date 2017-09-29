#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <type_traits>
#include "psychic-ui/psychic-ui.hpp"
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
        static std::shared_ptr<StyleManager> instance;
        static std::shared_ptr<StyleManager> getInstance();

        StyleManager() = default;

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

        StyleManager *loadFont(const std::string &name, const std::string &path);
        const sk_sp<SkTypeface> font(const std::string &name) const;

        StyleManager *registerSkin(const std::string &name, SkinMaker hatcher);
        std::shared_ptr<internal::SkinBase> skin(const std::string &name);

        Style *style(std::string selector);
        std::unique_ptr<Style> computeStyle(const Div *component);

    protected:
        std::unordered_map<std::string, std::unique_ptr<StyleDeclaration>> _declarations{};
        std::unordered_map<std::string, sk_sp<SkTypeface>>                 _fonts{};
        std::unordered_map<std::string, SkinMaker>                         _skins{};
        bool                                                               _valid{false};
    };
}
