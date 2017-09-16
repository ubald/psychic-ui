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
#include <lib/psychic-color/psychic-color/PsychicColor.hpp>

namespace psychic_ui {

    using namespace psychic_color;

    class Colors : public Div {
    public:
        Colors();
    protected:
        ARGB           argb{};
        RGB            rgb{};
        Hex            hex{};
        Gray           gray{};
        HSB            hsb{};
        HSL            hsl{};
        HSB            primary{};
        Analogous<HSB> analogous{primary};
    };

    Colors::Colors() : Div() {
        setTag("Divs");
        setClassNames({"demo-panel"});

        styleManager()
            ->style(".swatch")
            ->set(borderRadius, 6)
            ->set(shrink, 0)
            ->set(height, 64.0f);

        styleManager()
            ->style(".swatches")
            ->set(direction, "row");

        styleManager()
            ->style(".swatches .swatch")
            ->set(grow, 1);

        styleManager()
            ->style(".column .title")
            ->set(marginBottom, 6);

        styleManager()
            ->style(".column .swatch")
            ->set(marginBottom, 6);

        styleManager()
            ->style(".column Range")
            ->set(shrink, 0)
            ->set(marginBottom, 6);

        style()
            ->set(direction, "row")
            ->set(padding, 0); // We're using a scroller so no padding

        auto scroller = add<Div>();
        scroller
            ->style()
            ->set(shrink, 1)
            ->set(widthPercent, 1.0f)
            ->set(heightPercent, 1.0f)
            ->set(padding, 24)
            ->set(overflow, "scroll")
            ->set(wrap, "wrap")
            ->set(direction, "row");

        auto scrollbar = add<ScrollBar>(scroller);
        scrollbar
            ->style()
            ->set(heightPercent, 1.0f);

        {
            auto container = scroller->add<Div>();
            container
                ->style()
                ->set(widthPercent, 1.0f)
                ->set(direction, "row");

            // Anti Aliasing

            auto left = container->add<Div>();
            left
                ->addClassName("column")
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1)
                ->set(widthPercent, 0.5f);

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

            // Schemes

            auto right = container->add<Div>();
            right
                ->addClassName("column")
                ->style()
                ->set(grow, 1)
                ->set(shrink, 1)
                ->set(widthPercent, 0.5f);

            {
                right->add<Label>("Primary Color (HSB)")
                     ->addClassName("h1")
                     ->addClassName("title");

                auto swatch = right->add<Div>()->addClassName("swatch");

                right->add<Range<float>>(
                    [this, swatch](float value) {
                        primary.setHue(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(primary.getColor(), 255));
                    }
                );
                right->add<Range<float>>(
                    [this, swatch](float value) {
                        primary.setSaturation(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(primary.getColor(), 255));
                    }
                );
                right->add<Range<float>>(
                    [this, swatch](float value) {
                        primary.setBrightness(value);
                        swatch->style()
                              ->set(backgroundColor, PsychicColor::setColorAlpha(primary.getColor(), 255));
                    }
                );

                {
                    right->add<Label>("Analogous")
                         ->addClassName("h2")
                         ->addClassName("title");

                    auto swatches = right->add<Div>();
                    swatches
                        ->addClassName("swatches");

                    auto swatch0 = swatches->add<Div>()->addClassName("swatch");
                    auto swatch1 = swatches->add<Div>()->addClassName("swatch");
                    auto swatch2 = swatches->add<Div>()->addClassName("swatch");
                    auto swatch3 = swatches->add<Div>()->addClassName("swatch");
                    auto swatch4 = swatches->add<Div>()->addClassName("swatch");

                    auto update = [this, swatch0, swatch1, swatch2, swatch3, swatch4]() {
                        std::cout << analogous.getColor(0) << std::endl;
                        swatch0->style()
                               ->set(
                                   backgroundColor,
                                   PsychicColor::setColorAlpha(analogous.getColor(0).getColor(), 255));
                        swatch1->style()
                               ->set(
                                   backgroundColor,
                                   PsychicColor::setColorAlpha(analogous.getColor(1).getColor(), 255));
                        swatch2->style()
                               ->set(
                                   backgroundColor,
                                   PsychicColor::setColorAlpha(analogous.getColor(2).getColor(), 255));
                        swatch3->style()
                               ->set(
                                   backgroundColor,
                                   PsychicColor::setColorAlpha(analogous.getColor(3).getColor(), 255));
                        swatch4->style()
                               ->set(
                                   backgroundColor,
                                   PsychicColor::setColorAlpha(analogous.getColor(4).getColor(), 255));
                    };
                    right->add<Label>("Angle");
                    right->add<Range<float>>(
                        0.0f, 360.0f,
                        [this, update](float value) {
                            analogous.setAngle(value);
                            update();
                        }
                    );
                    right->add<Label>("Contrast");
                    right->add<Range<float>>(
                        [this, update](float value) {
                            analogous.setContrast(value);
                            update();
                        }
                    );
                }
            }
        }
    }


}

