#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/components/Box.hpp>

namespace psychic_ui {

    class Labels : public Div {
    public:
        Labels();
    };


    Labels::Labels() : Div() {
        setTag("Labels");
        setClassNames({"demo-panel"});

        style()
            ->set(flexDirection, "row")
            ->set(padding, 0); // We're using a scroller so no padding

        auto scroller = add<Div>();
        scroller
            ->style()
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f)
            ->set(padding, 24)
            ->set(overflow, "scroll");

        auto scrollbar = add<ScrollBar>(scroller);
        scrollbar
            ->style()
            ->set(heightPercent, 1.0f);

        {
            auto container = scroller->add<HBox>();
            container
                ->style()
                ->set(shrink, 0)
                ->set(gap, 12);

            // Anti Aliasing

            auto left = container->add<VBox>();
            left
                ->style()
                ->set(grow, 1)
                ->set(basis, 0) // Both columns needs to be considered the same size
                ->set(gap, 6);

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

            auto right = container->add<VBox>();
            right
                ->style()
                ->set(grow, 1)
                ->set(basis, 0) // Both columns needs to be considered the same size
                ->set(gap, 6);

            right->add<Label>("Ellipsis")
                 ->addClassName("h1");

            right->add<Label>(lorem);
        }

        // BLOCKS
        {
            scroller->add<Label>("Blocks")
                    ->addClassName("h1");

            auto container = scroller->add<HBox>();
            container
                ->style()
                ->set(shrink, 0)
                ->set(gap, 12);

            auto left = container->add<VBox>();
            left
                ->style()
                ->set(grow, 1)
                ->set(basis, 0) // Both columns needs to be considered the same size
                ->set(gap, 6);

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

            auto right = container->add<VBox>();
            right
                ->style()
                ->set(grow, 1)
                ->set(basis, 0) // Both columns needs to be considered the same size
                ->set(gap, 6);

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

