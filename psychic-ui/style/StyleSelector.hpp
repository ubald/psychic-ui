#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>

namespace psychic_ui {
    class Div;

    enum class Pseudo : std::uint8_t {
        Focus,
        FocusVisible,
        Hover,
        Active,
        Disabled,
        Empty,
        FirstChild,
        LastChild,
        Touch
    };

    enum class Token : std::uint8_t {
        Tag,
        Id,
        Class,
        Pseudo
    };

    class StyleSelector {
    public:
        static std::unique_ptr<StyleSelector> fromSelector(const std::string &selector);

        bool matches(const Div *component) const;

        bool direct() const;
        int depth() const;
        const std::string &tag() const;
        const std::string &id() const;
        const std::vector<std::string> &classes() const;
        const std::unordered_set<Pseudo> &pseudo() const;
        const StyleSelector *next() const;

        /**
         * Computes the selector's weight
         * This is used when trying to figure out selectors priority
         * @return int
         */
        int weight() const;

    protected:
        /**
         * Internal match method
         * This method allows expanding the search to the parent, it is hidden as we don't
         * want the end user calling it directly as the first level of matching must not expand
         * since we optimized by retrieving the parent's computed style instead of walking
         * back up the hierarchy for every possible style declaration. We assume that the
         * parent is current.
         * @param component
         * @param expand bool - Expand the search to the parent
         * @return
         */
        bool matches(const Div *component, bool expand) const;
        bool                                       _direct{false};
        int                                        _depth{0};
        std::string                                _tag{};
        std::string                                _id{};
        std::vector<std::string>                   _classes{};
        std::unordered_set<Pseudo> _pseudo{};
        std::unique_ptr<StyleSelector>             _next{nullptr};
    };
}
