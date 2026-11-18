#pragma once

#include <psychic-color/spaces/ARGB.hpp>
#include <psychic-color/spaces/RGB.hpp>
#include <psychic-color/spaces/Hex.hpp>
#include <psychic-color/spaces/Gray.hpp>
#include <psychic-color/spaces/HSB.hpp>
#include <psychic-color/spaces/HSL.hpp>
#include <psychic-color/schemes/Analogous.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-color/PsychicColor.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-color/schemes/Complementary.hpp>
#include <psychic-color/schemes/Compound.hpp>
#include <psychic-color/schemes/FlippedCompound.hpp>
#include <psychic-color/schemes/Monochrome.hpp>
#include <psychic-color/schemes/SplitComplementary.hpp>
#include <psychic-color/schemes/Triad.hpp>
#include <psychic-color/schemes/Tetrad.hpp>

namespace psychic_ui {

    using namespace psychic_color;

    class Colors : public Div {
    public:
        Colors();
    protected:
        ARGB                    argb{};
        RGB                     rgb{};
        Hex                     hex{};
        Gray                    gray{};
        HSB                     hsb{};
        HSL                     hsl{};
        HSB                     primary{};
        Div *    primarySwatch{nullptr};

        Analogous<HSB>          analogous{primary};
        Div *    analogousSwatch0{nullptr};
        Div *    analogousSwatch1{nullptr};
        Div *    analogousSwatch2{nullptr};
        Div *    analogousSwatch3{nullptr};
        Div *    analogousSwatch4{nullptr};
        Analogous<HSB>          rybAnalogous{primary, true};
        Div *    rybAnalogousSwatch0{nullptr};
        Div *    rybAnalogousSwatch1{nullptr};
        Div *    rybAnalogousSwatch2{nullptr};
        Div *    rybAnalogousSwatch3{nullptr};
        Div *    rybAnalogousSwatch4{nullptr};

        Monochrome<HSB>         monochrome{primary};
        Div *    monochromeSwatch0{nullptr};
        Div *    monochromeSwatch1{nullptr};
        Div *    monochromeSwatch2{nullptr};
        Div *    monochromeSwatch3{nullptr};
        Div *    monochromeSwatch4{nullptr};

        Complementary<HSB>      complementary{primary};
        Div *    complementarySwatch0{nullptr};
        Div *    complementarySwatch1{nullptr};
        Div *    complementarySwatch2{nullptr};
        Div *    complementarySwatch3{nullptr};
        Div *    complementarySwatch4{nullptr};
        Div *    complementarySwatch5{nullptr};
        Complementary<HSB>      rybComplementary{primary, true};
        Div *    rybComplementarySwatch0{nullptr};
        Div *    rybComplementarySwatch1{nullptr};
        Div *    rybComplementarySwatch2{nullptr};
        Div *    rybComplementarySwatch3{nullptr};
        Div *    rybComplementarySwatch4{nullptr};
        Div *    rybComplementarySwatch5{nullptr};

        SplitComplementary<HSB> splitComplementary{primary};
        Div *    splitComplementarySwatch0{nullptr};
        Div *    splitComplementarySwatch1{nullptr};
        Div *    splitComplementarySwatch2{nullptr};
        SplitComplementary<HSB> rybSplitComplementary{primary, true};
        Div *    rybSplitComplementarySwatch0{nullptr};
        Div *    rybSplitComplementarySwatch1{nullptr};
        Div *    rybSplitComplementarySwatch2{nullptr};

        Compound<HSB>           compound{primary};
        Div *    compoundSwatch0{nullptr};
        Div *    compoundSwatch1{nullptr};
        Div *    compoundSwatch2{nullptr};
        Div *    compoundSwatch3{nullptr};
        Div *    compoundSwatch4{nullptr};
        Div *    compoundSwatch5{nullptr};
        Compound<HSB>           rybCompound{primary, true};
        Div *    rybCompoundSwatch0{nullptr};
        Div *    rybCompoundSwatch1{nullptr};
        Div *    rybCompoundSwatch2{nullptr};
        Div *    rybCompoundSwatch3{nullptr};
        Div *    rybCompoundSwatch4{nullptr};
        Div *    rybCompoundSwatch5{nullptr};

