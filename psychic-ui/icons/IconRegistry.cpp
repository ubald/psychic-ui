#include <initializer_list>
#include <iostream>
#include <span>
#include <string_view>
#include <utility>
#include <vector>
#include "include/utils/SkParsePath.h"
#include "IconRegistry.hpp"
#include "IconSet.hpp"

namespace psychic_ui {

    IconRegistry &IconRegistry::shared() {
        static IconRegistry registry{icons::pivotalIcons, icons::lucideIcons};
        return registry;
    }

    IconRegistry::IconRegistry(std::initializer_list<std::span<const IconDefinition>> sets) {
        for (auto set: sets) {
            add(set);
        }
    }

    IconRegistry &IconRegistry::add(const IconDefinition &definition) {
        Entry entry{.name = std::string{definition.name}, .grid = definition.grid, .elements = {}};
        entry.elements.reserve(definition.elements.size());
        for (const auto &element: definition.elements) {
            entry.elements.emplace_back(element.paint, std::string{element.d});
        }
        auto found = _index.find(entry.name);
        if (found != _index.end()) {
            _icons[found->second] = std::move(entry);
        } else {
            _index.emplace(entry.name, _icons.size());
            _icons.push_back(std::move(entry));
        }
        return *this;
    }

    IconRegistry &IconRegistry::add(std::span<const IconDefinition> set) {
        _icons.reserve(_icons.size() + set.size());
        for (const auto &definition: set) {
            add(definition);
        }
        return *this;
    }

    bool IconRegistry::has(std::string_view name) const {
        return _index.contains(std::string{name});
    }

    std::vector<std::string_view> IconRegistry::names() const {
        std::vector<std::string_view> names;
        names.reserve(_icons.size());
        for (const auto &icon: _icons) {
            names.emplace_back(icon.name);
        }
        return names;
    }

    const IconShape *IconRegistry::shape(std::string_view name) {
        auto found = _index.find(std::string{name});
        if (found == _index.end()) {
            report(name, "unknown icon");
            return nullptr;
        }
        Entry &entry = _icons[found->second];
        if (entry.invalid) {
            return nullptr;
        }
        if (!entry.shape) {
            IconShape shape{.grid = entry.grid, .paths = {}};
            shape.paths.reserve(entry.elements.size());
            for (const auto &[paint, data]: entry.elements) {
                auto path = SkParsePath::FromSVGString(data.c_str());
                if (!path || path->isEmpty()) {
                    entry.invalid = true;
                    report(name, "icon path data doesn't parse");
                    return nullptr;
                }
                shape.paths.emplace_back(paint, std::move(*path));
            }
            entry.shape = std::move(shape);
        }
        return &*entry.shape;
    }

    void IconRegistry::report(std::string_view name, std::string_view problem) {
        if (_reported.emplace(name).second) {
            std::cerr << "Psychic UI: " << problem << " \"" << name << "\"\n";
        }
    }
}
