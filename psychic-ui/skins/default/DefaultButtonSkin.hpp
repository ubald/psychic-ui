#pragma once

#include "psychic-ui/skins/base/ButtonSkinBase.hpp"

namespace psychic_ui {
    /**
     * The default button, Pivotal's secondary button: a pill of its background and border, behind its icon and label.
     * The variants are the same skin with their name as a class, so the set's rules color them
     * (`Button DefaultButtonSkin.primary`): `primary`, `ghost`, `quiet` and `danger`, as `button.skin("primary")`.
     */
    class DefaultButtonSkin : public ButtonSkinBase {
    public:
        DefaultButtonSkin();
    protected:
        explicit DefaultButtonSkin(const char *variant);
    };

    /**
     * The accent filled, white label: the one action of a page or a dialog
     */
    class DefaultPrimaryButtonSkin : public DefaultButtonSkin {
    public:
        DefaultPrimaryButtonSkin() : DefaultButtonSkin("primary") {}
    };

    /**
     * No chrome until hovered, secondary text
     */
    class DefaultGhostButtonSkin : public DefaultButtonSkin {
    public:
        DefaultGhostButtonSkin() : DefaultButtonSkin("ghost") {}
    };

    /**
     * A ghost in muted text, for actions that stay out of the way
     */
    class DefaultQuietButtonSkin : public DefaultButtonSkin {
    public:
        DefaultQuietButtonSkin() : DefaultButtonSkin("quiet") {}
    };

    /**
     * Destructive: the danger color as text, in a faint ring of it
     */
    class DefaultDangerButtonSkin : public DefaultButtonSkin {
    public:
        DefaultDangerButtonSkin() : DefaultButtonSkin("danger") {}
    };

    /**
     * A tab (the "tab" variant, which `Tabs` use): its label over an underline in its `borderColor`, which isn't
     * drawn as a border, so the label doesn't move
     */
    class DefaultTabButtonSkin : public ButtonSkinBase {
    public:
        DefaultTabButtonSkin();
    protected:
        void drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState state) override;
    };
}
