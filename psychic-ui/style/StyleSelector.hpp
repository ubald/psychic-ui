#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_set>

namespace psychic_ui {
    class Div;

    enum Pseudo {
        focus,
        hover,
        active,
        disabled,
        empty,
        firstChild,
        lastChild
    };

    class StyleSelector {
    public:
        static std::unique_ptr<StyleSelector> fromSelector(const std::string &selector);

        bool matches(const Div *component) const;

        const std::string tag() const;
        const std::vector<std::string> classes() const;
        const std::unordered_set<Pseudo, std::hash<int>> pseudo() const;
        const StyleSelector *next() const;

        /**
         * Computes the selector's weight
         * This is used when trying to figure out selectors priority
         * @return int
         */
        const int weight() const;

    protected:
        /**
         * Internal match method
         * This method allows expanding the search to the parent, it is hidden as we don't
         * want the end user calling it directly as the first level of matching must not expand
         * since we optimized by retreiving the parent's computed style instead of walking
         * back up the hierarchy for every possible style declaration. We assume that the
         * parent is current.
         * @param component
         * @param expand bool - Expand the search to the parent
         * @return
         */
        bool matches(const Div *component, bool expand) const;
        std::string                                _tag{};
        std::vector<std::string>                   _classes{};
        std::unordered_set<Pseudo, std::hash<int>> _pseudo{};
        std::unique_ptr<StyleSelector>             _next{nullptr};
    };
}
