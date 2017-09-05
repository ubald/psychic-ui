#pragma once

#include <string>
#include <psychicui/Component.hpp>
#include <psychicui/components/Range.hpp>

namespace psychicui {

    class Ranges : public Component {
    public:
        Ranges() {
            setTag("Ranges");
            setClassNames({"demo-panel"});

            auto container = add<Component>();
            container->style()->set(direction, "row");

            auto left = container->add<Component>();
            left
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1);

            left->add<Label>("Horizontal");

            left->add<Range<int>>()
                ->setRange(0, 10);

            left->add<Range<float>>()
                ->setRange(1.23f, 4.56f);

            auto right = container->add<Component>();
            right
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1);

            right->add<Label>("Vertical");

            right->add<Range<int>>()
                 ->setRange(0, 10);

            right->add<Range<float>>()
                 ->setRange(1.23f, 4.56f);

//            add<Label>("Skinned");
//            add<Range<int>>(std::make_shared<MySliderSkin>())
//                ->setRange(0, 10);
//            add<Range<float>>(std::make_shared<MySliderSkin>())
//                ->setRange(1.23f, 4.56f);
        }
    };
}