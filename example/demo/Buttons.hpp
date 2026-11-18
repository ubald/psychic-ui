#pragma once

#include <string>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Label.hpp>

namespace psychic_ui {
    class Buttons : public Div {
    public:
        Buttons();

    private:
        /**
         * A titled group, with a line of explanation under its title, and a row for its controls
         */
        Div &group(const std::string &title, const std::string &caption);
    };

    inline Div &Buttons::group(const std::string &title, const std::string &caption) {
        auto &group = add<VBox>().addClassName("group");
        group.add<Label>(title).addClassName("h4");
        group.add<Label>(caption).addClassName("text-tertiary");
        return group.add<HBox>().addClassName("row");
    }

    inline Buttons::Buttons() : Div() {
        tag("Buttons")
            .classNames({"demo-panel"});

        auto &variants = group("Variants", "Secondary is the default; the others are skin variants: skin(\"primary\")");
        variants.add<Button>("Add fixtures").icon("plus").skin("primary");
        variants.add<Button>("Secondary");
        variants.add<Button>("Ghost").skin("ghost");
        variants.add<Button>("Quiet").skin("quiet");
        variants.add<Button>("Remove").icon("trash").skin("danger");

        auto &sizes = group("Sizes and icons", "28 px, 24 px with the small class, and icon buttons");
        sizes.add<Button>("Search").icon("search");
        sizes.add<Button>("Small").addClassName("small");
        sizes.add<Button>("Address").icon("arrow-up").addClassName("small").skin("quiet");
        sizes.add<Button>().icon("more").addClassName("icon-button");
        sizes.add<Button>().icon("sidebar").addClassName("icon-button");
        sizes.add<Button>().icon("settings").addClassName("icon-button");

        auto &toggles = group("Toggles and states", "A selected toggle and a pressed button share the active state");
        toggles.add<Button>("Push");
        toggles.add<Button>("Push (Disabled)").enabled(false);
        toggles.add<Button>("Toggle").toggle(true);
        toggles.add<Button>("Toggle (Selected)").toggle(true).selected(true);
        toggles.add<Button>("Toggle (Disabled)").toggle(true).enabled(false);
        toggles.add<Button>("Primary (Disabled)").skin("primary").enabled(false);

        auto &checks = group("Check boxes", "Space toggles the focused one; Tab shows the focus ring");
        checks.add<CheckBox>("Invert pan");
        checks.add<CheckBox>("Include in blackout").checked(true);
        checks.add<CheckBox>("Disabled").enabled(false);
        checks.add<CheckBox>("Checked, disabled").checked(true).enabled(false);
    }

}
