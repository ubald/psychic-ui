#pragma once

#include <string>
#include <psychicui/Div.hpp>
#include <psychicui/components/Range.hpp>

namespace psychicui {

    class Ranges : public Div {
    public:
        Ranges() {
            setTag("Ranges");
            setClassNames({"demo-panel"});

            auto container = add<Div>();
            container->style()->set(direction, "row");

            auto left = container->add<Div>();
            left
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1);

            left->add<Label>("Horizontal");

            auto lcont = left->add<Div>();

            lcont->add<Range<int>>()
                ->setRange(0, 10);

            lcont->add<Range<float>>()
                ->setRange(1.23f, 4.56f);

            auto right = container->add<Div>();
            right
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1);

            right->add<Label>("Vertical");

            auto rcont = right->add<Div>();
            rcont
                ->style()
                ->set(direction, "row");

            rcont->add<Range<int>>()
                 ->setRange(0, 10)
                 ->style()
                 ->set(orientation, "vertical");

            rcont->add<Range<float>>()
                 ->setRange(1.23f, 4.56f)
                 ->style()
                 ->set(orientation, "vertical");

//            add<Label>("Skinned");
//            add<Range<int>>(std::make_shared<MySliderSkin>())
//                ->setRange(0, 10);
//            add<Range<float>>(std::make_shared<MySliderSkin>())
//                ->setRange(1.23f, 4.56f);
        }
    };
}