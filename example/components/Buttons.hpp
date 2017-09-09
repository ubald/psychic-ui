#pragma once

#include <psychicui/Div.hpp>
#include <psychicui/components/Button.hpp>
#include <psychicui/components/CheckBox.hpp>

namespace psychicui {
    class Buttons : public Div {
    public:
        Buttons();
    };

    Buttons::Buttons() : Div() {
        setTag("Buttons");
        setClassNames({"demo-panel"});

        style()
            ->set(wrap, "wrap")
            ->set(direction, "row");

        // Push
        add<Button>("Push");

        // Toggle
        add<Button>("Toggle")
            ->setToggle(true);

        // CheckBox
        add<CheckBox>("Checkbox");

    }

}

