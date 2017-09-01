#include <algorithm>
#include "StyleRule.hpp"
#include "../Component.hpp"
#include "../utils/StringUtils.hpp"

namespace psychicui {

    std::unique_ptr<Rule> Rule::fromSelector(const std::string &selector) {
        auto                  rule_parts = split(selector, ' ');
        std::unique_ptr<Rule> rule       = nullptr;
        for (const auto       &rule_part: rule_parts) {
            bool hasTag    = rule_part[0] != '.';
            auto parts     = split(rule_part, '.');
            auto partCount = parts.size();

            if (partCount > 0) {
                std::unique_ptr<Rule> r = std::make_unique<Rule>();
                {
                    // Find pseudp
                    auto pseudoParts = split(parts.back(), ':', false);
                    if (pseudoParts.size() > 1) {
                        std::string pseudo = pseudoParts.back();
                        if (pseudo == "hover") {
                            r->_pseudo = hover;
                        } else if (pseudo == "active") {
                            r->_pseudo = active;
                        }
                        parts.back() = pseudoParts.front();
                    }
                }

                // Might be empty but we don't care
                // It also can just hold garbage if the styleName is not properly formatted
                // but we don't care about it for now
                if (hasTag) {
                    r->_tag = parts[0];
                }

                // Get classes
                r->_classes = std::vector<std::string>(hasTag ? parts.begin() + 1 : parts.begin(), parts.end());


                if (rule) {
                    r->_next = std::move(rule);
                }
                rule = std::move(r);
            }
        }
        return rule;
    }

    bool Rule::matches(const Component *component) const {
        return matches(component, false);
    }

    bool Rule::matches(const Component *component, bool expand) const {
        const Component *parent = component->parent();

        // Match tag
        if (!_tag.empty()
            && std::find(component->tags().begin(), component->tags().end(), _tag) == component->tags().end()) {
            return expand && parent && matches(parent, true);
        }

        // Match classes
        if (!std::all_of(
            _classes.begin(), _classes.end(), [&component](const auto &className) {
                return std::find(component->classNames().begin(), component->classNames().end(), className)
                       != component->classNames().end();
            }
        )) {
            return expand && parent && matches(parent, true);
        }

        // Match pseudo
        switch (_pseudo) {
            case hover:
                if (!component->mouseOver()) { return expand && parent && matches(parent, true); }
                break;
            case active:
                if (!component->mouseDown()) { return expand && parent && matches(parent, true); }
                break;
            default:
                break;
        }

        // If we are still here it means the first rule level matched us
        // We have to check if it has a parent rule that matches our parent.
        // TODO: Check rule length from the start
        if (_next) {
            return parent && _next->matches(parent, true);
        }

        // Well it's a success!
        return true;
    }

    const std::string Rule::tag() const {
        return _tag;
    }

    const std::vector<std::string> Rule::classes() const {
        return _classes;
    }

    const Pseudo Rule::pseudo() const {
        return _pseudo;
    }

    const Rule *Rule::next() const {
        return _next.get();
    }

    const int Rule::weight() const {
        int w = _tag.empty() ? 0 : 10;
        w += (int) _classes.size() * 10;
        switch (_pseudo) {
            case hover:
                w += 1;
                break;
            case active:
                w += 2;
                break;
            default:
                break;
        }
        w += _next ? _next->weight() : 0;
        return w;
    }
}