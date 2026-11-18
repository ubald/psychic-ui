#pragma once

#include <chrono>
#include <memory>
#include "include/core/SkRefCnt.h"
#include <psychic-ui/psychic-ui.hpp>
#include "HeadlessSystemWindow.hpp"
#include "PngWriter.hpp"

class SkImage;
class SkSurface;

namespace psychic_ui {
    class Div;
    class Window;
}

namespace psychic_ui::showcase {
    /**
     * Where a scene runs: its window on a headless system window, drawn into a raster surface at a pixel ratio, with
     * time as the tests drive it: each frame advances the clock by a sixtieth of a second, runs the window's
     * gestures, popups and animations at that time, its styles and layout, then paints. Nothing depends on the real
     * clock, so the same scene always gives the same pixels.
     *
     * Live (for `--live`), the window belongs to a real runner instead: frames only update styles and layout, and
     * time doesn't advance.
     */
    class Stage {
    public:
        static constexpr std::chrono::nanoseconds frameTime{16'666'667};

        Stage(std::shared_ptr<Window> window, int width, int height, float pixelRatio);

        /**
         * A stage for a window a real runner shows
         */
        explicit Stage(std::shared_ptr<Window> window);

        Stage(const Stage &) = delete;
        Stage &operator=(const Stage &) = delete;
        Stage(Stage &&) = delete;
        Stage &operator=(Stage &&) = delete;
        ~Stage();

        Window &window() const { return *_window; }
        bool live() const { return !_systemWindow; }
        std::chrono::nanoseconds now() const { return _now; }

        /**
         * One frame: time advances, then the window updates and paints
         */
        void frame();

        /**
         * Frames until this much time has passed
         */
        void advance(std::chrono::milliseconds duration);

        /**
         * Frames until nothing moves and nothing waits for the next second (at most a few seconds)
         */
        void settle();

        /**
         * Move the mouse to a div's center, as a backend reports it
         */
        void hover(Div &div);

        /**
         * Press and release a key, after a frame so that what it goes to is laid out
         */
        void key(Key key, Mod mod = {});

        /**
         * What the last frame painted
         */
        PngFrame snapshot() const;
        sk_sp<SkImage> image() const;

    private:
        std::shared_ptr<Window>               _window;
        std::unique_ptr<HeadlessSystemWindow> _systemWindow{};
        sk_sp<SkSurface>                      _surface{};
        float                                 _pixelRatio{1.0f};
        std::chrono::nanoseconds              _now{std::chrono::seconds(1)};
    };
}
