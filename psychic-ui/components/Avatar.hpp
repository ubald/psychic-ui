#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Skin.hpp"
#include "Tone.hpp"

namespace psychic_ui {
    class Avatar;

    class AvatarSkin : public Skin<Avatar> {
    public:
        AvatarSkin() : Skin<Avatar>() {
            tag("AvatarSkin");
        }

        /**
         * What the avatar shows: its initials, its status (none when empty)
         */
        virtual void setInitials(const std::string &/*initials*/) {};
        virtual void setStatus(std::optional<Tone> /*status*/) {};
    };

    /**
     * An avatar's size, as a class: 16 px (`xs`), 20 px (`sm`), 24 px (`md`, the default) or 32 px (`lg`)
     */
    enum class AvatarSize : std::uint8_t {
        XSmall,
        Small,
        Medium,
        Large
    };

    /**
     * @class Avatar
     *
     * Someone's initials in a circle (the `.initials` part), with an optional status dot at its corner (`.status`).
     * As Pivotal's, it is neutral (secondary text on the raised layer); `colorful(true)` tints it with a color the
     * skin derives from the theme and the name, the same name always getting the same one.
     *
     *     add<Avatar>("Ada Lovelace").size(AvatarSize::Large).status(Tone::Success);
     */
    class Avatar : public Component<AvatarSkin> {
    public:
        explicit Avatar(const std::string &name = "");

        const std::string &name() const { return _name; }

        /**
         * Whose avatar it is, which gives its initials
         */
        template<class Self>
        Self &name(this Self &self, const std::string &name) {
            static_cast<Avatar &>(self).updateName(name);
            return self;
        }

        /**
         * Up to two initials, as Pivotal makes them: of the first and last words ("Ada Lovelace" is "AL"), or the
         * first letter of a single word or of an email address
         */
        const std::string &initials() const { return _initials; }

        static std::string initialsOf(const std::string &name);

        AvatarSize avatarSize() const { return _size; }

        template<class Self>
        Self &size(this Self &self, AvatarSize size) {
            static_cast<Avatar &>(self).updateSize(size);
            return self;
        }

        using Div::size;

        const std::optional<Tone> &status() const { return _status; }

        /**
         * The status dot's tone, or `std::nullopt` for none
         */
        template<class Self>
        Self &status(this Self &self, std::optional<Tone> status) {
            static_cast<Avatar &>(self).updateStatus(status);
            return self;
        }

        bool colorful() const { return _colorful; }

        /**
         * Tint it with a color of its own (the `colorful` class), instead of Pivotal's neutral grey
         */
        template<class Self>
        Self &colorful(this Self &self, detail::BoolArg colorful) {
            static_cast<Avatar &>(self).updateColorful(colorful.value);
            return self;
        }

    protected:
        std::string         _name{};
        std::string         _initials{};
        AvatarSize          _size{AvatarSize::Medium};
        std::optional<Tone> _status{};
        bool                _colorful{false};

        void updateName(const std::string &name);
        void updateSize(AvatarSize size);
        void updateStatus(std::optional<Tone> status);
        void updateColorful(bool colorful);
        void skinChanged() override;
    };
}