        FlippedCompound<HSB>    flippedCompound{primary};
        Div *    flippedCompoundSwatch0{nullptr};
        Div *    flippedCompoundSwatch1{nullptr};
        Div *    flippedCompoundSwatch2{nullptr};
        Div *    flippedCompoundSwatch3{nullptr};
        Div *    flippedCompoundSwatch4{nullptr};
        Div *    flippedCompoundSwatch5{nullptr};
        FlippedCompound<HSB>    rybFlippedCompound{primary, true};
        Div *    rybFlippedCompoundSwatch0{nullptr};
        Div *    rybFlippedCompoundSwatch1{nullptr};
        Div *    rybFlippedCompoundSwatch2{nullptr};
        Div *    rybFlippedCompoundSwatch3{nullptr};
        Div *    rybFlippedCompoundSwatch4{nullptr};
        Div *    rybFlippedCompoundSwatch5{nullptr};

        Triad<HSB>              triad{primary};
        Div *    triadSwatch0{nullptr};
        Div *    triadSwatch1{nullptr};
        Div *    triadSwatch2{nullptr};
        Triad<HSB>              rybTriad{primary, true};
        Div *    rybTriadSwatch0{nullptr};
        Div *    rybTriadSwatch1{nullptr};
        Div *    rybTriadSwatch2{nullptr};

        Tetrad<HSB>             tetrad{primary};
        Div *    tetradSwatch0{nullptr};
        Div *    tetradSwatch1{nullptr};
        Div *    tetradSwatch2{nullptr};
        Div *    tetradSwatch3{nullptr};
        Tetrad<HSB>             rybTetrad{primary, true};
        Div *    rybTetradSwatch0{nullptr};
        Div *    rybTetradSwatch1{nullptr};
        Div *    rybTetradSwatch2{nullptr};
        Div *    rybTetradSwatch3{nullptr};

        void updateSwatches();
    };

