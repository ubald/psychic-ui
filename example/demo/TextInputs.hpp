#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/TextArea.hpp>

namespace psychic_ui {

    class TextInputs : public Div {
    public:
        TextInputs();
    };


    TextInputs::TextInputs() : Div() {
        setTag("TextInputs");
        setClassNames({"demo-panel"});

        style()
            ->set(flexDirection, "row")
            ->set(padding, 0); // We're using a scroller so no padding

        auto scroller = add<Div>();
        scroller
            ->style()
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f)
            ->set(padding, 24)
            ->set(overflow, "scroll");

        auto scrollbar = add<ScrollBar>(scroller);
        scrollbar
            ->style()
            ->set(heightPercent, 1.0f);

        {
            auto container = scroller->add<HBox>();
            container
                ->style()
                ->set(shrink, 0)
                ->set(gap, 12);

            // Single Lines

            auto left = container->add<VBox>();
            left
                ->style()
                ->set(grow, 1)
                ->set(basis, 0) // Both columns needs to be considered the same size
                ->set(gap, 6);

            left->add<Label>("Single Line Inputs")
                ->addClassName("h1");

            left->add<TextInput>();
            left->add<TextInput>();
            left->add<TextInput>();
            left->add<TextInput>();

            // Multi Lines

            auto right = container->add<VBox>();
            right
                ->style()
                ->set(grow, 1)
                ->set(basis, 0) // Both columns needs to be considered the same size
                ->set(gap, 6);

            right->add<Label>("Multiline Text Areas")
                 ->addClassName("h1");

            right->add<TextArea>();
            right->add<TextArea>();
            right->add<TextArea>();
            right->add<TextArea>();

        }
    }

}

