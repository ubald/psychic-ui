#pragma once

#pragma once

#include <psychicui/Div.hpp>
#include <psychicui/components/Label.hpp>
#include <psychicui/components/Scroller.hpp>
#include <example/demo.hpp>

namespace psychicui {

    class Scrollers : public Div {
    public:
        Scrollers();
    };


    Scrollers::Scrollers() : Div() {
        setTag("Scroller");
        setClassNames({"demo-panel"});

        style()
            ->set(padding, 48);

        auto container = std::make_shared<Div>();
        container->style()
                 ->set(padding, 12);

        auto contents = container->add<Label>(lorem + '\n' + lorem + '\n' + lorem)
                                 ->style()
                                 ->set(multiline, true);

        auto scroller = add<Scroller>(container);
        scroller->style()
                ->set(margin, 12)
                ->set(border, 1)
                ->set(borderColor, BASE_00);

        auto container2 = std::make_shared<Div>();
        container2->style()
                  ->set(direction, "row")
                  ->set(padding, 12);

        auto contents2 = container2->add<Label>(lorem + '\n' + lorem + '\n' + lorem)
                                   ->style()
                                   ->set(multiline, true);

        auto scroller2 = add<Scroller>(container2);
        scroller2->style()
                 ->set(margin, 12)
                 ->set(border, 1)
                 ->set(borderColor, BASE_00);
    }

}

