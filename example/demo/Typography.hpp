#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Text.hpp>

namespace psychic_ui {

    class Typography : public Div {
    public:
        Typography();
    };


    Typography::Typography() : Div() {
        setTag("Typography");
        setClassNames({"demo-panel"});

        style()
            .direction(Direction::Row)
            .padding(0); // We're using a scroller so no padding

        auto scroller = add<Div>();
        scroller
            ->style()
            .width(100_pct)
            .height(100_pct)
            .padding(24)
            .overflow(Overflow::Scroll);

        auto scrollbar = add<ScrollBar>(scroller);
        scrollbar
            ->style()
            .height(100_pct);

        {
            auto container = scroller->add<HBox>();
            container
                ->style()
                .shrink(0)
                .gap(12);

            // Anti Aliasing

            auto left = container->add<VBox>();
            left
                ->style()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            left->add<Label>("Anti Aliasing")
                ->addClassName("h1");

            left->add<Label>("Default Label");

            left->add<Label>("Simple Label (No AA)")
                ->style()
                .textAntiAlias(false);

            left->add<Label>("Simple Label (AA)")
                ->setLcdRender(false)
                ->setSubPixelText(false)
                ->style()
                .textAntiAlias(true);

            left->add<Label>("Simple Label (AA+LCD)")
                ->setLcdRender(true)
                ->setSubPixelText(false)
                ->style()
                .textAntiAlias(true);

            left->add<Label>("Simple Label (AA+SUB)")
                ->setLcdRender(false)
                ->setSubPixelText(true)
                ->style()
                .textAntiAlias(true);

            left->add<Label>("Simple Label (AA+LCD+SUB)")
                ->setLcdRender(true)
                ->setSubPixelText(true)
                ->style()
                .textAntiAlias(true);

            // Ellipsis

            auto right = container->add<VBox>();
            right
                ->style()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            right->add<Label>("Ellipsis")
                 ->addClassName("h1");

            right->add<Label>(lorem);

            right->add<Label>("International")
                 ->addClassName("h1");

            right->add<Label>("French: Brébœuf");
            right->add<Label>("Japanese: ありがとうございました");
            right->add<Label>("Simplified Chinese: 谢谢");
            right->add<Label>("Traditional Chinese: 謝謝");
            right->add<Label>("Greek: ευχαριστώ");
            right->add<Label>("תודה :Hebrew");
            right->add<Label>("Indi: धन्यवाद");
            right->add<Label>("Korean: 고맙습니다");
            right->add<Label>("شكرا :Arabic");
        }

        // BLOCKS
        {
            scroller->add<Label>("Blocks")
                    ->addClassName("h1");

            auto container = scroller->add<HBox>();
            container
                ->style()
                .shrink(0)
                .gap(12);

            auto left = container->add<VBox>();
            left
                ->style()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            left->add<Label>("Automatic Line Height");

            left->add<Text>(lorem)
                ->setEditable(true)
                ->setMultiline(true)
                ->style()
                .lineHeight(Auto)
                .fontSize(10);

            left->add<Text>(lorem)
                ->setEditable(true)
                ->setMultiline(true)
                ->style()
                .lineHeight(Auto)
                .fontSize(12);

            left->add<Text>(lorem)
                ->setEditable(true)
                ->setMultiline(true)
                ->style()
                .lineHeight(Auto)
                .fontSize(16);

            left->add<Text>(lorem)
                ->setEditable(true)
                ->setMultiline(true)
                ->style()
                .lineHeight(Auto)
                .fontSize(24);

            auto right = container->add<VBox>();
            right
                ->style()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            right->add<Label>("Manual Line Height");

            right->add<Text>(lorem + "\n" + lorem)
                 ->setEditable(true)
                 ->setMultiline(true)
                 ->style()
                 .padding(12)
                 .fontSize(10)
                 .lineHeight(12);

            right->add<Text>(lorem)
                 ->setEditable(true)
                 ->setMultiline(true)
                 ->style()
                 .fontSize(12)
                 .lineHeight(14);

            right->add<Text>(lorem)
                 ->setEditable(true)
                 ->setMultiline(true)
                 ->style()
                 .fontSize(16)
                 .lineHeight(18);

            right->add<Text>(lorem)
                 ->setEditable(true)
                 ->setMultiline(true)
                 ->style()
                 .fontSize(24)
                 .lineHeight(28);
        }
    }

}

