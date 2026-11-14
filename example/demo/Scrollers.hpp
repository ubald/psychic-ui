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
        tag("Scrollers")
            .classNames({"demo-panel"})
            .padding(48);

        add<Label>("Boxes")
            .addClassName("h2");

        auto contents0 = std::make_shared<Div>();
        contents0->shrink(0); // Its own height, scrolled
        //contents0->style()
        //         .alignItems(Align::Start)
        //         .direction(Direction::Row)
        //         .padding(12);
        for (int i = 0; i < 30; ++i) {
            contents0->add<Div>().addClassName("box");
        }
        add<Scroller>(contents0);

        add<Label>("Single Line Text")
            .addClassName("h2");

        // FIXME: No scrollbars
        auto contents1 = std::make_shared<Text>(lorem + ' ' + lorem + ' ' + lorem);
        contents1->multiline(false)
                 .padding(12);
        add<Scroller>(contents1);


        add<Label>("Single Line Text (viewport.style.flexDirection=row)")
            .addClassName("h2");

        // FIXME: No scrollbars unless `scroller2.viewport()->direction(Direction::Row);`
        // FIXME: Line returns in single line boxes messes up content size for scroll (do we need multiline distinction?)
        auto contents2 = std::make_shared<Text>(lorem + " " + lorem + "\n" + lorem);
        contents2->multiline(false)
                 .padding(12);
        auto &scroller2 = add<Scroller>(contents2);
        scroller2.viewport()->direction(Direction::Row);

        add<Label>("Multiline Text")
            .addClassName("h2");


        auto contents3 = std::make_shared<Text>(lorem + '\n' + lorem + '\n' + lorem);
        contents3->multiline(true)
                 .padding(12);
        add<Scroller>(contents3);


        add<Label>("Multiline Text (viewport.style.flexDirection=row)")
            .addClassName("h2");

        // FIXME: No scrollbars unless `scroller2.viewport()->direction(Direction::Row);`
        // FIXME: Line length is messed up, it breaks the line before the end
        // FIXME: Vertical scrollbar doesn't appear
        auto contents4 = std::make_shared<Text>(lorem + "\n" + lorem + "\n" + lorem + "\n" + lorem + "\n" + lorem);
        contents4->multiline(true)
                 .padding(12);
        auto &scroller4 = add<Scroller>(contents4);
        scroller4.viewport()->direction(Direction::Row);
    }

}

