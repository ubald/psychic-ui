#pragma once

#include <chrono>
#include <format>
#include <algorithm>
#include <functional>
#include <random>
#include <string>
#include <vector>
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/animation/Animation.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/List.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/SegmentedControl.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/TextInput.hpp>

namespace psychic_ui {
    using namespace std::chrono_literals;

    /**
     * A list whose rows can be shuffled, keeping the selection: they slide to their new places (the default set's
     * `List` has a `layoutTransition`)
     */
    class ShuffleList : public List {
    public:
        void shuffle() {
            std::vector<std::shared_ptr<Div>> rows{children().begin(), children().end()};
            const auto before = rows;
            while (rows.size() > 1 && rows == before) {
                std::shuffle(rows.begin(), rows.end(), _random);
            }
            reorder(*this, rows);
        }

    private:
        std::mt19937 _random{2026};
    };

    /**
     * A lane whose puck is drawn by hand, from an `Animated<float>`: the tween primitive, for what isn't a style
     */
    class DrawnLane : public Div {
    public:
        DrawnLane() : Div() {
            tag("DrawnLane");
        }

        void toggle(bool forward) {
            _position.to(forward ? 1.0f : 0.0f);
        }

    protected:
        Animated<float> _position{this, 0.0f, Timing{Spring::Bouncy}};

        void draw(SkCanvas *canvas) override {
            Div::draw(canvas);
            constexpr float size  = 20.0f;
            const float     inset = (_rect.height() - size) / 2.0f;
            const float     x     = _rect.left() + inset + (_position.value() * (_rect.width() - size - 2.0f * inset));
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(_computedStyle.color().argb());
            canvas->drawCircle(x + (size / 2.0f), _rect.centerY(), size / 2.0f, paint);
        }
    };

    /**
     * An easing curve, or a spring's response, plotted over time, with a dot that runs along it when played
     */
    class CurveView : public Div {
    public:
        explicit CurveView(Timing timing) : Div(), _timing(timing) {
            tag("CurveView");
        }

        void play() {
            animate(0.0f, 1.0f, Timing{plotDuration(), Ease::Linear}, [this](float t) { _time = t; });
        }

    protected:
        Timing _timing;
        float  _time{0.0f}; // Of the plot's width

        /**
         * The time the plot's width shows: the duration, or 800 ms for a spring
         */
        std::chrono::nanoseconds plotDuration() const {
            return _timing.isSpring() ? std::chrono::nanoseconds{800ms} : _timing.duration();
        }

        /**
         * Where the value is (0 start, 1 end) at a share of the plot's time: from an offset of -1 to its target, 1
         */
        float valueAt(float t) const {
            const float seconds = t * std::chrono::duration<float>(plotDuration()).count();
            return 1.0f + _timing.approach(-1.0f, 0.0f, seconds, 1e-3f).offset;
        }

        void draw(SkCanvas *canvas) override {
            Div::draw(canvas);
            // Room for an overshoot above and an undershoot below
            const SkRect plot    = _rect.makeInset(10.0f, _rect.height() * 0.2f);
            const auto   pointAt = [&plot](float t, float value) {
                return SkPoint::Make(plot.left() + (t * plot.width()), plot.bottom() - (value * plot.height()));
            };

            SkPaint line;
            line.setAntiAlias(true);
            line.setStyle(SkPaint::kStroke_Style);
            line.setStrokeWidth(1.0f);
            line.setColor(_computedStyle.borderColor().argb());
            canvas->drawLine(pointAt(0, 0), pointAt(1, 0), line);
            canvas->drawLine(pointAt(0, 1), pointAt(1, 1), line);

            SkPathBuilder path;
            path.moveTo(pointAt(0, valueAt(0)));
            for (int i = 1; i <= 64; ++i) {
                const float t = static_cast<float>(i) / 64.0f;
                path.lineTo(pointAt(t, valueAt(t)));
            }
            line.setStrokeWidth(1.5f);
            line.setColor(_computedStyle.color().argb());
            canvas->drawPath(path.detach(), line);

            SkPaint dot;
            dot.setAntiAlias(true);
            dot.setColor(_computedStyle.color().argb());
            canvas->drawCircle(pointAt(_time, valueAt(_time)), 4.0f, dot);
        }
    };

