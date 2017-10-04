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
        std::shared_ptr<Div>    primarySwatch{};

        Analogous<HSB>          analogous{primary};
        std::shared_ptr<Div>    analogousSwatch0{};
        std::shared_ptr<Div>    analogousSwatch1{};
        std::shared_ptr<Div>    analogousSwatch2{};
        std::shared_ptr<Div>    analogousSwatch3{};
        std::shared_ptr<Div>    analogousSwatch4{};
        Analogous<HSB>          rybAnalogous{primary, true};
        std::shared_ptr<Div>    rybAnalogousSwatch0{};
        std::shared_ptr<Div>    rybAnalogousSwatch1{};
        std::shared_ptr<Div>    rybAnalogousSwatch2{};
        std::shared_ptr<Div>    rybAnalogousSwatch3{};
        std::shared_ptr<Div>    rybAnalogousSwatch4{};

        Monochrome<HSB>         monochrome{primary};
        std::shared_ptr<Div>    monochromeSwatch0{};
        std::shared_ptr<Div>    monochromeSwatch1{};
        std::shared_ptr<Div>    monochromeSwatch2{};
        std::shared_ptr<Div>    monochromeSwatch3{};
        std::shared_ptr<Div>    monochromeSwatch4{};

        Complementary<HSB>      complementary{primary};
        std::shared_ptr<Div>    complementarySwatch0{};
        std::shared_ptr<Div>    complementarySwatch1{};
        std::shared_ptr<Div>    complementarySwatch2{};
        std::shared_ptr<Div>    complementarySwatch3{};
        std::shared_ptr<Div>    complementarySwatch4{};
        std::shared_ptr<Div>    complementarySwatch5{};
        Complementary<HSB>      rybComplementary{primary, true};
        std::shared_ptr<Div>    rybComplementarySwatch0{};
        std::shared_ptr<Div>    rybComplementarySwatch1{};
        std::shared_ptr<Div>    rybComplementarySwatch2{};
        std::shared_ptr<Div>    rybComplementarySwatch3{};
        std::shared_ptr<Div>    rybComplementarySwatch4{};
        std::shared_ptr<Div>    rybComplementarySwatch5{};

        SplitComplementary<HSB> splitComplementary{primary};
        std::shared_ptr<Div>    splitComplementarySwatch0{};
        std::shared_ptr<Div>    splitComplementarySwatch1{};
        std::shared_ptr<Div>    splitComplementarySwatch2{};
        SplitComplementary<HSB> rybSplitComplementary{primary, true};
        std::shared_ptr<Div>    rybSplitComplementarySwatch0{};
        std::shared_ptr<Div>    rybSplitComplementarySwatch1{};
        std::shared_ptr<Div>    rybSplitComplementarySwatch2{};

        Compound<HSB>           compound{primary};
        std::shared_ptr<Div>    compoundSwatch0{};
        std::shared_ptr<Div>    compoundSwatch1{};
        std::shared_ptr<Div>    compoundSwatch2{};
        std::shared_ptr<Div>    compoundSwatch3{};
        std::shared_ptr<Div>    compoundSwatch4{};
        std::shared_ptr<Div>    compoundSwatch5{};
        Compound<HSB>           rybCompound{primary, true};
        std::shared_ptr<Div>    rybCompoundSwatch0{};
        std::shared_ptr<Div>    rybCompoundSwatch1{};
        std::shared_ptr<Div>    rybCompoundSwatch2{};
        std::shared_ptr<Div>    rybCompoundSwatch3{};
        std::shared_ptr<Div>    rybCompoundSwatch4{};
        std::shared_ptr<Div>    rybCompoundSwatch5{};

        FlippedCompound<HSB>    flippedCompound{primary};
        std::shared_ptr<Div>    flippedCompoundSwatch0{};
        std::shared_ptr<Div>    flippedCompoundSwatch1{};
        std::shared_ptr<Div>    flippedCompoundSwatch2{};
        std::shared_ptr<Div>    flippedCompoundSwatch3{};
        std::shared_ptr<Div>    flippedCompoundSwatch4{};
        std::shared_ptr<Div>    flippedCompoundSwatch5{};
        FlippedCompound<HSB>    rybFlippedCompound{primary, true};
        std::shared_ptr<Div>    rybFlippedCompoundSwatch0{};
        std::shared_ptr<Div>    rybFlippedCompoundSwatch1{};
        std::shared_ptr<Div>    rybFlippedCompoundSwatch2{};
        std::shared_ptr<Div>    rybFlippedCompoundSwatch3{};
        std::shared_ptr<Div>    rybFlippedCompoundSwatch4{};
        std::shared_ptr<Div>    rybFlippedCompoundSwatch5{};

        Triad<HSB>              triad{primary};
        std::shared_ptr<Div>    triadSwatch0{};
        std::shared_ptr<Div>    triadSwatch1{};
        std::shared_ptr<Div>    triadSwatch2{};
        Triad<HSB>              rybTriad{primary, true};
        std::shared_ptr<Div>    rybTriadSwatch0{};
        std::shared_ptr<Div>    rybTriadSwatch1{};
        std::shared_ptr<Div>    rybTriadSwatch2{};

        Tetrad<HSB>             tetrad{primary};
        std::shared_ptr<Div>    tetradSwatch0{};
        std::shared_ptr<Div>    tetradSwatch1{};
        std::shared_ptr<Div>    tetradSwatch2{};
        std::shared_ptr<Div>    tetradSwatch3{};
        Tetrad<HSB>             rybTetrad{primary, true};
        std::shared_ptr<Div>    rybTetradSwatch0{};
        std::shared_ptr<Div>    rybTetradSwatch1{};
        std::shared_ptr<Div>    rybTetradSwatch2{};
        std::shared_ptr<Div>    rybTetradSwatch3{};

        void updateSwatches();
    };

    Colors::Colors() : Div() {
        setTag("Colors");
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

            // region Colors
            auto left = container->add<VBox>();
            left
                ->style()
                ->set(grow, 1)
                ->set(basis, 0) // Both columns needs to be considered the same size
                ->set(gap, 6);

            // region Colors Content
            {
                left->add<Label>("ARGB")
                    ->addClassName("h1")
                    ->addClassName("title");

                auto swatch = left->add<Div>()->addClassName("swatch");

                left->add<Range<float>>(
                    [this, swatch](float value) {
                        argb.setAlpha(value);
                        swatch->style()->set(backgroundColor, argb.getColor());
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        argb.setRed(value);
                        swatch->style()->set(backgroundColor, argb.getColor());
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        argb.setGreen(value);
                        swatch->style()->set(backgroundColor, argb.getColor());
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        argb.setBlue(value);
                        swatch->style()->set(backgroundColor, argb.getColor());
                    }
                );
            }

            {
                left->add<Label>("RGB")
                    ->addClassName("h1")
                    ->addClassName("title");

                auto swatch = left->add<Div>()->addClassName("swatch");

                left->add<Range<float>>(
                    [this, swatch](float value) {
                        rgb.setRed(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(rgb.getColor(), 255));
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        rgb.setGreen(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(rgb.getColor(), 255));
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        rgb.setBlue(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(rgb.getColor(), 255));
                    }
                );
            }

            {
                left->add<Label>("Hex")
                    ->addClassName("h1")
                    ->addClassName("title");

                auto swatch = left->add<Div>()->addClassName("swatch");

                left->add<Range<unsigned int>>(
                    0, 255,
                    [this, swatch](unsigned int value) {
                        hex.setAlpha(value);
                        swatch->style()->set(backgroundColor, hex.getColor());
                    }
                );
                left->add<Range<unsigned int>>(
                    0, 255,
                    [this, swatch](unsigned int value) {
                        hex.setRed(value);
                        swatch->style()->set(backgroundColor, hex.getColor());
                    }
                );
                left->add<Range<unsigned int>>(
                    0, 255,
                    [this, swatch](unsigned int value) {
                        hex.setGreen(value);
                        swatch->style()->set(backgroundColor, hex.getColor());
                    }
                );
                left->add<Range<unsigned int>>(
                    0, 255,
                    [this, swatch](unsigned int value) {
                        hex.setBlue(value);
                        swatch->style()->set(backgroundColor, hex.getColor());
                    }
                );
            }

            {
                left->add<Label>("Gray")
                    ->addClassName("h1")
                    ->addClassName("title");

                auto swatch = left->add<Div>()->addClassName("swatch");

                left->add<Range<float>>(
                    [this, swatch](float value) {
                        gray.setGray(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(gray.getColor(), 255));
                    }
                );
            }

            {
                left->add<Label>("HSB")
                    ->addClassName("h1")
                    ->addClassName("title");

                auto swatch = left->add<Div>()->addClassName("swatch");

                left->add<Range<float>>(
                    [this, swatch](float value) {
                        hsb.setHue(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(hsb.getColor(), 255));
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        hsb.setSaturation(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(hsb.getColor(), 255));
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        hsb.setBrightness(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(hsb.getColor(), 255));
                    }
                );
            }

            {
                left->add<Label>("HSL")
                    ->addClassName("h1")
                    ->addClassName("title");

                auto swatch = left->add<Div>()->addClassName("swatch");

                left->add<Range<float>>(
                    [this, swatch](float value) {
                        hsl.setHue(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(hsl.getColor(), 255));
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        hsl.setSaturation(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(hsl.getColor(), 255));
                    }
                );
                left->add<Range<float>>(
                    [this, swatch](float value) {
                        hsl.setLightness(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(hsl.getColor(), 255));
                    }
                );
            }
            // endregion

            // endregion

            // region Schemes
            auto right = container->add<VBox>();
            right
                ->addClassName("color-schemes")
                ->style()
                ->set(grow, 1)
                ->set(basis, 0) // Both columns needs to be considered the same size
                ->set(gap, 6);

            // region Schemes Content
            {
                right->add<Label>("Primary Color (HSB)")
                     ->addClassName("h1")
                     ->addClassName("title");

                primarySwatch = right->add<Div>();
                primarySwatch->addClassName("swatch");

                right->add<Range<float>>(
                    [this](float value) {
                        primary.setHue(value);
                        updateSwatches();
                    }
                );
                right->add<Range<float>>(
                    [this](float value) {
                        primary.setSaturation(value);
                        updateSwatches();
                    }
                );
                right->add<Range<float>>(
                    [this](float value) {
                        primary.setBrightness(value);
                        updateSwatches();
                    }
                );

                {
                    right->add<Label>("Monochrome")
                         ->addClassName("h2")
                         ->addClassName("title");

                    right->add<Label>("Contrast");
                    auto contrast = right->add<Range<float>>(
                        0.25f,
                        [this](float value) {
                            monochrome.setContrast(value);
                            updateSwatches();
                        }
                    );

                    auto swatches = right->add<Div>();
                    swatches->addClassName("swatches");
                    monochromeSwatch0 = swatches->add<Div>();
                    monochromeSwatch0->addClassName("swatch");
                    monochromeSwatch1 = swatches->add<Div>();
                    monochromeSwatch1->addClassName("swatch");
                    monochromeSwatch2 = swatches->add<Div>();
                    monochromeSwatch2->addClassName("swatch");
                    monochromeSwatch3 = swatches->add<Div>();
                    monochromeSwatch3->addClassName("swatch");
                    monochromeSwatch4 = swatches->add<Div>();
                    monochromeSwatch4->addClassName("swatch");
                }

                {
                    right->add<Label>("Analogous")
                         ->addClassName("h2")
                         ->addClassName("title");

                    right->add<Label>("Angle");
                    auto angle = right->add<Range<float>>(
                        0.0f, 90.0f, 15.0f,
                        [this](float value) {
                            rybAnalogous.setAngle(value);
                            analogous.setAngle(value);
                            updateSwatches();
                        }
                    );

                    right->add<Label>("Artistic");
                    auto rybSwatches = right->add<Div>();
                    rybSwatches->addClassName("swatches");
                    rybAnalogousSwatch0 = rybSwatches->add<Div>();
                    rybAnalogousSwatch0->addClassName("swatch");
                    rybAnalogousSwatch1 = rybSwatches->add<Div>();
                    rybAnalogousSwatch1->addClassName("swatch");
                    rybAnalogousSwatch2 = rybSwatches->add<Div>();
                    rybAnalogousSwatch2->addClassName("swatch");
                    rybAnalogousSwatch3 = rybSwatches->add<Div>();
                    rybAnalogousSwatch3->addClassName("swatch");
                    rybAnalogousSwatch4 = rybSwatches->add<Div>();
                    rybAnalogousSwatch4->addClassName("swatch");

                    right->add<Label>("Scientific");
                    auto swatches = right->add<Div>();
                    swatches->addClassName("swatches");
                    analogousSwatch0 = swatches->add<Div>();
                    analogousSwatch0->addClassName("swatch");
                    analogousSwatch1 = swatches->add<Div>();
                    analogousSwatch1->addClassName("swatch");
                    analogousSwatch2 = swatches->add<Div>();
                    analogousSwatch2->addClassName("swatch");
                    analogousSwatch3 = swatches->add<Div>();
                    analogousSwatch3->addClassName("swatch");
                    analogousSwatch4 = swatches->add<Div>();
                    analogousSwatch4->addClassName("swatch");
                }

                {
                    right->add<Label>("Complementary")
                         ->addClassName("h2")
                         ->addClassName("title");

                    right->add<Label>("Artistic");
                    auto rybSwatches = right->add<Div>();
                    rybSwatches->addClassName("swatches");
                    rybComplementarySwatch0 = rybSwatches->add<Div>();
                    rybComplementarySwatch0->addClassName("swatch");
                    rybComplementarySwatch1 = rybSwatches->add<Div>();
                    rybComplementarySwatch1->addClassName("swatch");
                    rybComplementarySwatch2 = rybSwatches->add<Div>();
                    rybComplementarySwatch2->addClassName("swatch");
                    rybComplementarySwatch3 = rybSwatches->add<Div>();
                    rybComplementarySwatch3->addClassName("swatch");
                    rybComplementarySwatch4 = rybSwatches->add<Div>();
                    rybComplementarySwatch4->addClassName("swatch");
                    rybComplementarySwatch5 = rybSwatches->add<Div>();
                    rybComplementarySwatch5->addClassName("swatch");

                    right->add<Label>("Scientific");
                    auto swatches = right->add<Div>();
                    swatches->addClassName("swatches");
                    complementarySwatch0 = swatches->add<Div>();
                    complementarySwatch0->addClassName("swatch");
                    complementarySwatch1 = swatches->add<Div>();
                    complementarySwatch1->addClassName("swatch");
                    complementarySwatch2 = swatches->add<Div>();
                    complementarySwatch2->addClassName("swatch");
                    complementarySwatch3 = swatches->add<Div>();
                    complementarySwatch3->addClassName("swatch");
                    complementarySwatch4 = swatches->add<Div>();
                    complementarySwatch4->addClassName("swatch");
                    complementarySwatch5 = swatches->add<Div>();
                    complementarySwatch5->addClassName("swatch");
                }

                {
                    right->add<Label>("Split Complementary")
                         ->addClassName("h2")
                         ->addClassName("title");

                    right->add<Label>("Angle");
                    auto range = right->add<Range<float>>(
                        0.0f, 180.0f, 30.0f,
                        [this](float value) {
                            rybSplitComplementary.setAngle(value);
                            splitComplementary.setAngle(value);
                            updateSwatches();
                        }
                    );

                    right->add<Label>("Artistic");
                    auto rybSwatches = right->add<Div>();
                    rybSwatches->addClassName("swatches");
                    rybSplitComplementarySwatch0 = rybSwatches->add<Div>();
                    rybSplitComplementarySwatch0->addClassName("swatch");
                    rybSplitComplementarySwatch1 = rybSwatches->add<Div>();
                    rybSplitComplementarySwatch1->addClassName("swatch");
                    rybSplitComplementarySwatch2 = rybSwatches->add<Div>();
                    rybSplitComplementarySwatch2->addClassName("swatch");

                    right->add<Label>("Scientific");
                    auto swatches = right->add<Div>();
                    swatches->addClassName("swatches");
                    splitComplementarySwatch0 = swatches->add<Div>();
                    splitComplementarySwatch0->addClassName("swatch");
                    splitComplementarySwatch1 = swatches->add<Div>();
                    splitComplementarySwatch1->addClassName("swatch");
                    splitComplementarySwatch2 = swatches->add<Div>();
                    splitComplementarySwatch2->addClassName("swatch");

                }

                {
                    right->add<Label>("Compound")
                         ->addClassName("h2")
                         ->addClassName("title");
                         
                    right->add<Label>("Artistic");
                    auto rybSwatches = right->add<Div>();
                    rybSwatches->addClassName("swatches");
                    rybCompoundSwatch0 = rybSwatches->add<Div>();
                    rybCompoundSwatch0->addClassName("swatch");
                    rybCompoundSwatch1 = rybSwatches->add<Div>();
                    rybCompoundSwatch1->addClassName("swatch");
                    rybCompoundSwatch2 = rybSwatches->add<Div>();
                    rybCompoundSwatch2->addClassName("swatch");
                    rybCompoundSwatch3 = rybSwatches->add<Div>();
                    rybCompoundSwatch3->addClassName("swatch");
                    rybCompoundSwatch4 = rybSwatches->add<Div>();
                    rybCompoundSwatch4->addClassName("swatch");
                    rybCompoundSwatch5 = rybSwatches->add<Div>();
                    rybCompoundSwatch5->addClassName("swatch");
                    
                    right->add<Label>("Scientific");
                    auto swatches = right->add<Div>();
                    swatches->addClassName("swatches");
                    compoundSwatch0 = swatches->add<Div>();
                    compoundSwatch0->addClassName("swatch");
                    compoundSwatch1 = swatches->add<Div>();
                    compoundSwatch1->addClassName("swatch");
                    compoundSwatch2 = swatches->add<Div>();
                    compoundSwatch2->addClassName("swatch");
                    compoundSwatch3 = swatches->add<Div>();
                    compoundSwatch3->addClassName("swatch");
                    compoundSwatch4 = swatches->add<Div>();
                    compoundSwatch4->addClassName("swatch");
                    compoundSwatch5 = swatches->add<Div>();
                    compoundSwatch5->addClassName("swatch");
                }

                {
                    right->add<Label>("Flipped Compound")
                         ->addClassName("h2")
                         ->addClassName("title");
                         
                    right->add<Label>("Artistic");
                    auto rybSwatches = right->add<Div>();
                    rybSwatches->addClassName("swatches");
                    rybFlippedCompoundSwatch0 = rybSwatches->add<Div>();
                    rybFlippedCompoundSwatch0->addClassName("swatch");
                    rybFlippedCompoundSwatch1 = rybSwatches->add<Div>();
                    rybFlippedCompoundSwatch1->addClassName("swatch");
                    rybFlippedCompoundSwatch2 = rybSwatches->add<Div>();
                    rybFlippedCompoundSwatch2->addClassName("swatch");
                    rybFlippedCompoundSwatch3 = rybSwatches->add<Div>();
                    rybFlippedCompoundSwatch3->addClassName("swatch");
                    rybFlippedCompoundSwatch4 = rybSwatches->add<Div>();
                    rybFlippedCompoundSwatch4->addClassName("swatch");
                    rybFlippedCompoundSwatch5 = rybSwatches->add<Div>();
                    rybFlippedCompoundSwatch5->addClassName("swatch");
                    
                    right->add<Label>("Scientific");
                    auto swatches = right->add<Div>();
                    swatches->addClassName("swatches");
                    flippedCompoundSwatch0 = swatches->add<Div>();
                    flippedCompoundSwatch0->addClassName("swatch");
                    flippedCompoundSwatch1 = swatches->add<Div>();
                    flippedCompoundSwatch1->addClassName("swatch");
                    flippedCompoundSwatch2 = swatches->add<Div>();
                    flippedCompoundSwatch2->addClassName("swatch");
                    flippedCompoundSwatch3 = swatches->add<Div>();
                    flippedCompoundSwatch3->addClassName("swatch");
                    flippedCompoundSwatch4 = swatches->add<Div>();
                    flippedCompoundSwatch4->addClassName("swatch");
                    flippedCompoundSwatch5 = swatches->add<Div>();
                    flippedCompoundSwatch5->addClassName("swatch");
                }

                {
                    right->add<Label>("Triad")
                         ->addClassName("h2")
                         ->addClassName("title");

                    right->add<Label>("Artistic");
                    auto rybSwatches = right->add<Div>();
                    rybSwatches->addClassName("swatches");
                    rybTriadSwatch0 = rybSwatches->add<Div>();
                    rybTriadSwatch0->addClassName("swatch");
                    rybTriadSwatch1 = rybSwatches->add<Div>();
                    rybTriadSwatch1->addClassName("swatch");
                    rybTriadSwatch2 = rybSwatches->add<Div>();
                    rybTriadSwatch2->addClassName("swatch");

                    right->add<Label>("Scientific");
                    auto swatches = right->add<Div>();
                    swatches->addClassName("swatches");
                    triadSwatch0 = swatches->add<Div>();
                    triadSwatch0->addClassName("swatch");
                    triadSwatch1 = swatches->add<Div>();
                    triadSwatch1->addClassName("swatch");
                    triadSwatch2 = swatches->add<Div>();
                    triadSwatch2->addClassName("swatch");
                }

                {
                    right->add<Label>("Tetrad")
                         ->addClassName("h2")
                         ->addClassName("title");

                    right->add<Label>("Angle");
                    auto range = right->add<Range<float>>(
                        0.0f, 180.0f, 90.0f,
                        [this](float value) {
                            rybTetrad.setAngle(value);
                            tetrad.setAngle(value);
                            updateSwatches();
                        }
                    );

                    right->add<Label>("Artistic");
                    auto rybSwatches = right->add<Div>();
                    rybSwatches->addClassName("swatches");
                    rybTetradSwatch0 = rybSwatches->add<Div>();
                    rybTetradSwatch0->addClassName("swatch");
                    rybTetradSwatch1 = rybSwatches->add<Div>();
                    rybTetradSwatch1->addClassName("swatch");
                    rybTetradSwatch2 = rybSwatches->add<Div>();
                    rybTetradSwatch2->addClassName("swatch");
                    rybTetradSwatch3 = rybSwatches->add<Div>();
                    rybTetradSwatch3->addClassName("swatch");

                    right->add<Label>("Scientific");
                    auto swatches = right->add<Div>();
                    swatches->addClassName("swatches");
                    tetradSwatch0 = swatches->add<Div>();
                    tetradSwatch0->addClassName("swatch");
                    tetradSwatch1 = swatches->add<Div>();
                    tetradSwatch1->addClassName("swatch");
                    tetradSwatch2 = swatches->add<Div>();
                    tetradSwatch2->addClassName("swatch");
                    tetradSwatch3 = swatches->add<Div>();
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
    void Colors::updateSwatches() {
        primarySwatch->style()->set(backgroundColor, PsychicColor::setColorAlpha(primary.getColor(), 255));

        analogous.generate();
        analogousSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(analogous.getColor(0).getColor(), 255));
        analogousSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(analogous.getColor(1).getColor(), 255));
        analogousSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(analogous.getColor(2).getColor(), 255));
        analogousSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(analogous.getColor(3).getColor(), 255));
        analogousSwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(analogous.getColor(4).getColor(), 255));

        rybAnalogous.generate();
        rybAnalogousSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybAnalogous.getColor(0).getColor(), 255));
        rybAnalogousSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybAnalogous.getColor(1).getColor(), 255));
        rybAnalogousSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybAnalogous.getColor(2).getColor(), 255));
        rybAnalogousSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybAnalogous.getColor(3).getColor(), 255));
        rybAnalogousSwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybAnalogous.getColor(4).getColor(), 255));

        monochrome.generate();
        monochromeSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(monochrome.getColor(0).getColor(), 255));
        monochromeSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(monochrome.getColor(1).getColor(), 255));
        monochromeSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(monochrome.getColor(2).getColor(), 255));
        monochromeSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(monochrome.getColor(3).getColor(), 255));
        monochromeSwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(monochrome.getColor(4).getColor(), 255));

        complementary.generate();
        complementarySwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(complementary.getColor(0).getColor(), 255));
        complementarySwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(complementary.getColor(1).getColor(), 255));
        complementarySwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(complementary.getColor(2).getColor(), 255));
        complementarySwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(complementary.getColor(3).getColor(), 255));
        complementarySwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(complementary.getColor(4).getColor(), 255));
        complementarySwatch5->style()->set(backgroundColor, PsychicColor::setColorAlpha(complementary.getColor(5).getColor(), 255));

        rybComplementary.generate();
        rybComplementarySwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybComplementary.getColor(0).getColor(), 255));
        rybComplementarySwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybComplementary.getColor(1).getColor(), 255));
        rybComplementarySwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybComplementary.getColor(2).getColor(), 255));
        rybComplementarySwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybComplementary.getColor(3).getColor(), 255));
        rybComplementarySwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybComplementary.getColor(4).getColor(), 255));
        rybComplementarySwatch5->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybComplementary.getColor(5).getColor(), 255));

        splitComplementary.generate();
        splitComplementarySwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(splitComplementary.getColor(0).getColor(), 255));
        splitComplementarySwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(splitComplementary.getColor(1).getColor(), 255));
        splitComplementarySwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(splitComplementary.getColor(2).getColor(), 255));

        rybSplitComplementary.generate();
        rybSplitComplementarySwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybSplitComplementary.getColor(0).getColor(), 255));
        rybSplitComplementarySwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybSplitComplementary.getColor(1).getColor(), 255));
        rybSplitComplementarySwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybSplitComplementary.getColor(2).getColor(), 255));
        
        compound.generate();
        compoundSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(compound.getColor(0).getColor(), 255));
        compoundSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(compound.getColor(1).getColor(), 255));
        compoundSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(compound.getColor(2).getColor(), 255));
        compoundSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(compound.getColor(3).getColor(), 255));
        compoundSwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(compound.getColor(4).getColor(), 255));
        compoundSwatch5->style()->set(backgroundColor, PsychicColor::setColorAlpha(compound.getColor(5).getColor(), 255));
        
        rybCompound.generate();
        rybCompoundSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybCompound.getColor(0).getColor(), 255));
        rybCompoundSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybCompound.getColor(1).getColor(), 255));
        rybCompoundSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybCompound.getColor(2).getColor(), 255));
        rybCompoundSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybCompound.getColor(3).getColor(), 255));
        rybCompoundSwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybCompound.getColor(4).getColor(), 255));
        rybCompoundSwatch5->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybCompound.getColor(5).getColor(), 255));
        
        flippedCompound.generate();
        flippedCompoundSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(flippedCompound.getColor(0).getColor(), 255));
        flippedCompoundSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(flippedCompound.getColor(1).getColor(), 255));
        flippedCompoundSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(flippedCompound.getColor(2).getColor(), 255));
        flippedCompoundSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(flippedCompound.getColor(3).getColor(), 255));
        flippedCompoundSwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(flippedCompound.getColor(4).getColor(), 255));
        flippedCompoundSwatch5->style()->set(backgroundColor, PsychicColor::setColorAlpha(flippedCompound.getColor(5).getColor(), 255));

        rybFlippedCompound.generate();
        rybFlippedCompoundSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybFlippedCompound.getColor(0).getColor(), 255));
        rybFlippedCompoundSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybFlippedCompound.getColor(1).getColor(), 255));
        rybFlippedCompoundSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybFlippedCompound.getColor(2).getColor(), 255));
        rybFlippedCompoundSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybFlippedCompound.getColor(3).getColor(), 255));
        rybFlippedCompoundSwatch4->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybFlippedCompound.getColor(4).getColor(), 255));
        rybFlippedCompoundSwatch5->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybFlippedCompound.getColor(5).getColor(), 255));
        
        rybTriad.generate();
        rybTriadSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybTriad.getColor(0).getColor(), 255));
        rybTriadSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybTriad.getColor(1).getColor(), 255));
        rybTriadSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybTriad.getColor(2).getColor(), 255));

        triad.generate();
        triadSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(triad.getColor(0).getColor(), 255));
        triadSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(triad.getColor(1).getColor(), 255));
        triadSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(triad.getColor(2).getColor(), 255));
        
        rybTetrad.generate();
        rybTetradSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybTetrad.getColor(0).getColor(), 255));
        rybTetradSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybTetrad.getColor(1).getColor(), 255));
        rybTetradSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybTetrad.getColor(2).getColor(), 255));
        rybTetradSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(rybTetrad.getColor(3).getColor(), 255));

        tetrad.generate();
        tetradSwatch0->style()->set(backgroundColor, PsychicColor::setColorAlpha(tetrad.getColor(0).getColor(), 255));
        tetradSwatch1->style()->set(backgroundColor, PsychicColor::setColorAlpha(tetrad.getColor(1).getColor(), 255));
        tetradSwatch2->style()->set(backgroundColor, PsychicColor::setColorAlpha(tetrad.getColor(2).getColor(), 255));
        tetradSwatch3->style()->set(backgroundColor, PsychicColor::setColorAlpha(tetrad.getColor(3).getColor(), 255));
    }
}

