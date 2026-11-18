#pragma once

#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Text.hpp>
#include <example/demo.hpp>

namespace psychic_ui {

    class TypographyPanel : public Div {
    public:
        TypographyPanel();
    };


    inline TypographyPanel::TypographyPanel() : Div() {
        tag("Typography")
            .classNames({"demo-panel"})
            .direction(Direction::Row)
            .padding(0); // We're using a scroller so no padding

        auto &scroller = add<Div>()
            .width(100_pct)
            .height(100_pct)
            .padding(24)
            .overflow(Overflow::Scroll);

        auto &scrollbar = add<ScrollBar>(scroller)
            .height(100_pct);

        {
            auto &container = scroller.add<HBox>()
                .shrink(0)
                .gap(12);

            // Font smoothing

            auto &left = container.add<VBox>()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            left.add<Label>("Font Smoothing")
                .addClassName("h1");

            left.add<Label>("Default Label");

            left.add<Label>("Aliased (FontSmoothing::None)")
                .fontSmoothing(FontSmoothing::None);

            left.add<Label>("Grayscale (FontSmoothing::Grayscale)")
                .fontSmoothing(FontSmoothing::Grayscale);

            left.add<Label>("LCD subpixels (FontSmoothing::Subpixel)")
                .fontSmoothing(FontSmoothing::Subpixel);

            // Ellipsis

            auto &right = container.add<VBox>()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            right.add<Label>("Ellipsis")
                 .addClassName("h1");

            right.add<Label>(lorem)
                 .textOverflow(TextOverflow::Ellipsis);

            right.add<Label>("International")
                 .addClassName("h1");

            right.add<Label>("French: Brébœuf");
            right.add<Label>("Japanese: ありがとうございました");
            right.add<Label>("Simplified Chinese: 谢谢");
            right.add<Label>("Traditional Chinese: 謝謝");
            right.add<Label>("Greek: ευχαριστώ");
            right.add<Label>("תודה :Hebrew");
            right.add<Label>("Indi: धन्यवाद");
            right.add<Label>("Korean: 고맙습니다");
            right.add<Label>("شكرا :Arabic");
        }

        // BLOCKS
        {
            scroller.add<Label>("Blocks")
                    .addClassName("h1");

            auto &container = scroller.add<HBox>()
                .shrink(0)
                .gap(12);

            auto &left = container.add<VBox>()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            left.add<Label>("Automatic Line Height");

            left.add<Text>(lorem)
                .multiline(true)
                .lineHeight(Auto)
                .fontSize(10);

            left.add<Text>(lorem)
                .multiline(true)
                .lineHeight(Auto)
                .fontSize(12);

            left.add<Text>(lorem)
                .multiline(true)
                .lineHeight(Auto)
                .fontSize(16);

            left.add<Text>(lorem)
                .multiline(true)
                .lineHeight(Auto)
                .fontSize(24);

            auto &right = container.add<VBox>()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            right.add<Label>("Manual Line Height");

            right.add<Text>(lorem + "\n" + lorem)
                 .multiline(true)
                 .padding(12)
                 .fontSize(10)
                 .lineHeight(12);

            right.add<Text>(lorem)
                 .multiline(true)
                 .fontSize(12)
                 .lineHeight(14);

            right.add<Text>(lorem)
                 .multiline(true)
                 .fontSize(16)
                 .lineHeight(18);

            right.add<Text>(lorem)
                 .multiline(true)
                 .fontSize(24)
                 .lineHeight(28);
        }
    }

}

