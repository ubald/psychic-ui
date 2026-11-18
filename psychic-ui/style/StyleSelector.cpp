#include <algorithm>
#include "StyleSelector.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/Window.hpp"
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
                    const std::size_t pos = static_cast<std::size_t >(std::distance(selectorItem.cbegin(), it)) + offset;
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
                                const std::string pseudo = selectorItem.substr(lastTokenPos, pos - lastTokenPos);
                                if (pseudo == "focus") {
                                    r->_pseudo.insert(Pseudo::Focus);
                                } else if (pseudo == "focus-visible") {
                                    r->_pseudo.insert(Pseudo::FocusVisible);
                                } else if (pseudo == "hover") {
                                    r->_pseudo.insert(Pseudo::Hover);
                                } else if (pseudo == "active") {
                                    r->_pseudo.insert(Pseudo::Active);
                                } else if (pseudo == "disabled") {
                                    r->_pseudo.insert(Pseudo::Disabled);
                                } else if (pseudo == "empty") {
                                    r->_pseudo.insert(Pseudo::Empty);
                                } else if (pseudo == "firstchild" || pseudo == "first-child") {
                                    r->_pseudo.insert(Pseudo::FirstChild);
                                } else if (pseudo == "lastchild" || pseudo == "last-child") {
                                    r->_pseudo.insert(Pseudo::LastChild);
                                } else if (pseudo == "touch") {
                                    r->_pseudo.insert(Pseudo::Touch);
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
                r->_next  = std::move(selector);
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
        if (!_tag.empty()
            && std::find(component->tags().cbegin(), component->tags().cend(), _tag) == component->tags().cend()) {
            return parentMatches;
        }

        // Match classes
        if (!std::ranges::all_of(
            _classes,
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
        if (!std::ranges::all_of(
            _pseudo,
            [&component](const auto &pseudo) {
                switch (pseudo) {
                    case Pseudo::Hover:
                        return component->mouseOver();
                    case Pseudo::Focus:
                        return component->focusEnabled() && component->hasFocus();
                    case Pseudo::FocusVisible: {
                        // Focused from the keyboard, as CSS's: a focus ring for the keyboard, none for a click
                        if (!component->focusEnabled() || !component->hasFocus()) {
                            return false;
                        }
                        const Window *window = const_cast<Div *>(component)->window();
                        return window && window->focusVisible();
                    }
                    case Pseudo::Active:
                        return component->active();
                    case Pseudo::Disabled:
                        return !component->enabled();
                    case Pseudo::Empty:
                        return component->childCount() == 0;
                    case Pseudo::FirstChild:
                        return component->parent() && component->parent()->childIndex(component) == 0;
                    case Pseudo::LastChild:
                        return component->parent()
                               && component->parent()->childIndex(component) == (int) component->parent()->childCount() - 1;
                    case Pseudo::Touch: {
                        // The window's input mode: `Window:touch Button` and `Button:touch` both work
                        const Window *window = const_cast<Div *>(component)->window();
                        return window && window->inputMode() == InputMode::Touch;
                    }
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

    bool StyleSelector::direct() const {
        return _direct;
    }

    int StyleSelector::depth() const {
        return _depth;
    }

    const std::string &StyleSelector::tag() const {
        return _tag;
    }

    const std::string &StyleSelector::id() const {
        return _id;
    }

    const std::vector<std::string> &StyleSelector::classes() const {
        return _classes;
    }

    const std::unordered_set<Pseudo> &StyleSelector::pseudo() const {
        return _pseudo;
    }

    const StyleSelector *StyleSelector::next() const {
        return _next.get();
    }

    int StyleSelector::weight() const {
        int w = _id.empty() ? 0 : 15; // ??? Is 15 too much?
        w += _tag.empty() ? 0 : 10;
        w += (int) _classes.size() * 10;
        for (const auto &pseudo: _pseudo) {
            switch (pseudo) {
                case Pseudo::Hover:
                case Pseudo::Empty:
                case Pseudo::FirstChild:
                case Pseudo::LastChild:
                case Pseudo::Touch:
                    w += 1;
                    break;
                case Pseudo::Active:
                    w += 2;
                    break;
                case Pseudo::Focus:
                case Pseudo::FocusVisible:
                    w += 3;
                    break;
                case Pseudo::Disabled:
                    w += 4;
                    break;
            }
        }
        w += _next ? _next->weight() : 0;
        return w;
    }
}
