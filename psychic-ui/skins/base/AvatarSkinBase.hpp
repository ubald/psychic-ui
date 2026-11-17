#pragma once

#include "psychic-ui/components/Avatar.hpp"
#include "psychic-ui/components/Badge.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "SkinState.hpp"

namespace psychic_ui {
    /**
     * What every avatar skin shares: the `.initials` part, centered, and the `.status` part, a StatusDot at the
     * bottom right corner (not displayed without a status). A skin set's avatar only paints the circle behind them.
     */
    class AvatarSkinBase : public AvatarSkin {
    public:
        AvatarSkinBase();

        void setInitials(const std::string &initials) override;
        void setStatus(std::optional<Tone> status) override;

    protected:
        Label     *_initials{nullptr}; // The `.initials` part, a child, which we own
        StatusDot *_status{nullptr};   // The `.status` part, a child, which we own

        void draw(SkCanvas *canvas) final;

        /**
         * Paint the avatar behind its initials. The default fills its border box with the background color and
         * strokes its border inside, in the border color, from the computed style.
         */
        virtual void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state);
    };
}
