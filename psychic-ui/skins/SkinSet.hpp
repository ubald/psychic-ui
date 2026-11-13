#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
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
         * Register ImplT as the `variant` of SkinT, replacing what was registered there
         */
        template<class SkinT, class ImplT>
        SkinRegistry &add(std::string_view variant = "") {
            static_assert(std::is_base_of_v<SkinT, ImplT>, "ImplT must implement SkinT");
            _makers[Key{typeid(SkinT), std::string{variant}}] = [] { return std::make_shared<ImplT>(); };
            return *this;
        }

        /**
         * Whether something is registered for that interface and variant
         */
        template<class SkinT>
        bool has(std::string_view variant = "") const {
            return _makers.contains(Key{typeid(SkinT), std::string{variant}});
        }

        /**
         * A new skin: the variant if registered, else the interface's default; nullptr if neither is
         */
        template<class SkinT>
        std::shared_ptr<SkinT> make(std::string_view variant = "") const {
            auto it = _makers.find(Key{typeid(SkinT), std::string{variant}});
            if (it == _makers.cend() && !variant.empty()) {
                it = _makers.find(Key{typeid(SkinT), std::string{}});
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

        virtual const Theme &defaultTheme() const = 0;

        virtual void registerSkins(SkinRegistry &skins) const = 0;

        /**
         * Where the skins this set lacks come from; the default set has none
         */
        virtual std::unique_ptr<SkinSet> fallback() const { return nullptr; }
    };
}
