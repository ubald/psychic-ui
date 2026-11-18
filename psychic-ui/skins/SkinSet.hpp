#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "psychic-ui/style/StyleSheet.hpp"
#include "psychic-ui/themes/Theme.hpp"

namespace psychic_ui {
    namespace internal {
        class SkinBase;
    }

    /**
     * Skins by the interface they implement (ButtonSkin, RangeSkin...) and a variant name; "" is the default
     */
    class SkinRegistry {
    public:
        /**
         * Register ImplT as the `variant` of SkinT, replacing what was registered there. The skins it makes have the
         * variant's name as a class, so one skin serves several variants that rules tell apart
         * (`Button ButtonSkinBase.primary`).
         */
        template<class SkinT, class ImplT>
        SkinRegistry &add(std::string_view variant = "") {
            static_assert(std::is_base_of_v<SkinT, ImplT>, "ImplT must implement SkinT");
            _makers[Key{.type = typeid(SkinT), .variant = std::string{variant}}] = [name = std::string{variant}] {
                auto skin = std::make_shared<ImplT>();
                if (!name.empty()) {
                    skin->addClassName(name);
                }
                return skin;
            };
            return *this;
        }

        /**
         * Whether something is registered for that interface and variant
         */
        template<class SkinT>
        bool has(std::string_view variant = "") const {
            return _makers.contains(Key{.type = typeid(SkinT), .variant = std::string{variant}});
        }

        /**
         * A new skin: the variant if registered, else the interface's default; nullptr if neither is
         */
        template<class SkinT>
        std::shared_ptr<SkinT> make(std::string_view variant = "") const {
            auto it = _makers.find(Key{.type = typeid(SkinT), .variant = std::string{variant}});
            if (it == _makers.cend() && !variant.empty()) {
                it = _makers.find(Key{.type = typeid(SkinT), .variant = std::string{}});
            }
            // The maker made an ImplT, which implements SkinT
            return it != _makers.cend() ? std::static_pointer_cast<SkinT>(it->second()) : nullptr;
        }

        void clear() { _makers.clear(); }

    private:
        struct Key {
            std::type_index type;
            std::string     variant;
            bool operator==(const Key &) const = default;
        };

        struct KeyHash {
            std::size_t operator()(const Key &key) const {
                return key.type.hash_code() ^ (std::hash<std::string>{}(key.variant) << 1);
            }
        };

        std::unordered_map<Key, std::function<std::shared_ptr<internal::SkinBase>()>, KeyHash> _makers{};
    };

    /**
     * A design system's implementation: a skin per component (`registerSkins`), the rules that lay them out
     * (`load`, written from `manager->theme()`), and the theme it ships with.
     * A set may implement only some skins: the others come from its fallback set, whose skins and rules are
     * registered and loaded first. A style manager has one set at a time (`StyleManager::skinSet<S>()`).
     */
    class SkinSet : public StyleSheet {
    public:
        virtual std::string_view name() const = 0;

        /**
         * The theme the set ships with, for a system that doesn't say whether it is light or dark
         */
        virtual const Theme &defaultTheme() const = 0;

        /**
         * The set's theme for a light system: its default theme unless it has a light one
         */
        virtual const Theme &lightTheme() const { return defaultTheme(); }

        /**
         * The set's theme for a dark system: its default theme unless it has a dark one
         */
        virtual const Theme &darkTheme() const { return defaultTheme(); }

        /**
         * The set's theme for a system color scheme: light, dark, or the default when it is unknown
         */
        const Theme &theme(ColorScheme scheme) const {
            switch (scheme) {
                case ColorScheme::Light:
                    return lightTheme();
                case ColorScheme::Dark:
                    return darkTheme();
                default:
                    return defaultTheme();
            }
        }

        /**
         * The themes made for the set, as a picker lists them: its light, dark and default themes unless it says more
         */
        virtual std::vector<const Theme *> themes() const {
            std::vector<const Theme *> own{};
            for (const Theme *theme: {&lightTheme(), &darkTheme(), &defaultTheme()}) {
                if (std::ranges::find(own, theme) == own.end()) {
                    own.push_back(theme);
                }
            }
            return own;
        }

        /**
         * Whether the theme is one of the set's (`themes()`), by name
         */
        bool hasTheme(const Theme &theme) const {
            return std::ranges::any_of(themes(), [&theme](const Theme *own) { return own->name == theme.name; });
        }

        virtual void registerSkins(SkinRegistry &skins) const = 0;

        /**
         * Where the skins this set lacks come from; the default set has none
         */
        virtual std::unique_ptr<SkinSet> fallback() const { return nullptr; }
    };
}
