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
        tag("TextInputs")
            .classNames({"demo-panel"})
            .direction(Direction::Row)
            .padding(0); // We're using a scroller so no padding

        auto &scroller = add<Div>()
            .width(100_pct)
            .height(100_pct)
            .padding(24)
            .overflow(Overflow::Scroll);

        auto &scrollbar = add<ScrollBar>(scroller)
            .height(100_pct);

        {
            auto &container = scroller.add<HBox>()
                .shrink(0)
                .gap(12);

            // Single Lines

            auto &left = container.add<VBox>()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            left.add<Label>("Single Line Inputs")
                .addClassName("h1");

            left.add<TextInput>(lorem);
            left.add<TextInput>();

            // Multi Lines

            auto &right = container.add<VBox>()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            right.add<Label>("Multiline Text Areas")
                 .addClassName("h1");

            right.add<TextArea>(lorem);
            right.add<TextArea>();

        }
    }

}

