#pragma once

#include <memory>
#include "include/core/SkSurface.h"
#include <psychic-ui/SystemWindow.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>

namespace psychic_ui_tests {
    using namespace psychic_ui;

    /**
     * A window that counts what reaches it. Motion is reduced: values are at their end after a style pass, without
     * advancing time (tests of motion turn it back on).
     */
    struct TestWindow : Window {
        std::vector<std::string> dropped{};

        TestWindow() : Window("Test") {
            skinSet<PivotalSkins>();
            reducedMotion(true);
            // Context menus on the right button's press on every platform (Windows defaults to its release): tests of the
            // release turn it back on
            contextMenuOnRelease(false);
        }

        bool dropEvent(const std::vector<std::string> &filenames) override {
            dropped = filenames;
            return true;
        }

        /**
         * Styles, layout and a raster render, twice since rendering creates the children of data containers
         */
        void layout(SkCanvas *canvas) {
            for (int i = 0; i < 2; ++i) {
                updateStylesAndLayout();
                render(canvas);
            }
        }
    };

    /**
     * A system window without a platform: no GL, the base class's in-process clipboard,
     * and a raster canvas to run the frame's passes on
     */
    struct FakeSystemWindow : SystemWindow {
        sk_sp<SkSurface> surface{SkSurfaces::Raster(SkImageInfo::MakeN32Premul(800, 600))};
        int              dragsStarted{0};
        int              wakes{0}; // `wake` calls: the window asking for a frame from outside the loop

        std::shared_ptr<TestWindow> testWindow;

        explicit FakeSystemWindow(const std::shared_ptr<TestWindow> &window, int width = 800, int height = 600, float pixelRatio = 1.0f) :
            SystemWindow(window), testWindow(window) {
            _width    = width;
            _height   = height;
            _fbWidth  = static_cast<int>(static_cast<float>(width) * pixelRatio);
            _fbHeight = static_cast<int>(static_cast<float>(height) * pixelRatio);
            _window->open(this);
        }

        ~FakeSystemWindow() override {
            detach();
        }

        void layout() {
            testWindow->layout(surface->getCanvas());
        }

        bool render() override {
            layout();
            return true;
        }

        void *glProcAddress(const char * /*name*/) const override { return nullptr; }

        void setTitle(const std::string & /*title*/) override {}
        void setFullscreen(bool /*fullscreen*/) override {}
        bool getMinimized() const override { return false; }
        void setMinimized(bool /*minimized*/) override {}
        bool getMaximized() const override { return false; }
        void setMaximized(bool /*maximized*/) override {}
        void setVisible(bool /*visible*/) override {}
        void setCursor(Cursor /*cursor*/) override {}
        void startDrag() override { ++dragsStarted; }
        void wake() override { ++wakes; }
        void stopDrag() override {}
        void setSize(int /*width*/, int /*height*/) override {}
        void setPosition(int /*x*/, int /*y*/) override {}
    };

}
