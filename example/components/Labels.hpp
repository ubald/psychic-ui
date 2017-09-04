#pragma once

#include <psychicui/Component.hpp>

namespace psychicui {

    class Labels : public Component {
    public:
        Labels();
    };


    Labels::Labels() : Component() {
        setTag("Labels");
        setClassNames({"demo-panel"});

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

        auto container = add<Component>();
        container
            ->style()
            ->set(direction, "row");

        auto left = container->add<Component>();
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

        auto right = container->add<Component>();
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

