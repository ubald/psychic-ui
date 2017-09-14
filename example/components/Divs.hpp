#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/ScrollBar.hpp>

namespace psychic_ui {
    class Divs : public Div {
    public:
        Divs();
    };

    Divs::Divs() : Div() {
        setTag("Divs");
        setClassNames({"demo-panel"});

        styleManager()
            ->style("divs .demo-div")
            ->set(margin, 12)
            ->set(width, 128.0f)
            ->set(height, 64.0f)
            ->set(backgroundColor, BASE_03)
            ->set(borderColor, BASE_06);

        style()
            ->set(direction, "row")
            ->set(padding, 0); // We're using a scroller so no padding

        auto scroller = add<Div>();
        scroller
            ->style()
            ->set(shrink, 1)
            ->set(heightPercent, 1.0f)
            ->set(padding, 24)
            ->set(overflow, "scroll")
            ->set(wrap, "wrap")
            ->set(direction, "row");

        auto scrollbar = add<ScrollBar>(scroller);
        scrollbar
            ->style()
            ->set(heightPercent, 1.0f);

        // region No Borders

        {
            // Rectangle
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rectangle");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadius, 0);
        }
        {
            // Rounded
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded 12px");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadius, 12);
        }
        {
            // Rounded Left
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Left");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusLeft, 12);
        }
        {
            // Rounded Right
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Right");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusRight, 12);
        }
        {
            // Rounded Top
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusTop, 12);
        }
        {
            // Rounded Bottom
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Bottom");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusBottom, 12);
        }
        {
            // Rounded Top Left
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top Left");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusTopLeft, 12);
        }
        {
            // Rounded Top Right
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top Right");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusTopRight, 12);
        }
        {
            // Rounded Bottom Left
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Bottom Left");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusBottomLeft, 12);
        }
        {
            // Rounded Bottom Right
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Bottom Right");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusBottomRight, 12);
        }
        // endregion
        // region Full Borders
        {
            // Rectangle
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rectangle Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadius, 0);
        }
        {
            // Rounded
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded 12px Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadius, 12);
        }
        {
            // Rounded Left
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Left Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadiusLeft, 12);
        }
        {
            // Rounded Right
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Right Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadiusRight, 12);
        }
        {
            // Rounded Top
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadiusTop, 12);
        }
        {
            // Rounded Bottom
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Bottom Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadiusBottom, 12);
        }
        {
            // Rounded Top Left
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top Left Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadiusTopLeft, 12);
        }
        {
            // Rounded Top Right
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top Right Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadiusTopRight, 12);
        }
        {
            // Rounded Bottom Left
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Bottom Left Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadiusBottomLeft, 12);
        }
        {
            // Rounded Bottom Right
            auto c = scroller->add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Bottom Right Border");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(border, 3)
             ->set(borderRadiusBottomRight, 12);

        }
        // endregion
    }

}

