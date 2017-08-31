#pragma once

#include <map>
#include <string>
#include <memory>
#include "../psychicui.hpp"
#include "Style.hpp"
#include "StyleRule.hpp"
#include "StyleSheet.hpp"
#include "StyleDeclaration.hpp"

namespace psychicui {
    class Component;

    class StyleManager {
    public:
        static std::shared_ptr<StyleManager> instance;
        static std::shared_ptr<StyleManager> getInstance();

        void loadFont(const std::string &name, const std::string &path);
        sk_sp<SkTypeface> font(const std::string &name) const;

        Style *style(std::string selector);
        std::unique_ptr<Style> computeStyle(const Component *component);

//        /**
//         * Load an instance of StyleSheet T
//         * @tparam T :StyleSheet
//         */
//        template <typename T>
//        void load() {
//            static_assert(std::is_base_of<StyleSheet, T>::value, "T must extend StyleSheet");
//            std::make_unique<T>()->load(this);
//        }

    protected:
        std::map<std::string, std::unique_ptr<StyleDeclaration>> _declarations{};
        std::map<std::string, sk_sp<SkTypeface>>                 _fonts{};
    };
}
