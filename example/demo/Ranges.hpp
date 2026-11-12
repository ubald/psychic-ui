#pragma once

#include <string>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Range.hpp>

namespace psychic_ui {

    class Ranges : public Div {
    public:
        Ranges() {
            setTag("Ranges");
            classNames({"demo-panel"});

            auto &container = add<HBox>();

            auto &left = container.add<Div>();
            left.grow(1)
                .shrink(1);

            left.add<Label>("Horizontal");

            auto &lcont = left.add<Div>();

            lcont.add<Range<int>>()
                .range(0, 10);

            lcont.add<Range<float>>()
                .range(1.23f, 4.56f);

            auto &right = container.add<Div>();
            right.grow(1)
                .shrink(1);

            right.add<Label>("Vertical");

            auto &rcont = right.add<HBox>();
            rcont.grow(1);

            rcont.add<Range<int>>()
                 .range(0, 10)
                 .orientation(Orientation::Vertical);

            rcont.add<Range<float>>()
                 .range(1.23f, 4.56f)
                 .orientation(Orientation::Vertical);

//            add<Label>("Skinned");
//            add<Range<int>>(std::make_shared<MySliderSkin>())
//                ->range(0, 10);
//            add<Range<float>>(std::make_shared<MySliderSkin>())
//                ->range(1.23f, 4.56f);
        }
    };
}
