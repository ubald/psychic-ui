#pragma once

#include <psychicui/Div.hpp>

namespace psychicui {
    class Divs : public Div {
    public:
        Divs();
    };

    Divs::Divs() : Div() {
        setTag("Divs");
        setClassNames({"demo-panel"});

        style()
            ->set(wrap, "wrap")
            ->set(direction, "row");

        styleManager()
            ->style("divs div")
            ->set(margin, 12);

        styleManager()
            ->style("divs div div.demo-div")
            ->set(width, 128.0f)
            ->set(height, 64.0f)
            ->set(backgroundColor, BASE_03)
            ->set(borderColor, BASE_06);

        // region No Borders

        {
            // Rectangle
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rectangle");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadius, 0);
        }
        {
            // Rounded
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded 12px");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadius, 12);
        }
        {
            // Rounded Left
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Left");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusLeft, 12);
        }
        {
            // Rounded Right
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Right");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusRight, 12);
        }
        {
            // Rounded Top
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusTop, 12);
        }
        {
            // Rounded Bottom
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Bottom");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusBottom, 12);
        }
        {
            // Rounded Top Left
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top Left");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusTopLeft, 12);
        }
        {
            // Rounded Top Right
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Top Right");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusTopRight, 12);
        }
        {
            // Rounded Bottom Left
            auto c = add<Div>();
            c->style()->set(direction, "column");
            c->add<Label>("Rounded Bottom Left");
            c->add<Div>()
             ->addClassName("demo-div")
             ->style()
             ->set(borderRadiusBottomLeft, 12);
        }
        {
            // Rounded Bottom Right
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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
            auto c = add<Div>();
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