    /**
     * Motion: eased, spring and instant side by side, the curves, the theme's tokens, reduced motion, and controls
     * whose colors blend
     */
    class MotionPanel : public Div {
    public:
        MotionPanel();

    protected:
        void addedToRender() override;
        void styleUpdated() override;

    private:
        std::vector<Div *>       _pucks{};
        DrawnLane               *_drawn{nullptr};
        std::vector<CurveView *> _curves{};
        Switch                  *_reduced{nullptr};
        Label                   *_tokens{nullptr};
        std::string             _theme{};
        bool                    _on{false};

        Div &group(const std::string &title, const std::string &caption);
        void play();
        void showTokens();
    };

    inline Div &MotionPanel::group(const std::string &title, const std::string &caption) {
        auto &group = add<VBox>().addClassName("group");
        group.add<Label>(title).addClassName("h4");
        group.add<Label>(caption).addClassName("text-tertiary");
        return group;
    }

    inline MotionPanel::MotionPanel() : Div() {
        tag("Motion")
            .classNames({"demo-panel"});

        // region Lanes

        auto &lanes = group(
            "Eased, spring, instant",
            "Play, then play again half-way: the eased pucks start over from where they are, the springs keep their speed. "
            "Reduced motion makes every duration 0."
        );
        auto &bar = lanes.add<HBox>().addClassName("row");
        bar.add<Button>("Play").icon("play").skin("primary").onClick([this] { play(); });
        // Every duration becomes 0, values jump to their end (as with --reduced-motion)
        _reduced = &bar.add<Switch>("Reduce motion");
        _reduced->onChange([this](bool reduced) {
            if (Window *w = window()) {
                w->reducedMotion(reduced);
            }
        });

        const auto lane = [&](const std::string &name, const Timing &timing) {
            auto &row = lanes.add<HBox>().addClassName("lane-row");
            row.add<Label>(name).addClassName("name").addClassName("text-secondary");
            // The rail is the track less the puck's width: the puck goes from 0 to 100% of it
            auto &rail = row.add<Div>().addClassName("lane").add<Div>().addClassName("rail");
            // A style transition on `left`, inline: a layout property, laid out every frame while it moves
            auto &puck = rail.add<Div>().addClassName("puck").left(0_pct).transition(Property::left, timing);
            _pucks.push_back(&puck);
        };
        lane("Eased · 400 ms, ease-out", Timing{400ms, Ease::Out});
        lane("Eased · 400 ms, Pivotal's out", Timing{400ms, Ease{.x1 = 0.16f, .y1 = 1.0f, .x2 = 0.3f, .y2 = 1.0f}});
        lane("Spring · Snappy", Timing{Spring::Snappy});
        lane("Spring · Gentle", Timing{Spring::Gentle});
        lane("Instant", Timing{});

        auto &drawn = lanes.add<HBox>().addClassName("lane-row");
        drawn.add<Label>("Animated<float> · Bouncy").addClassName("name").addClassName("text-secondary");
        _drawn = &drawn.add<DrawnLane>();
        _drawn->addClassName("lane");

        // endregion

        // region Layout transitions

        auto &layout = group(
            "Layout transitions",
            "A container's layoutTransition slides its children from where they were painted to where a new layout puts "
            "them: lists, trees, the sidebar and toasts at the arrive timing, a sorted grid at the slow step"
        );
        auto &shuffleBar = layout.add<HBox>().addClassName("row");
        auto &list       = layout.add<ShuffleList>();
        list.addClassName("sm").addClassName("shuffle");
        for (const char *name: {"Wash 1", "Wash 2", "Spot left", "Spot right", "Strobe"}) {
            list.row(name);
        }
        list.select(2);
        shuffleBar.add<Button>("Shuffle").icon("arrow-up-down").onClick([&list] { list.shuffle(); });

        // endregion

        // region Curves

        auto &curves = group("Curves", "CSS's named curves as cubic Béziers, Pivotal's, and springs over 800 ms");
        auto &plots  = curves.add<HBox>().addClassName("curves");
        for (const auto &[name, timing]: std::vector<std::pair<std::string, Timing>>{
                 {"Linear", Timing{600ms, Ease::Linear}},
                 {"Standard (ease)", Timing{600ms, Ease::Standard}},
                 {"In", Timing{600ms, Ease::In}},
                 {"Out", Timing{600ms, Ease::Out}},
                 {"In out", Timing{600ms, Ease::InOut}},
                 {"Pivotal's out", Timing{600ms, Ease{.x1 = 0.16f, .y1 = 1.0f, .x2 = 0.3f, .y2 = 1.0f}}},
                 {"Spring · Snappy", Timing{Spring::Snappy}},
                 {"Spring · Bouncy", Timing{Spring::Bouncy}},
             }) {
            auto &cell = plots.add<VBox>().addClassName("curve-cell");
            _curves.push_back(&cell.add<CurveView>(timing));
            _curves.back()->addClassName("curve");
            cell.add<Label>(name).addClassName("text-tertiary");
        }

        // endregion

        // region Tokens

        auto &tokens = group("The theme's motion", "Metrics::motion: what the skin set's transitions use");
        _tokens = &tokens.add<Label>("").addClassName("tokens");

        // endregion

        // region Controls

        auto &controls = group(
            "Play with it",
            "Hover, press and Tab: colors blend, the focus ring fades in; the switch's thumb springs, the slider glides on keys"
        );
        auto &row = controls.add<HBox>().addClassName("row");
        row.add<Button>("Secondary");
        row.add<Button>("Primary").skin("primary");
        row.add<Button>("Ghost").skin("ghost");
        row.add<Switch>("Switch");
        row.add<CheckBox>("Check box");
        auto &more = controls.add<HBox>().addClassName("row");
        more.add<SegmentedControl>(std::vector<Segment>{{"Day"}, {"Week"}, {"Month"}}).selected(0);
        more.add<Range<int>>(0, 10, 3, nullptr).addClassName("slider");
        more.add<TextInput>("").placeholder("Focus me");

        // endregion
    }

