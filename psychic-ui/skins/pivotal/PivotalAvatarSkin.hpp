#pragma once

#include <string_view>
#include "psychic-ui/skins/base/AvatarSkinBase.hpp"

namespace psychic_ui {
    /**
     * Pivotal's avatar: a circle on its background color in a hairline, the initials in its `color`.
     * A `colorful` avatar is filled instead with one of the eighteen hues of Pivotal's swatches
     * (`palette::pivotalSwatches`, red to pink), picked from its name, the initials in white or near black, whichever
     * contrasts more with it, so they read on every hue in every theme.
     */
    class PivotalAvatarSkin : public AvatarSkinBase {
    public:
        PivotalAvatarSkin();

        /**
         * The color a colorful avatar with this name is filled with: the same for the same name, on every platform
         */
        static Color tint(std::string_view name);

        /**
         * The initials' color on a fill: white or near black, whichever contrasts more (WCAG's contrast ratio)
         */
        static Color initialsOn(Color fill);

        void setInitials(const std::string &initials) override;

    protected:
        void styleUpdated() override;
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;

    private:
        Color _tint{};

        void updateTint();
    };
}
