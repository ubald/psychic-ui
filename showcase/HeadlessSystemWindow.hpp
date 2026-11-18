#pragma once

#include <chrono>
#include <memory>
#include <psychic-ui/SystemWindow.hpp>

namespace psychic_ui::showcase {
    /**
     * A system window with no platform behind it: a size, a pixel ratio and a clock the caller sets, and nothing
     * shown anywhere. The window is drawn into a raster canvas (see `Capture`), never through GL.
     */
    class HeadlessSystemWindow : public SystemWindow {
    public:
        HeadlessSystemWindow(std::shared_ptr<Window> window, int width, int height, float pixelRatio);
        HeadlessSystemWindow(const HeadlessSystemWindow &) = delete;
        HeadlessSystemWindow &operator=(const HeadlessSystemWindow &) = delete;
        HeadlessSystemWindow(HeadlessSystemWindow &&) = delete;
        HeadlessSystemWindow &operator=(HeadlessSystemWindow &&) = delete;
        ~HeadlessSystemWindow() override;

        /**
         * The time `gestureTime()` reports: the frames' time, which the capture advances
         */
        void time(std::chrono::nanoseconds now) { _now = now; }
        std::chrono::nanoseconds gestureTime() const override { return _now; }

        bool render() override { return false; } // Captures draw the window themselves, into their own canvas
        GLProc glProcAddress(const char * /*name*/) const override { return nullptr; }

        void setTitle(const std::string & /*title*/) override {}
        void setFullscreen(bool /*fullscreen*/) override {}
        bool getMinimized() const override { return false; }
        void setMinimized(bool /*minimized*/) override {}
        bool getMaximized() const override { return false; }
        void setMaximized(bool /*maximized*/) override {}
        void setVisible(bool /*visible*/) override {}
        void setCursor(Cursor /*cursor*/) override {}
        void startDrag() override {}
        void stopDrag() override {}
        void setSize(int /*width*/, int /*height*/) override {}
        void setPosition(int /*x*/, int /*y*/) override {}

    private:
        std::chrono::nanoseconds _now{0};
    };
}
