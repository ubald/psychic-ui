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
            ->set(wrap, "wrap")
            ->set(flexDirection, "row");

        // Push
        add<Button>("Push");

        // Toggle
        add<Button>("Toggle")
            ->setToggle(true);

        // CheckBox
        add<CheckBox>("Checkbox");

    }

}

