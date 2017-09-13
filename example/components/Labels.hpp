#pragma once

#include <psychicui/Div.hpp>
#include <psychicui/components/ScrollBar.hpp>

namespace psychicui {

    class Labels : public Div {
    public:
        Labels();
    };


    Labels::Labels() : Div() {
        setTag("Labels");
        setClassNames({"demo-panel"});

        style()
            ->set(direction, "row")
            ->set(padding, 0); // We're using a scroller so no padding

        auto scroller = add<Div>();
        scroller
            ->style()
            ->set(shrink, 1)
            ->set(heightPercent, 1.0f)
            ->set(padding, 24)
            ->set(overflow, "scroll");

        auto scrollbar = add<ScrollBar>(scroller);
        scrollbar
            ->style()
            ->set(heightPercent, 1.0f);

        {
            auto container = scroller->add<Div>();
            container
                ->style()
                ->set(shrink, 0)
                ->set(direction, "row");

            // Anti Aliasing

            auto left = container->add<Div>();
            left
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1)
                ->set(widthPercent, 0.5f);

            left->add<Label>("Anti Aliasing")
                ->addClassName("h1");

            left->add<Label>("Default Label");

            left->add<Label>("Simple Label (No AA)")
                ->style()
                ->set(textAntiAlias, false);

            left->add<Label>("Simple Label (AA)")
                ->setLcdRender(false)
                ->setSubpixelText(false)
                ->style()
                ->set(textAntiAlias, true);

            left->add<Label>("Simple Label (AA+LCD)")
                ->setLcdRender(true)
                ->setSubpixelText(false)
                ->style()
                ->set(textAntiAlias, true);

            left->add<Label>("Simple Label (AA+SUB)")
                ->setLcdRender(false)
                ->setSubpixelText(true)
                ->style()
                ->set(textAntiAlias, true);

            left->add<Label>("Simple Label (AA+LCD+SUB)")
                ->setLcdRender(true)
                ->setSubpixelText(true)
                ->style()
                ->set(textAntiAlias, true);

            // Ellipsis

            auto right = container->add<Div>();
            right
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1)
                ->set(widthPercent, 0.5f);

            right->add<Label>("Ellipsis")
                 ->addClassName("h1");

            right->add<Label>(lorem);
        }

        // BLOCKS
        {
            scroller->add<Label>("Blocks")
                    ->addClassName("h1");

            auto container = scroller->add<Div>();
            container
                ->style()
                ->set(shrink, 0)
                ->set(direction, "row");

            auto left = container->add<Div>();
            left
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1)
                ->set(lineHeight, Style::Auto);

            left->add<Label>("Automatic Line Height");

            left->add<Label>(lorem)
                ->style()
                ->set(multiline, true)
                ->set(fontSize, 10);

            left->add<Label>(lorem)
                ->style()
                ->set(multiline, true)
                ->set(fontSize, 12);

            left->add<Label>(lorem)
                ->style()
                ->set(multiline, true)
                ->set(fontSize, 16);

            left->add<Label>(lorem)
                ->style()
                ->set(multiline, true)
                ->set(fontSize, 24);

            auto right = container->add<Div>();
            right
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1);

            right->add<Label>("Manual Line Height");

            right->add<Label>(lorem)
                 ->style()
                 ->set(multiline, true)
                 ->set(fontSize, 10)
                 ->set(lineHeight, 12);

            right->add<Label>(lorem)
                 ->style()
                 ->set(multiline, true)
                 ->set(fontSize, 12)
                 ->set(lineHeight, 14);

            right->add<Label>(lorem)
                 ->style()
                 ->set(multiline, true)
                 ->set(fontSize, 16)
                 ->set(lineHeight, 18);

            right->add<Label>(lorem)
                 ->style()
                 ->set(multiline, true)
                 ->set(fontSize, 24)
                 ->set(lineHeight, 28);
        }
    }

}

