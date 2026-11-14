#include <cctype>
#include <string_view>
#include <vector>
#include "Avatar.hpp"

namespace psychic_ui {
    namespace {
        constexpr std::string_view sizeClasses[]{"xs", "sm", "md", "lg"};

        /**
         * The first character of a word, a whole UTF-8 sequence, upper-cased when it is ASCII
         */
        std::string firstCharacter(const std::string_view word) {
            if (word.empty()) {
                return {};
            }
            std::size_t length = 1;
            const auto  lead   = static_cast<unsigned char>(word[0]);
            if (lead >= 0xF0) {
                length = 4;
            } else if (lead >= 0xE0) {
                length = 3;
            } else if (lead >= 0xC0) {
                length = 2;
            }
            std::string character{word.substr(0, std::min(length, word.size()))};
            if (length == 1) {
                character[0] = static_cast<char>(std::toupper(lead));
            }
            return character;
        }
    }

    Avatar::Avatar(const std::string &name) :
        Component<AvatarSkin>() {
        tag("Avatar");
        _defaultStyle.shrink(0)
            .alignSelf(Align::Center);
        updateSize(AvatarSize::Medium);
        updateName(name);
    }

    std::string Avatar::initialsOf(const std::string &name) {
        // Words split on spaces, @, dots, underscores and dashes
        std::vector<std::string_view> words{};
        const std::string_view        text{name};
        std::size_t                   start = 0;
        for (std::size_t i = 0; i <= text.size(); ++i) {
            const bool separator = i == text.size() || std::isspace(static_cast<unsigned char>(text[i]))
                                   || text[i] == '@' || text[i] == '.' || text[i] == '_' || text[i] == '-';
            if (separator) {
                if (i > start) {
                    words.push_back(text.substr(start, i - start));
                }
                start = i + 1;
            }
        }
        if (words.empty()) {
            return {};
        }
        std::string initials = firstCharacter(words.front());
        if (words.size() > 1 && name.find('@') == std::string::npos) {
            initials += firstCharacter(words.back());
        }
        return initials;
    }

    void Avatar::updateName(const std::string &name) {
        _name     = name;
        _initials = initialsOf(name);
        if (_skin) {
            _skin->setInitials(_initials); // Its tint follows the name too
        }
    }

    void Avatar::updateSize(const AvatarSize size) {
        removeClassName(std::string{sizeClasses[static_cast<std::size_t>(_size)]});
        _size = size;
        addClassName(std::string{sizeClasses[static_cast<std::size_t>(_size)]});
    }

    void Avatar::updateStatus(const std::optional<Tone> status) {
        _status = status;
        if (_skin) {
            _skin->setStatus(_status);
        }
    }

    void Avatar::updateColorful(const bool colorful) {
        _colorful = colorful;
        if (colorful) {
            addClassName("colorful");
        } else {
            removeClassName("colorful");
        }
    }

    void Avatar::skinChanged() {
        if (_skin) {
            _skin->setInitials(_initials);
            _skin->setStatus(_status);
        }
    }
}