    inline void MotionPanel::addedToRender() {
        Div::addedToRender();
        _reduced->checked(window()->reducedMotion());
    }

    inline void MotionPanel::styleUpdated() {
        Div::styleUpdated();
        // The tokens of the theme in use, again when it changes
        if (const Theme &theme = styleManager()->theme(); theme.name != _theme) {
            _theme = std::string{theme.name};
            showTokens();
        }
    }

    inline void MotionPanel::showTokens() {
        const auto &motion = styleManager()->theme().metrics.motion;
        const auto    ease   = [](const Ease &curve) { return std::format("cubic-bezier({}, {}, {}, {})", curve.x1, curve.y1, curve.x2, curve.y2); };
        _tokens->text(std::format(
            "{}: fast {} ms, medium {} ms, slow {} ms · standard {} · out {} · spring stiffness {}, damping {}",
            _theme, motion.fast.count(), motion.medium.count(), motion.slow.count(), ease(motion.standard),
            ease(motion.out), motion.spring.stiffness, motion.spring.damping
        ));
    }

    inline void MotionPanel::play() {
        _on = !_on;
        for (Div *puck: _pucks) {
            puck->left(_on ? Full : 0_pct);
        }
        _drawn->toggle(_on);
        for (CurveView *curve: _curves) {
            curve->play();
        }
    }
}
