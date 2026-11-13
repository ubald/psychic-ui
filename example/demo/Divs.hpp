#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/ScrollBar.hpp>

namespace psychic_ui {
    class Divs : public Div {
    public:
        Divs();
    };

    Divs::Divs() : Div() {
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
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rectangle");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadius(0);
        }
        {
            // Rounded
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded 12px");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadius(12);
        }
        {
            // Rounded Left
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Left");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadiusLeft(12);
        }
        {
            // Rounded Right
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Right");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadiusRight(12);
        }
        {
            // Rounded Top
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Top");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadiusTop(12);
        }
        {
            // Rounded Bottom
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Bottom");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadiusBottom(12);
        }
        {
            // Rounded Top Left
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Top Left");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadiusTopLeft(12);
        }
        {
            // Rounded Top Right
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Top Right");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadiusTopRight(12);
        }
        {
            // Rounded Bottom Left
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Bottom Left");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadiusBottomLeft(12);
        }
        {
            // Rounded Bottom Right
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Bottom Right");
            c.add<Div>()
             .addClassName("demo-div")
             .borderRadiusBottomRight(12);
        }
        // endregion
        // region Full Borders
        {
            // Rectangle
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rectangle Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadius(0);
        }
        {
            // Rounded
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded 12px Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadius(12);
        }
        {
            // Rounded Left
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Left Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusLeft(12);
        }
        {
            // Rounded Right
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Right Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusRight(12);
        }
        {
            // Rounded Top
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Top Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusTop(12);
        }
        {
            // Rounded Bottom
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Bottom Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusBottom(12);
        }
        {
            // Rounded Top Left
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Top Left Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusTopLeft(12);
        }
        {
            // Rounded Top Right
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Top Right Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusTopRight(12);
        }
        {
            // Rounded Bottom Left
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Bottom Left Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusBottomLeft(12);
        }
        {
            // Rounded Bottom Right
            auto &c = scroller.add<Div>()
                .direction(Direction::Column);
            c.add<Label>("Rounded Bottom Right Border");
            c.add<Div>()
             .addClassName("demo-div")
             .border(3)
             .borderRadiusBottomRight(12);

        }
        // endregion
    }

}

