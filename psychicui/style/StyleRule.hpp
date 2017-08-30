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

    enum Match {
        unmatched,
        direct,
        inherited
    };

    class Rule {
    public:
        static std::unique_ptr<Rule> fromSelector(const std::string &selector);

        Match matches(const Component *component) const;

        const std::string tag() const;
        const std::vector<std::string> classes() const;
        const Pseudo pseudo() const;
        const Rule *next() const;

    protected:
        std::string              _tag{};
        std::vector<std::string> _classes{};
        Pseudo                   _pseudo{};
        std::unique_ptr<Rule>    _next{nullptr};
    };
}

