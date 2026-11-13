#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "include/core/SkPath.h"
#include "IconSet.hpp"

namespace psychic_ui {

    /**
     * An icon parsed into Skia paths, on its grid
     */
    struct IconShape {
        float                                    grid{16.0f};
        std::vector<std::pair<IconPaint, SkPath>> paths{};
    };

    /**
     * The icons, by name: the bundled sets, then what the application adds.
     *
     * Path data is parsed once, the first time an icon is drawn, and its paths are kept. An unknown name, or path
     * data that doesn't parse, draws nothing and is reported once on stderr.
     */
    class IconRegistry {
    public:
        /**
         * The registry `Icon` and `paint::icon` draw from, with the bundled sets
         */
        static IconRegistry &shared();

        /**
         * An empty registry, or one with the given sets, registered in order
         */
        IconRegistry() = default;
        explicit IconRegistry(std::initializer_list<std::span<const IconDefinition>> sets);

        /**
         * Registers an icon (copying its data), replacing the one of that name if there is one
         */
        IconRegistry &add(const IconDefinition &definition);

        /**
         * Registers every icon of a set
         */
        IconRegistry &add(std::span<const IconDefinition> set);

        bool has(std::string_view name) const;

        /**
         * The names, in registration order
         */
        std::vector<std::string_view> names() const;

        std::size_t size() const { return _icons.size(); }

        /**
         * The icon's paths, parsed on first use; nullptr for an unknown name or unparsable data (reported once)
         */
        const IconShape *shape(std::string_view name);

    private:
        struct Entry {
            std::string                                   name;
            float                                         grid;
            std::vector<std::pair<IconPaint, std::string>> elements;
            std::optional<IconShape>                      shape{};
            bool                                          invalid{false};
        };

        std::vector<Entry>                           _icons{};
        std::unordered_map<std::string, std::size_t> _index{};
        std::unordered_set<std::string>              _reported{};

        void report(std::string_view name, std::string_view problem);
    };
}
