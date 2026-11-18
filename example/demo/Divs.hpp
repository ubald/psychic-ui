#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/ScrollBar.hpp>

namespace psychic_ui {
    class Divs : public Div {
    public:
        Divs();
    };

    inline Divs::Divs() : Div() {
        tag("Divs")
            .classNames({"demo-panel"})
            .direction(Direction::Row)
            .padding(0); // We're using a scroller so no padding

        auto &scroller = add<Div>()
            .shrink(1)
            .height(100_pct)
            .padding(24)
            .overflow(Overflow::Scroll)
            .wrap(Wrap::Wrap)
            .direction(Direction::Row);

        auto &scrollbar = add<ScrollBar>(scroller)
            .height(100_pct);

        // region No Borders

        {
            // Rectangle
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rectangle");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadius(0);
        }
        {
            // Rounded
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded 12px");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadius(12);
        }
        {
            // Rounded Left
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Left");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadiusLeft(12);
        }
        {
            // Rounded Right
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Right");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadiusRight(12);
        }
        {
            // Rounded Top
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Top");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadiusTop(12);
        }
        {
            // Rounded Bottom
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Bottom");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadiusBottom(12);
        }
        {
            // Rounded Top Left
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Top Left");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadiusTopLeft(12);
        }
        {
            // Rounded Top Right
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Top Right");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadiusTopRight(12);
        }
        {
            // Rounded Bottom Left
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Bottom Left");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadiusBottomLeft(12);
        }
        {
            // Rounded Bottom Right
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Bottom Right");
            sample.add<Div>()
             .addClassName("demo-div")
             .borderRadiusBottomRight(12);
        }
        // endregion
        // region Full Borders
        {
            // Rectangle
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rectangle Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadius(0);
        }
        {
            // Rounded
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded 12px Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadius(12);
        }
        {
            // Rounded Left
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Left Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusLeft(12);
        }
        {
            // Rounded Right
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Right Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusRight(12);
        }
        {
            // Rounded Top
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Top Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusTop(12);
        }
        {
            // Rounded Bottom
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Bottom Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusBottom(12);
        }
        {
            // Rounded Top Left
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Top Left Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusTopLeft(12);
        }
        {
            // Rounded Top Right
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Top Right Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusTopRight(12);
        }
        {
            // Rounded Bottom Left
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Bottom Left Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusBottomLeft(12);
        }
        {
            // Rounded Bottom Right
            auto &sample = scroller.add<Div>()
                .direction(Direction::Column);
            sample.add<Label>("Rounded Bottom Right Border");
            sample.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusBottomRight(12);

        }
        // endregion
    }

}

