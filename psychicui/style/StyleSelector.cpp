#include <algorithm>
#include "StyleSelector.hpp"
#include "../Component.hpp"
#include "../utils/StringUtils.hpp"

namespace psychicui {

    std::unique_ptr<StyleSelector> StyleSelector::fromSelector(const std::string &selectorString) {
        auto                           selector_parts = split(selectorString, ' ');
        std::unique_ptr<StyleSelector> selector       = nullptr;
        for (const auto                &selector_part: selector_parts) {
            bool hasTag    = selector_part[0] != '.';
            auto parts     = split(selector_part, '.');
            auto partCount = parts.size();

            if (partCount > 0) {
                std::unique_ptr<StyleSelector> r = std::make_unique<StyleSelector>();
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


                if (selector) {
                    r->_next = std::move(selector);
                }
                selector = std::move(r);
            }
        }
        return selector;
    }

    bool StyleSelector::matches(const Component *component) const {
        return matches(component, false);
    }

    bool StyleSelector::matches(const Component *component, bool expand) const {
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
                if (!component->active()) { return expand && parent && matches(parent, true); }
                break;
            default:
                break;
        }

        // If we are still here it means the first selector level matched us
        // We have to check if it has a parent selector that matches our parent.
        // TODO: Check selector length from the start
        if (_next) {
            return parent && _next->matches(parent, true);
        }

        // Well it's a success!
        return true;
    }

    const std::string StyleSelector::tag() const {
        return _tag;
    }

    const std::vector<std::string> StyleSelector::classes() const {
        return _classes;
    }

    const Pseudo StyleSelector::pseudo() const {
        return _pseudo;
    }

    const StyleSelector *StyleSelector::next() const {
        return _next.get();
    }

    const int StyleSelector::weight() const {
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