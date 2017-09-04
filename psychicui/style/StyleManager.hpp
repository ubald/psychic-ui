#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include "../psychicui.hpp"
#include "Style.hpp"
#include "StyleSelector.hpp"
#include "StyleSheet.hpp"
#include "StyleDeclaration.hpp"

namespace psychicui {
    class Component;

    class StyleManager {
    public:
        static std::shared_ptr<StyleManager> instance;
        static std::shared_ptr<StyleManager> getInstance();

        const bool valid() const { return _valid; };
        void setValid() { _valid = true; }

        StyleManager *loadFont(const std::string &name, const std::string &path);
        const sk_sp<SkTypeface> font(const std::string &name) const;

        Style *style(std::string selector);
        std::unique_ptr<Style> computeStyle(const Component *component);

        template<typename T>
        void loadStyleSheet() {
            static_assert(std::is_base_of<StyleSheet, T>::value, "T must extend StyleSheet");
            std::make_unique<T>()->load(this);
            _valid = false;
        }

    protected:
        std::unordered_map<std::string, std::unique_ptr<StyleDeclaration>> _declarations{};
        std::unordered_map<std::string, sk_sp<SkTypeface>>                 _fonts{};
        bool                                                               _valid{false};
    };
}
