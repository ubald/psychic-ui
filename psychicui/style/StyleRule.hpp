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

        int matches(const Component *component) const;

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
        int matches(const Component *component, int depth) const;
        std::string              _tag{};
        std::vector<std::string> _classes{};
        Pseudo                   _pseudo{};
        std::unique_ptr<Rule>    _next{nullptr};
    };
}

