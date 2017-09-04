#pragma once

#include <string>
#include <psychicui/Component.hpp>
#include <psychicui/components/Slider.hpp>

namespace psychicui {

    class MySliderSkin : public SliderSkin {

    };

    class Sliders : public Component {
    public:
        Sliders() {
            setTag("Sliders");
            setClassNames({"demo-panel"});

            auto container = add<Component>();
            container->style()->set(direction, "row");

            auto left = container->add<Component>();
            left
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1);

            left->add<Label>("Horizontal");
            left->add<Slider<int>>()
                ->setRange(0, 10);
            left->add<Slider<float>>()
                ->setRange(1.23f, 4.56f);

            auto right = container->add<Component>();
            right
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1);

            right->add<Label>("Vertical");
            right->add<Slider<int>>()
                 ->setRange(0, 10);
            right->add<Slider<float>>()
                 ->setRange(1.23f, 4.56f);

            add<Label>("Skinned");
            add<Slider<int>>(std::make_shared<MySliderSkin>())
                ->setRange(0, 10);
            add<Slider<float>>(std::make_shared<MySliderSkin>())
                ->setRange(1.23f, 4.56f);
        }
    };
}