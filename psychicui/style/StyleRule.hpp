#pragma once

#include <vector>
#include <string>

namespace psychicui {
    class Component;

    enum Pseudo {
        none,
        hover,
        active
    };

    class Rule {
    public:
        static std::unique_ptr<Rule> fromSelector(const std::string &selector);

        bool matches(const Component *component) const;

        const std::string tag() const;
        const std::vector<std::string> classes() const;
        const Pseudo pseudo() const;
        const Rule *next() const;

        /**
         * Computes the rule's weight
         * This is used when trying to figure out rules priority
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
        bool matches(const Component *component, bool expand) const;
        std::string              _tag{};
        std::vector<std::string> _classes{};
        Pseudo                   _pseudo{};
        std::unique_ptr<Rule>    _next{nullptr};
    };
}

