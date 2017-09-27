#pragma once

#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/components/Scroller.hpp>
#include <example/demo.hpp>

namespace psychic_ui {

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

        auto contents = container->add<Text>(lorem + '\n' + lorem + '\n' + lorem)
                                 ->style();

        auto scroller = add<Scroller>(container);
        scroller->style()
                ->set(margin, 12)
                ->set(border, 1)
                ->set(borderColor, BASE_00);

        auto container2 = std::make_shared<Div>();
        container2->style()
                  ->set(flexDirection, "row")
                  ->set(padding, 12);

        auto contents2 = container2->add<Text>(lorem + '\n' + lorem + '\n' + lorem);

        auto scroller2 = add<Scroller>(container2);
        scroller2->style()
                 ->set(margin, 12)
                 ->set(border, 1)
                 ->set(borderColor, BASE_00);
    }

}

