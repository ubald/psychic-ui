#include <algorithm>
#include "StyleSelector.hpp"
#include "psychic-ui/Div.hpp"
#include "../utils/StringUtils.hpp"

namespace psychic_ui {

    std::unique_ptr<StyleSelector> StyleSelector::fromSelector(const std::string &selectorString) {
        auto selectorItems = string_utils::split(selectorString, ' ');

        std::unique_ptr<StyleSelector> selector = nullptr;

        for (const auto &selectorItem: selectorItems) {
            if (selectorItem == ">" && selector) {
                selector->_direct = true;
                continue;
            }

            // Create the selector
            std::unique_ptr<StyleSelector> r = std::make_unique<StyleSelector>();

            Token token{Token::Tag};
            Token nextToken{};
            bool  parsedAtLeastOneToken      = false;

            std::size_t lastTokenPos = 0;

            for (auto it = selectorItem.cbegin(); it < selectorItem.cend(); ++it) {
                const char   c        = (*it);
                unsigned int offset   = 0;
                bool         newToken = false;

                if (c == '#') {
                    nextToken = Token::Id;
                    newToken  = true;
                } else if (c == '.') {
                    nextToken = Token::Class;
                    newToken  = true;
                } else if (c == ':') {
                    nextToken = Token::Pseudo;
                    newToken  = true;
                } else if (it == selectorItem.cend() - 1) {
                    newToken = true;
                    offset   = 1;
                }

                if (newToken) {
                    std::size_t pos = static_cast<std::size_t >(std::distance(selectorItem.cbegin(), it)) + offset;
                    if (pos - lastTokenPos != 0) {
                        switch (token) {
                            case Token::Tag:
                                r->_tag = selectorItem.substr(lastTokenPos, pos - lastTokenPos);
                                break;
                            case Token::Id:
                                r->_id             = selectorItem.substr(lastTokenPos, pos - lastTokenPos);
                                break;
                            case Token::Class:
                                r->_classes.push_back(selectorItem.substr(lastTokenPos, pos - lastTokenPos));
                                break;
                            case Token::Pseudo:
                                std::string pseudo = selectorItem.substr(lastTokenPos, pos - lastTokenPos);
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
                                } else if (pseudo == "firstchild" || pseudo == "first-child") {
                                    r->_pseudo.insert(firstChild);
                                } else if (pseudo == "lastchild" || pseudo == "last-child") {
                                    r->_pseudo.insert(lastChild);
                                }
                                break;
                        }

                        parsedAtLeastOneToken = true;
                    }

                    lastTokenPos = pos + 1;
                    token        = nextToken;
                }
            }

            if (!parsedAtLeastOneToken) {
                continue;
            }

            if (selector) {
                r->_depth = selector->_depth + 1;
                r->_next = std::move(selector);
            }

            selector = std::move(r);
        }

        return selector;
    }

    bool StyleSelector::matches(const Div *component) const {
        return matches(component, false);
    }

    #define parentMatches expand && parent && !_direct && matches(parent, true)

    bool StyleSelector::matches(const Div *component, bool expand) const {
        // Exclude rules that go deeper than us from the start
        if (component->depth() < _depth) {
            return false;
        }

        const Div *parent = component->parent();

        // Match id
        if ((!_id.empty() && component->id() != _id) && (component->internalId() != _id)) {
            return parentMatches;
        }

        // Match tag
        if (!_tag.empty() && std::find(component->tags().cbegin(), component->tags().cend(), _tag) == component->tags().cend()) {
            return parentMatches;
        }

        // Match classes
        if (!std::all_of(
            _classes.cbegin(),
            _classes.cend(),
            [&component](const auto &className) {
                return std::find(
                    component->classNames().cbegin(),
                    component->classNames().cend(),
                    className
                ) != component->classNames().cend();
            }
        )) {
            return parentMatches;
        }

        // Match pseudo
        if (!std::all_of(
            _pseudo.cbegin(),
            _pseudo.cend(),
            [&component](const auto &pseudo) {
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
                        // NOTE: Children are stored in reverse order
                        return component->parent() && component->parent()->childIndex(component) == component->parent()->childCount() - 1;
                    case lastChild:
                        // NOTE: Children are stored in reverse order
                        return component->parent() && component->parent()->childIndex(component) == 0;
                }
                return false; // Just to remove the warning...
            }
        )) {
            return parentMatches;
        }

        // If we are still here it means the first selector level matched us
        // We have to check if it has a parent selector that matches our parent.
        if (_next) {
            return parent && _next->matches(parent, true);
        }

        // Well it's a success!
        return true;
    }

    const bool StyleSelector::direct() const {
        return _direct;
    }

    const int StyleSelector::depth() const {
        return _depth;
    }

    const std::string StyleSelector::tag() const {
        return _tag;
    }

    const std::string StyleSelector::id() const {
        return _id;
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
        int w = _id.empty() ? 0 : 15; // ??? Is 15 too much?
        w += _tag.empty() ? 0 : 10;
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