    inline Colors::Colors() : Div() {
        tag("Colors")
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

            // region Colors
            auto &left = container.add<VBox>()
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            // region Colors Content
            {
                left.add<Label>("ARGB")
                    .addClassName("h1")
                    .addClassName("title");

                auto &swatch = left.add<Div>().addClassName("swatch");

                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        argb.setAlpha(value);
                        swatch.backgroundColor(Color::fromARGB(argb.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        argb.setRed(value);
                        swatch.backgroundColor(Color::fromARGB(argb.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        argb.setGreen(value);
                        swatch.backgroundColor(Color::fromARGB(argb.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        argb.setBlue(value);
                        swatch.backgroundColor(Color::fromARGB(argb.getColor()));
                    }
                );
            }

            {
                left.add<Label>("RGB")
                    .addClassName("h1")
                    .addClassName("title");

                auto &swatch = left.add<Div>().addClassName("swatch");

                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        rgb.setRed(value);
                        swatch.backgroundColor(psychic_ui::rgb(rgb.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        rgb.setGreen(value);
                        swatch.backgroundColor(psychic_ui::rgb(rgb.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        rgb.setBlue(value);
                        swatch.backgroundColor(psychic_ui::rgb(rgb.getColor()));
                    }
                );
            }

            {
                left.add<Label>("Hex")
                    .addClassName("h1")
                    .addClassName("title");

                auto &swatch = left.add<Div>().addClassName("swatch");

                left.add<Range<unsigned int>>(
                    0, 255,
                    [this, &swatch](unsigned int value) {
                        hex.setAlpha(value);
                        swatch.backgroundColor(Color::fromARGB(hex.getColor()));
                    }
                );
                left.add<Range<unsigned int>>(
                    0, 255,
                    [this, &swatch](unsigned int value) {
                        hex.setRed(value);
                        swatch.backgroundColor(Color::fromARGB(hex.getColor()));
                    }
                );
                left.add<Range<unsigned int>>(
                    0, 255,
                    [this, &swatch](unsigned int value) {
                        hex.setGreen(value);
                        swatch.backgroundColor(Color::fromARGB(hex.getColor()));
                    }
                );
                left.add<Range<unsigned int>>(
                    0, 255,
                    [this, &swatch](unsigned int value) {
                        hex.setBlue(value);
                        swatch.backgroundColor(Color::fromARGB(hex.getColor()));
                    }
                );
            }

            {
                left.add<Label>("Gray")
                    .addClassName("h1")
                    .addClassName("title");

                auto &swatch = left.add<Div>().addClassName("swatch");

                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        gray.setGray(value);
                        swatch.backgroundColor(psychic_ui::rgb(gray.getColor()));
                    }
                );
            }

            {
                left.add<Label>("HSB")
                    .addClassName("h1")
                    .addClassName("title");

                auto &swatch = left.add<Div>().addClassName("swatch");

                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        hsb.setHue(value);
                        swatch.backgroundColor(psychic_ui::rgb(hsb.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        hsb.setSaturation(value);
                        swatch.backgroundColor(psychic_ui::rgb(hsb.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        hsb.setBrightness(value);
                        swatch.backgroundColor(psychic_ui::rgb(hsb.getColor()));
                    }
                );
            }

            {
                left.add<Label>("HSL")
                    .addClassName("h1")
                    .addClassName("title");

                auto &swatch = left.add<Div>().addClassName("swatch");

                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        hsl.setHue(value);
                        swatch.backgroundColor(psychic_ui::rgb(hsl.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        hsl.setSaturation(value);
                        swatch.backgroundColor(psychic_ui::rgb(hsl.getColor()));
                    }
                );
                left.add<Range<float>>(
                    [this, &swatch](float value) {
                        hsl.setLightness(value);
                        swatch.backgroundColor(psychic_ui::rgb(hsl.getColor()));
                    }
                );
            }
            // endregion

            // endregion

            // region Schemes
            auto &right = container.add<VBox>()
                .addClassName("color-schemes")
                .grow(1)
                .basis(0) // Both columns needs to be considered the same size
                .gap(6);

            // region Schemes Content
            {
                right.add<Label>("Primary Color (HSB)")
                     .addClassName("h1")
                     .addClassName("title");

                primarySwatch = &right.add<Div>();
                primarySwatch->addClassName("swatch");

                right.add<Range<float>>(
                    [this](float value) {
                        primary.setHue(value);
                        updateSwatches();
                    }
                );
                right.add<Range<float>>(
                    [this](float value) {
                        primary.setSaturation(value);
                        updateSwatches();
                    }
                );
                right.add<Range<float>>(
                    [this](float value) {
                        primary.setBrightness(value);
                        updateSwatches();
                    }
                );

                {
                    right.add<Label>("Monochrome")
                         .addClassName("h2")
                         .addClassName("title");

                    right.add<Label>("Contrast");
                    auto &contrast = right.add<Range<float>>(
                        0.25f,
                        [this](float value) {
                            monochrome.setContrast(value);
                            updateSwatches();
                        }
                    );

                    auto &swatches = right.add<Div>()
                        .addClassName("swatches");
                    monochromeSwatch0 = &swatches.add<Div>();
                    monochromeSwatch0->addClassName("swatch");
                    monochromeSwatch1 = &swatches.add<Div>();
                    monochromeSwatch1->addClassName("swatch");
                    monochromeSwatch2 = &swatches.add<Div>();
                    monochromeSwatch2->addClassName("swatch");
                    monochromeSwatch3 = &swatches.add<Div>();
                    monochromeSwatch3->addClassName("swatch");
                    monochromeSwatch4 = &swatches.add<Div>();
                    monochromeSwatch4->addClassName("swatch");
                }

                {
                    right.add<Label>("Analogous")
                         .addClassName("h2")
                         .addClassName("title");

                    right.add<Label>("Angle");
                    auto &angle = right.add<Range<float>>(
                        0.0f, 90.0f, 15.0f,
                        [this](float value) {
                            rybAnalogous.setAngle(value);
                            analogous.setAngle(value);
                            updateSwatches();
                        }
                    );

                    right.add<Label>("Artistic");
                    auto &rybSwatches = right.add<Div>()
                        .addClassName("swatches");
                    rybAnalogousSwatch0 = &rybSwatches.add<Div>();
                    rybAnalogousSwatch0->addClassName("swatch");
                    rybAnalogousSwatch1 = &rybSwatches.add<Div>();
                    rybAnalogousSwatch1->addClassName("swatch");
                    rybAnalogousSwatch2 = &rybSwatches.add<Div>();
                    rybAnalogousSwatch2->addClassName("swatch");
                    rybAnalogousSwatch3 = &rybSwatches.add<Div>();
                    rybAnalogousSwatch3->addClassName("swatch");
                    rybAnalogousSwatch4 = &rybSwatches.add<Div>();
                    rybAnalogousSwatch4->addClassName("swatch");

                    right.add<Label>("Scientific");
                    auto &swatches = right.add<Div>()
                        .addClassName("swatches");
                    analogousSwatch0 = &swatches.add<Div>();
                    analogousSwatch0->addClassName("swatch");
                    analogousSwatch1 = &swatches.add<Div>();
                    analogousSwatch1->addClassName("swatch");
                    analogousSwatch2 = &swatches.add<Div>();
                    analogousSwatch2->addClassName("swatch");
                    analogousSwatch3 = &swatches.add<Div>();
                    analogousSwatch3->addClassName("swatch");
                    analogousSwatch4 = &swatches.add<Div>();
                    analogousSwatch4->addClassName("swatch");
                }

                {
                    right.add<Label>("Complementary")
                         .addClassName("h2")
                         .addClassName("title");

                    right.add<Label>("Artistic");
                    auto &rybSwatches = right.add<Div>()
                        .addClassName("swatches");
                    rybComplementarySwatch0 = &rybSwatches.add<Div>();
                    rybComplementarySwatch0->addClassName("swatch");
                    rybComplementarySwatch1 = &rybSwatches.add<Div>();
                    rybComplementarySwatch1->addClassName("swatch");
                    rybComplementarySwatch2 = &rybSwatches.add<Div>();
                    rybComplementarySwatch2->addClassName("swatch");
                    rybComplementarySwatch3 = &rybSwatches.add<Div>();
                    rybComplementarySwatch3->addClassName("swatch");
                    rybComplementarySwatch4 = &rybSwatches.add<Div>();
                    rybComplementarySwatch4->addClassName("swatch");
                    rybComplementarySwatch5 = &rybSwatches.add<Div>();
                    rybComplementarySwatch5->addClassName("swatch");

                    right.add<Label>("Scientific");
                    auto &swatches = right.add<Div>()
                        .addClassName("swatches");
                    complementarySwatch0 = &swatches.add<Div>();
                    complementarySwatch0->addClassName("swatch");
                    complementarySwatch1 = &swatches.add<Div>();
                    complementarySwatch1->addClassName("swatch");
                    complementarySwatch2 = &swatches.add<Div>();
                    complementarySwatch2->addClassName("swatch");
                    complementarySwatch3 = &swatches.add<Div>();
                    complementarySwatch3->addClassName("swatch");
                    complementarySwatch4 = &swatches.add<Div>();
                    complementarySwatch4->addClassName("swatch");
                    complementarySwatch5 = &swatches.add<Div>();
                    complementarySwatch5->addClassName("swatch");
                }

                {
                    right.add<Label>("Split Complementary")
                         .addClassName("h2")
                         .addClassName("title");

                    right.add<Label>("Angle");
                    auto &range = right.add<Range<float>>(
                        0.0f, 180.0f, 30.0f,
                        [this](float value) {
                            rybSplitComplementary.setAngle(value);
                            splitComplementary.setAngle(value);
                            updateSwatches();
                        }
                    );

                    right.add<Label>("Artistic");
                    auto &rybSwatches = right.add<Div>()
                        .addClassName("swatches");
                    rybSplitComplementarySwatch0 = &rybSwatches.add<Div>();
                    rybSplitComplementarySwatch0->addClassName("swatch");
                    rybSplitComplementarySwatch1 = &rybSwatches.add<Div>();
                    rybSplitComplementarySwatch1->addClassName("swatch");
                    rybSplitComplementarySwatch2 = &rybSwatches.add<Div>();
                    rybSplitComplementarySwatch2->addClassName("swatch");

                    right.add<Label>("Scientific");
                    auto &swatches = right.add<Div>()
                        .addClassName("swatches");
                    splitComplementarySwatch0 = &swatches.add<Div>();
                    splitComplementarySwatch0->addClassName("swatch");
                    splitComplementarySwatch1 = &swatches.add<Div>();
                    splitComplementarySwatch1->addClassName("swatch");
                    splitComplementarySwatch2 = &swatches.add<Div>();
                    splitComplementarySwatch2->addClassName("swatch");

                }

                {
                    right.add<Label>("Compound")
                         .addClassName("h2")
                         .addClassName("title");
                         
                    right.add<Label>("Artistic");
                    auto &rybSwatches = right.add<Div>()
                        .addClassName("swatches");
                    rybCompoundSwatch0 = &rybSwatches.add<Div>();
                    rybCompoundSwatch0->addClassName("swatch");
                    rybCompoundSwatch1 = &rybSwatches.add<Div>();
                    rybCompoundSwatch1->addClassName("swatch");
                    rybCompoundSwatch2 = &rybSwatches.add<Div>();
                    rybCompoundSwatch2->addClassName("swatch");
                    rybCompoundSwatch3 = &rybSwatches.add<Div>();
                    rybCompoundSwatch3->addClassName("swatch");
                    rybCompoundSwatch4 = &rybSwatches.add<Div>();
                    rybCompoundSwatch4->addClassName("swatch");
                    rybCompoundSwatch5 = &rybSwatches.add<Div>();
                    rybCompoundSwatch5->addClassName("swatch");
                    
                    right.add<Label>("Scientific");
                    auto &swatches = right.add<Div>()
                        .addClassName("swatches");
                    compoundSwatch0 = &swatches.add<Div>();
                    compoundSwatch0->addClassName("swatch");
                    compoundSwatch1 = &swatches.add<Div>();
                    compoundSwatch1->addClassName("swatch");
                    compoundSwatch2 = &swatches.add<Div>();
                    compoundSwatch2->addClassName("swatch");
                    compoundSwatch3 = &swatches.add<Div>();
                    compoundSwatch3->addClassName("swatch");
                    compoundSwatch4 = &swatches.add<Div>();
                    compoundSwatch4->addClassName("swatch");
                    compoundSwatch5 = &swatches.add<Div>();
                    compoundSwatch5->addClassName("swatch");
                }

                {
                    right.add<Label>("Flipped Compound")
                         .addClassName("h2")
                         .addClassName("title");
                         
                    right.add<Label>("Artistic");
                    auto &rybSwatches = right.add<Div>()
                        .addClassName("swatches");
                    rybFlippedCompoundSwatch0 = &rybSwatches.add<Div>();
                    rybFlippedCompoundSwatch0->addClassName("swatch");
                    rybFlippedCompoundSwatch1 = &rybSwatches.add<Div>();
                    rybFlippedCompoundSwatch1->addClassName("swatch");
                    rybFlippedCompoundSwatch2 = &rybSwatches.add<Div>();
                    rybFlippedCompoundSwatch2->addClassName("swatch");
                    rybFlippedCompoundSwatch3 = &rybSwatches.add<Div>();
                    rybFlippedCompoundSwatch3->addClassName("swatch");
                    rybFlippedCompoundSwatch4 = &rybSwatches.add<Div>();
                    rybFlippedCompoundSwatch4->addClassName("swatch");
                    rybFlippedCompoundSwatch5 = &rybSwatches.add<Div>();
                    rybFlippedCompoundSwatch5->addClassName("swatch");
                    
                    right.add<Label>("Scientific");
                    auto &swatches = right.add<Div>()
                        .addClassName("swatches");
                    flippedCompoundSwatch0 = &swatches.add<Div>();
                    flippedCompoundSwatch0->addClassName("swatch");
                    flippedCompoundSwatch1 = &swatches.add<Div>();
                    flippedCompoundSwatch1->addClassName("swatch");
                    flippedCompoundSwatch2 = &swatches.add<Div>();
                    flippedCompoundSwatch2->addClassName("swatch");
                    flippedCompoundSwatch3 = &swatches.add<Div>();
                    flippedCompoundSwatch3->addClassName("swatch");
                    flippedCompoundSwatch4 = &swatches.add<Div>();
                    flippedCompoundSwatch4->addClassName("swatch");
                    flippedCompoundSwatch5 = &swatches.add<Div>();
                    flippedCompoundSwatch5->addClassName("swatch");
                }

                {
                    right.add<Label>("Triad")
                         .addClassName("h2")
                         .addClassName("title");

                    right.add<Label>("Artistic");
                    auto &rybSwatches = right.add<Div>()
                        .addClassName("swatches");
                    rybTriadSwatch0 = &rybSwatches.add<Div>();
                    rybTriadSwatch0->addClassName("swatch");
                    rybTriadSwatch1 = &rybSwatches.add<Div>();
                    rybTriadSwatch1->addClassName("swatch");
                    rybTriadSwatch2 = &rybSwatches.add<Div>();
                    rybTriadSwatch2->addClassName("swatch");

                    right.add<Label>("Scientific");
                    auto &swatches = right.add<Div>()
                        .addClassName("swatches");
                    triadSwatch0 = &swatches.add<Div>();
                    triadSwatch0->addClassName("swatch");
                    triadSwatch1 = &swatches.add<Div>();
                    triadSwatch1->addClassName("swatch");
                    triadSwatch2 = &swatches.add<Div>();
                    triadSwatch2->addClassName("swatch");
                }

                {
                    right.add<Label>("Tetrad")
                         .addClassName("h2")
                         .addClassName("title");

                    right.add<Label>("Angle");
                    auto &range = right.add<Range<float>>(
                        0.0f, 180.0f, 90.0f,
                        [this](float value) {
                            rybTetrad.setAngle(value);
                            tetrad.setAngle(value);
                            updateSwatches();
                        }
                    );

                    right.add<Label>("Artistic");
                    auto &rybSwatches = right.add<Div>()
                        .addClassName("swatches");
                    rybTetradSwatch0 = &rybSwatches.add<Div>();
                    rybTetradSwatch0->addClassName("swatch");
                    rybTetradSwatch1 = &rybSwatches.add<Div>();
                    rybTetradSwatch1->addClassName("swatch");
                    rybTetradSwatch2 = &rybSwatches.add<Div>();
                    rybTetradSwatch2->addClassName("swatch");
                    rybTetradSwatch3 = &rybSwatches.add<Div>();
                    rybTetradSwatch3->addClassName("swatch");

                    right.add<Label>("Scientific");
                    auto &swatches = right.add<Div>()
                        .addClassName("swatches");
                    tetradSwatch0 = &swatches.add<Div>();
                    tetradSwatch0->addClassName("swatch");
                    tetradSwatch1 = &swatches.add<Div>();
                    tetradSwatch1->addClassName("swatch");
                    tetradSwatch2 = &swatches.add<Div>();
                    tetradSwatch2->addClassName("swatch");
                    tetradSwatch3 = &swatches.add<Div>();
                    tetradSwatch3->addClassName("swatch");
                }
            }
            // endregion

            // endregion
        }
    }

    /**
     * This is horrible but its only a test/demo
     */
    inline void Colors::updateSwatches() {
        primarySwatch->backgroundColor(psychic_ui::rgb(primary.getColor()));

        analogous.generate();
        analogousSwatch0->backgroundColor(psychic_ui::rgb(analogous.getColor(0).getColor()));
        analogousSwatch1->backgroundColor(psychic_ui::rgb(analogous.getColor(1).getColor()));
        analogousSwatch2->backgroundColor(psychic_ui::rgb(analogous.getColor(2).getColor()));
        analogousSwatch3->backgroundColor(psychic_ui::rgb(analogous.getColor(3).getColor()));
        analogousSwatch4->backgroundColor(psychic_ui::rgb(analogous.getColor(4).getColor()));

        rybAnalogous.generate();
        rybAnalogousSwatch0->backgroundColor(psychic_ui::rgb(rybAnalogous.getColor(0).getColor()));
        rybAnalogousSwatch1->backgroundColor(psychic_ui::rgb(rybAnalogous.getColor(1).getColor()));
        rybAnalogousSwatch2->backgroundColor(psychic_ui::rgb(rybAnalogous.getColor(2).getColor()));
        rybAnalogousSwatch3->backgroundColor(psychic_ui::rgb(rybAnalogous.getColor(3).getColor()));
        rybAnalogousSwatch4->backgroundColor(psychic_ui::rgb(rybAnalogous.getColor(4).getColor()));

        monochrome.generate();
        monochromeSwatch0->backgroundColor(psychic_ui::rgb(monochrome.getColor(0).getColor()));
        monochromeSwatch1->backgroundColor(psychic_ui::rgb(monochrome.getColor(1).getColor()));
        monochromeSwatch2->backgroundColor(psychic_ui::rgb(monochrome.getColor(2).getColor()));
        monochromeSwatch3->backgroundColor(psychic_ui::rgb(monochrome.getColor(3).getColor()));
        monochromeSwatch4->backgroundColor(psychic_ui::rgb(monochrome.getColor(4).getColor()));

        complementary.generate();
        complementarySwatch0->backgroundColor(psychic_ui::rgb(complementary.getColor(0).getColor()));
        complementarySwatch1->backgroundColor(psychic_ui::rgb(complementary.getColor(1).getColor()));
        complementarySwatch2->backgroundColor(psychic_ui::rgb(complementary.getColor(2).getColor()));
        complementarySwatch3->backgroundColor(psychic_ui::rgb(complementary.getColor(3).getColor()));
        complementarySwatch4->backgroundColor(psychic_ui::rgb(complementary.getColor(4).getColor()));
        complementarySwatch5->backgroundColor(psychic_ui::rgb(complementary.getColor(5).getColor()));

        rybComplementary.generate();
        rybComplementarySwatch0->backgroundColor(psychic_ui::rgb(rybComplementary.getColor(0).getColor()));
        rybComplementarySwatch1->backgroundColor(psychic_ui::rgb(rybComplementary.getColor(1).getColor()));
        rybComplementarySwatch2->backgroundColor(psychic_ui::rgb(rybComplementary.getColor(2).getColor()));
        rybComplementarySwatch3->backgroundColor(psychic_ui::rgb(rybComplementary.getColor(3).getColor()));
        rybComplementarySwatch4->backgroundColor(psychic_ui::rgb(rybComplementary.getColor(4).getColor()));
        rybComplementarySwatch5->backgroundColor(psychic_ui::rgb(rybComplementary.getColor(5).getColor()));

        splitComplementary.generate();
        splitComplementarySwatch0->backgroundColor(psychic_ui::rgb(splitComplementary.getColor(0).getColor()));
        splitComplementarySwatch1->backgroundColor(psychic_ui::rgb(splitComplementary.getColor(1).getColor()));
        splitComplementarySwatch2->backgroundColor(psychic_ui::rgb(splitComplementary.getColor(2).getColor()));

        rybSplitComplementary.generate();
        rybSplitComplementarySwatch0->backgroundColor(psychic_ui::rgb(rybSplitComplementary.getColor(0).getColor()));
        rybSplitComplementarySwatch1->backgroundColor(psychic_ui::rgb(rybSplitComplementary.getColor(1).getColor()));
        rybSplitComplementarySwatch2->backgroundColor(psychic_ui::rgb(rybSplitComplementary.getColor(2).getColor()));
        
        compound.generate();
        compoundSwatch0->backgroundColor(psychic_ui::rgb(compound.getColor(0).getColor()));
        compoundSwatch1->backgroundColor(psychic_ui::rgb(compound.getColor(1).getColor()));
        compoundSwatch2->backgroundColor(psychic_ui::rgb(compound.getColor(2).getColor()));
        compoundSwatch3->backgroundColor(psychic_ui::rgb(compound.getColor(3).getColor()));
        compoundSwatch4->backgroundColor(psychic_ui::rgb(compound.getColor(4).getColor()));
        compoundSwatch5->backgroundColor(psychic_ui::rgb(compound.getColor(5).getColor()));
        
        rybCompound.generate();
        rybCompoundSwatch0->backgroundColor(psychic_ui::rgb(rybCompound.getColor(0).getColor()));
        rybCompoundSwatch1->backgroundColor(psychic_ui::rgb(rybCompound.getColor(1).getColor()));
        rybCompoundSwatch2->backgroundColor(psychic_ui::rgb(rybCompound.getColor(2).getColor()));
        rybCompoundSwatch3->backgroundColor(psychic_ui::rgb(rybCompound.getColor(3).getColor()));
        rybCompoundSwatch4->backgroundColor(psychic_ui::rgb(rybCompound.getColor(4).getColor()));
        rybCompoundSwatch5->backgroundColor(psychic_ui::rgb(rybCompound.getColor(5).getColor()));
        
        flippedCompound.generate();
        flippedCompoundSwatch0->backgroundColor(psychic_ui::rgb(flippedCompound.getColor(0).getColor()));
        flippedCompoundSwatch1->backgroundColor(psychic_ui::rgb(flippedCompound.getColor(1).getColor()));
        flippedCompoundSwatch2->backgroundColor(psychic_ui::rgb(flippedCompound.getColor(2).getColor()));
        flippedCompoundSwatch3->backgroundColor(psychic_ui::rgb(flippedCompound.getColor(3).getColor()));
        flippedCompoundSwatch4->backgroundColor(psychic_ui::rgb(flippedCompound.getColor(4).getColor()));
        flippedCompoundSwatch5->backgroundColor(psychic_ui::rgb(flippedCompound.getColor(5).getColor()));

        rybFlippedCompound.generate();
        rybFlippedCompoundSwatch0->backgroundColor(psychic_ui::rgb(rybFlippedCompound.getColor(0).getColor()));
        rybFlippedCompoundSwatch1->backgroundColor(psychic_ui::rgb(rybFlippedCompound.getColor(1).getColor()));
        rybFlippedCompoundSwatch2->backgroundColor(psychic_ui::rgb(rybFlippedCompound.getColor(2).getColor()));
        rybFlippedCompoundSwatch3->backgroundColor(psychic_ui::rgb(rybFlippedCompound.getColor(3).getColor()));
        rybFlippedCompoundSwatch4->backgroundColor(psychic_ui::rgb(rybFlippedCompound.getColor(4).getColor()));
        rybFlippedCompoundSwatch5->backgroundColor(psychic_ui::rgb(rybFlippedCompound.getColor(5).getColor()));
        
        rybTriad.generate();
        rybTriadSwatch0->backgroundColor(psychic_ui::rgb(rybTriad.getColor(0).getColor()));
        rybTriadSwatch1->backgroundColor(psychic_ui::rgb(rybTriad.getColor(1).getColor()));
        rybTriadSwatch2->backgroundColor(psychic_ui::rgb(rybTriad.getColor(2).getColor()));

        triad.generate();
        triadSwatch0->backgroundColor(psychic_ui::rgb(triad.getColor(0).getColor()));
        triadSwatch1->backgroundColor(psychic_ui::rgb(triad.getColor(1).getColor()));
        triadSwatch2->backgroundColor(psychic_ui::rgb(triad.getColor(2).getColor()));
        
        rybTetrad.generate();
        rybTetradSwatch0->backgroundColor(psychic_ui::rgb(rybTetrad.getColor(0).getColor()));
        rybTetradSwatch1->backgroundColor(psychic_ui::rgb(rybTetrad.getColor(1).getColor()));
        rybTetradSwatch2->backgroundColor(psychic_ui::rgb(rybTetrad.getColor(2).getColor()));
        rybTetradSwatch3->backgroundColor(psychic_ui::rgb(rybTetrad.getColor(3).getColor()));

        tetrad.generate();
        tetradSwatch0->backgroundColor(psychic_ui::rgb(tetrad.getColor(0).getColor()));
        tetradSwatch1->backgroundColor(psychic_ui::rgb(tetrad.getColor(1).getColor()));
        tetradSwatch2->backgroundColor(psychic_ui::rgb(tetrad.getColor(2).getColor()));
        tetradSwatch3->backgroundColor(psychic_ui::rgb(tetrad.getColor(3).getColor()));
    }
}

