#include "Stage.hpp"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/Window.hpp>

namespace psychic_ui::showcase {
    Stage::Stage(std::shared_ptr<Window> window, const int width, const int height, const float pixelRatio) :
        _window(std::move(window)),
        _pixelRatio(pixelRatio) {
        _systemWindow = std::make_unique<HeadlessSystemWindow>(_window, width, height, pixelRatio);
        _systemWindow->time(_now);
        _surface = SkSurfaces::Raster(
            SkImageInfo::MakeN32Premul(_systemWindow->getFramebufferWidth(), _systemWindow->getFramebufferHeight())
        );
    }

    Stage::Stage(std::shared_ptr<Window> window) :
        _window(std::move(window)) {}

    Stage::~Stage() = default;

    void Stage::frame() {
        if (live()) {
            _window->updateStylesAndLayout();
            return;
        }
        _now += frameTime;
        _systemWindow->time(_now);
        _window->runPosted();
        _window->updateGestures(_now);
        _window->updatePopups(_now);
        _window->updateAnimations(_now);
        _window->updateStylesAndLayout();

        // As `Window::drawAll` paints, on a raster canvas instead of the GL one
        _window->paint(_surface->getCanvas(), _pixelRatio);
    }

    void Stage::advance(const std::chrono::milliseconds duration) {
        if (live()) {
            frame();
            return;
        }
        const std::chrono::nanoseconds end = _now + duration;
        while (_now < end) {
            frame();
        }
    }

    void Stage::settle() {
        if (live()) {
            frame();
            return;
        }
        constexpr std::chrono::seconds limit{4};
        const std::chrono::nanoseconds end = _now + limit;
        int                            quiet = 0;
        while (_now < end) {
            frame();
            const auto next    = _window->nextFrameTime();
            const bool waiting = next && *next < _now + std::chrono::seconds(1);
            // A couple of quiet frames, so what a frame's paint creates gets styled and painted too
            quiet = _window->needsFrame() || waiting ? 0 : quiet + 1;
            if (quiet >= 2) {
                break;
            }
        }
    }

    void Stage::hover(Div &div) {
        frame();
        MouseEvent event = div.mouseEventAtCenter();
        event.x          = event.windowX;
        event.y          = event.windowY;
        event.fromKeyboard = false;
        _window->handleMouseMove(event, false);
    }

    void Stage::key(const Key key, const Mod mod) {
        frame();
        _window->handleKeyPress(key, mod);
        _window->handleKeyRelease(key, mod);
    }

    PngFrame Stage::snapshot() const {
        PngFrame frame{};
        frame.width  = _surface->width();
        frame.height = _surface->height();
        frame.rgba.resize(static_cast<std::size_t>(frame.width) * frame.height * 4);
        const SkImageInfo info = SkImageInfo::Make(frame.width, frame.height, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
        _surface->readPixels(info, frame.rgba.data(), static_cast<std::size_t>(frame.width) * 4, 0, 0);
        return frame;
    }

    sk_sp<SkImage> Stage::image() const {
        return _surface->makeImageSnapshot();
    }
}
