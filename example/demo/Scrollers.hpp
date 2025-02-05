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

        add<Label>("Boxes")
            ->addClassName("h2");

        auto contents0 = std::make_shared<Div>();
        //contents0->style()
        //         ->set(alignItems, "start")
        //         ->set(flexDirection, "row")
        //         ->set(padding, 12);
        for (int i = 0; i < 30; ++i) {
            auto c = contents0->add<Div>();
            c->style()
             ->set(borderRadius, 0)
             ->set(margin, 12)
             ->set(width, 128.0f)
             ->set(height, 64.0f)
             ->set(backgroundColor, BASE_03)
             ->set(borderColor, BASE_06);
        }
        auto scroller0 = add<Scroller>(contents0);
        scroller0->style()
                 ->set(margin, 12)
                 ->set(border, 1)
                 ->set(borderColor, BASE_00);

        add<Label>("Single Line Text")
            ->addClassName("h2");

        // FIXME: No scrollbars
        auto contents1 = std::make_shared<Text>(lorem + ' ' + lorem + ' ' + lorem);
        contents1->setMultiline(false)
                 ->style()
                 ->set(padding, 12);
        auto scroller1 = add<Scroller>(contents1);
        scroller1->style()
                 ->set(margin, 12)
                 ->set(border, 1)
                 ->set(borderColor, BASE_00);


        add<Label>("Single Line Text (viewport.style.flexDirection=row)")
            ->addClassName("h2");

        // FIXME: No scrollbars unless `scroller2->viewport()->style()->set(flexDirection, "row");`
        // FIXME: Line returns in single line boxes messes up content size for scroll (do we need multiline distinction?)
        auto contents2 = std::make_shared<Text>(lorem + " " + lorem + "\n" + lorem);
        contents2->setMultiline(false)
                 ->style()
                 ->set(padding, 12);
        auto scroller2 = add<Scroller>(contents2);
        scroller2->viewport()->style()->set(flexDirection, "row");
        scroller2->style()
                 ->set(margin, 12)
                 ->set(border, 1)
                 ->set(borderColor, BASE_00);

        add<Label>("Multiline Text")
            ->addClassName("h2");


        auto contents3 = std::make_shared<Text>(lorem + '\n' + lorem + '\n' + lorem);
        contents3->setMultiline(true)
                 ->style()
                 ->set(padding, 12);
        auto scroller3 = add<Scroller>(contents3);
        scroller3->style()
                 ->set(margin, 12)
                 ->set(border, 1)
                 ->set(borderColor, BASE_00);


        add<Label>("Multiline Text (viewport.style.flexDirection=row)")
            ->addClassName("h2");

        // FIXME: No scrollbars unless `scroller2->viewport()->style()->set(flexDirection, "row");`
        // FIXME: Line length is messed up, it breaks the line before the end
        // FIXME: Vertical scrollbar doesn't appear
        auto contents4 = std::make_shared<Text>(lorem + "\n" + lorem + "\n" + lorem + "\n" + lorem + "\n" + lorem);
        contents4->setMultiline(true)
                 ->style()
                 ->set(padding, 12);
        auto scroller4 = add<Scroller>(contents4);
        scroller4->viewport()->style()->set(flexDirection, "row");
        scroller4->style()
                 ->set(margin, 12)
                 ->set(border, 1)
                 ->set(borderColor, BASE_00);
    }

}

