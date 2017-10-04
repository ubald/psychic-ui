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

        auto contents = std::make_shared<Text>(lorem + '\n' + lorem + '\n' + lorem);
        contents->style()
                ->set(padding, 12);
        auto scroller = add<Scroller>(contents);
        scroller->style()
                ->set(margin, 12)
                ->set(border, 1)
                ->set(borderColor, BASE_00);

        auto contents2 = std::make_shared<Text>(lorem + "\n" + lorem + "\n" + lorem);
        contents2->style()
                 ->set(grow, 1)
                 ->set(padding, 12);
        auto scroller2 = add<Scroller>(contents2);
        scroller2->viewport()->style()->set(flexDirection, "row");
        scroller2->style()
                 ->set(margin, 12)
                 ->set(border, 1)
                 ->set(borderColor, BASE_00);
    }

}

