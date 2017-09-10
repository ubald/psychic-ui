#pragma once

#include <psychicui/Div.hpp>

namespace psychicui {

    class Labels : public Div {
    public:
        Labels();
    };


    Labels::Labels() : Div() {
        setTag("Labels");
        setClassNames({"demo-panel"});
        style()->set(overflow, "scroll");

        add<Label>("Default Label");

        add<Label>("Simple Label (No AA)")
            ->style()
            ->set(textAntiAlias, false);

        add<Label>("Simple Label (AA)")
            ->setLcdRender(false)
            ->setSubpixelText(false)
            ->style()
            ->set(textAntiAlias, true);

        add<Label>("Simple Label (AA+LCD)")
            ->setLcdRender(true)
            ->setSubpixelText(false)
            ->style()
            ->set(textAntiAlias, true);

        add<Label>("Simple Label (AA+SUB)")
            ->setLcdRender(false)
            ->setSubpixelText(true)
            ->style()
            ->set(textAntiAlias, true);

        add<Label>("Simple Label (AA+LCD+SUB)")
            ->setLcdRender(true)
            ->setSubpixelText(true)
            ->style()
            ->set(textAntiAlias, true);

        auto container = add<Div>();
        container
            ->style()
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
            ->set(fontSize, 10);

        left->add<Label>(lorem)
            ->style()
            ->set(fontSize, 12);

        left->add<Label>(lorem)
            ->style()
            ->set(fontSize, 16);

        left->add<Label>(lorem)
            ->style()
            ->set(fontSize, 24);

        auto right = container->add<Div>();
        right
            ->style()
            ->set(grow, 1)
            ->set(shrink, 1);

        right->add<Label>("Manual Line Height");

        right->add<Label>(lorem)
             ->style()
             ->set(fontSize, 10)
             ->set(lineHeight, 12);

        right->add<Label>(lorem)
             ->style()
             ->set(fontSize, 12)
             ->set(lineHeight, 14);

        right->add<Label>(lorem)
             ->style()
             ->set(fontSize, 16)
             ->set(lineHeight, 18);

        right->add<Label>(lorem)
             ->style()
             ->set(fontSize, 24)
             ->set(lineHeight, 28);
    }

}

