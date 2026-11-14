#pragma once

#include <string_view>
#include "psychic-ui/skins/base/AvatarSkinBase.hpp"

namespace psychic_ui {
    struct Palette;

    /**
     * The default avatar, Pivotal's: a circle on its background color in a hairline, the initials in its `color`.
     * A `colorful` avatar is tinted instead: a hue picked from its name among six spread around the accent's
     * (skipping yellows, which don't read as text), at the accent text's saturation and brightness, so it reads on
     * every layer of the theme; the circle is that hue, faintly, and the initials are in it.
     */
    class DefaultAvatarSkin : public AvatarSkinBase {
    public:
        DefaultAvatarSkin();

        /**
         * The tint a colorful avatar with this name gets in a palette
         */
        static Color tint(std::string_view name, const Palette &palette);

        void setInitials(const std::string &initials) override;

    protected:
        void styleUpdated() override;
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;

    private:
        Color _tint{};

        void updateTint();
    };
}
