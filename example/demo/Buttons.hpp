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
        setTag("Buttons");
        setClassNames({"demo-panel"});

        style()
            ->set(flexWrap, "wrap")
            ->set(flexDirection, "row")
            ->set(alignItems, "start");

        // Push
        add<Button>("Push");


        add<Button>("Push (Disabled)")
            ->setEnabled(false);

        // Toggle
        add<Button>("Toggle")
            ->setToggle(true);

        add<Button>("Toggle (Selected)")
            ->setToggle(true)
            ->setSelected(true);

        add<Button>("Toggle (Disabled)")
            ->setToggle(true)
            ->setEnabled(false);

        add<Button>("Toggle (Selected, Disabled)")
            ->setToggle(true)
            ->setSelected(true)
            ->setEnabled(false);

        // CheckBox
        add<CheckBox>("Checkbox");

        add<CheckBox>("Checkbox (Disabled)")
            ->setEnabled(false);

        add<CheckBox>("Checkbox (Checked)")
            ->setChecked(true);

        add<CheckBox>("Checkbox (Checked, Disabled)")
            ->setChecked(true)
            ->setEnabled(false);

    }

}

