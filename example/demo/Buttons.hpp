#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>

namespace psychic_ui {
    class Buttons : public Div {
    public:
        Buttons();
    };

    Buttons::Buttons() : Div() {
        tag("Buttons")
            .classNames({"demo-panel"})
            .wrap(Wrap::Wrap)
            .direction(Direction::Row)
            .alignItems(Align::Start);

        // Push
        add<Button>("Push");


        add<Button>("Push (Disabled)")
            .enabled(false);

        // Toggle
        add<Button>("Toggle")
            .toggle(true);

        add<Button>("Toggle (Selected)")
            .toggle(true)
            .selected(true);

        add<Button>("Toggle (Disabled)")
            .toggle(true)
            .enabled(false);

        add<Button>("Toggle (Selected, Disabled)")
            .toggle(true)
            .selected(true)
            .enabled(false);

        // CheckBox
        add<CheckBox>("Checkbox");

        add<CheckBox>("Checkbox (Disabled)")
            .enabled(false);

        add<CheckBox>("Checkbox (Checked)")
            .checked(true);

        add<CheckBox>("Checkbox (Checked, Disabled)")
            .checked(true)
            .enabled(false);

    }

}

