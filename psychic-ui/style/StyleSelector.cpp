#include <algorithm>
#include "StyleSelector.hpp"
#include "psychic-ui/Div.hpp"
#include "../utils/StringUtils.hpp"

namespace psychic_ui {

    std::unique_ptr<StyleSelector> StyleSelector::fromSelector(const std::string &selectorString) {
        auto                           selector_parts = string_utils::split(selectorString, ' ');
        std::unique_ptr<StyleSelector> selector       = nullptr;
        for (const auto                &selector_part: selector_parts) {
            bool hasTag    = selector_part[0] != '.';
            auto parts     = string_utils::split(selector_part, '.');
            auto partCount = parts.size();

            if (partCount > 0) {
                std::unique_ptr<StyleSelector> r = std::make_unique<StyleSelector>();
                {
                    // Find pseudp
                    auto pseudoParts = string_utils::split(parts.back(), ':', false);
                    if (pseudoParts.size() > 1) {
//                        std::string pseudo = pseudoParts.back();
                        for (auto p = pseudoParts.begin() + 1; p != pseudoParts.end(); ++p) {
                            std::string pseudo = *p;
                            if (pseudo == "focus") {
                                r->_pseudo.insert(focus);
                            } else if (pseudo == "hover") {
                                r->_pseudo.insert(hover);
                            } else if (pseudo == "active") {
                                r->_pseudo.insert(active);
                            } else if (pseudo == "disabled") {
                                r->_pseudo.insert(disabled);
                            } else if (pseudo == "empty") {
                                r->_pseudo.insert(empty);
                            } else if (pseudo == "firstchild") {
                                r->_pseudo.insert(firstChild);
                            } else if (pseudo == "lastchild") {
                                r->_pseudo.insert(lastChild);
                            }
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

    bool StyleSelector::matches(const Div *component) const {
        return matches(component, false);
    }

    bool StyleSelector::matches(const Div *component, bool expand) const {
        const Div *parent = component->parent();

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
        if (!std::all_of(
            _pseudo.begin(), _pseudo.end(), [&component](const auto &pseudo) {
                switch (pseudo) {
                    case hover:
                        return component->mouseOver();
                    case focus:
                        return component->focused();
                    case active:
                        return component->active();
                    case disabled:
                        return !component->enabled();
                    case empty:
                        return component->childCount() == 0;
                    case firstChild:
                        return component->parent() && component->parent()->childIndex(component) == 0;
                    case lastChild:
                        return component->parent() && component->parent()->childIndex(component) == component->parent()->childCount() - 1;
                }
            }
        )) {
            return expand && parent && matches(parent, true);
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

    const std::unordered_set<Pseudo, std::hash<int>> StyleSelector::pseudo() const {
        return _pseudo;
    }

    const StyleSelector *StyleSelector::next() const {
        return _next.get();
    }

    const int StyleSelector::weight() const {
        int w = _tag.empty() ? 0 : 10;
        w += (int) _classes.size() * 10;
        for (const auto &pseudo: _pseudo) {
            switch (pseudo) {
                case hover:
                    w += 1;
                    break;
                case active:
                    w += 2;
                    break;
                case focus:
                    w += 1;
                    break;
                case disabled:
                    w += 3;
                    break;
                case empty:
                    w += 1;
                    break;
                case firstChild:
                    w += 1;
                    break;
                case lastChild:
                    w += 1;
                    break;
            }
        }
        w += _next ? _next->weight() : 0;
        return w;
    }
}
