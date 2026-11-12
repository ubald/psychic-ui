#pragma once

#include <memory>
#include <string>
#include "psychic-ui.hpp"

namespace psychic_ui {
    class Window;

    /**
     * The platform side of a `Window`: a native window and its OpenGL context.
     *
     * An implementation translates the platform's input and window events into calls on its `Window`
     * and draws it with `render()`, with the window's GL context current.
     *
     * Sizes: `getWidth()`/`getHeight()` are the window's size in the units the platform reports the mouse in,
     * which is what the UI is laid out in. `getFramebufferWidth()`/`getFramebufferHeight()` are the size of
     * what is drawn, in pixels: larger on a high pixel density display, by `getPixelRatio()`.
     */
    class SystemWindow {
    public:
        explicit SystemWindow(std::shared_ptr<Window> window);
        virtual ~SystemWindow();

        SystemWindow(const SystemWindow &) = delete;
        SystemWindow &operator=(const SystemWindow &) = delete;

        std::shared_ptr<Window> window() const;

        /**
         * Draws the window into its framebuffer, without presenting it.
         * @return false when there's nothing to draw (the window is hidden)
         */
        virtual bool render() = 0;

        /**
         * Address of an OpenGL function in this window's context, which Skia builds its GL interface from
         */
        virtual void *glProcAddress(const char *name) const = 0;

        /**
         * Whether something other than Psychic UI also draws with this GL context (a host application):
         * Skia then forgets the GL state it caches before each frame.
         */
        bool sharesContext() const;

        int getX() const;
        int getY() const;
        int getWidth() const;
        int getHeight() const;
        int getFramebufferWidth() const;
        int getFramebufferHeight() const;
        float getPixelRatio() const;
        int getSamples() const;
        int getStencilBits() const;

        virtual void setTitle(const std::string &title) = 0;
        virtual void setFullscreen(bool fullscreen) = 0;
        virtual bool getMinimized() const = 0;
        virtual void setMinimized(bool minimized) = 0;
        virtual bool getMaximized() const = 0;
        virtual void setMaximized(bool maximized) = 0;
        virtual void setVisible(bool visible) = 0;
        virtual void setCursor(int cursor) = 0;
        virtual void startDrag() = 0;
        virtual void stopDrag() = 0;
        virtual void setSize(int width, int height) = 0;
        virtual void setPosition(int x, int y) = 0;

        virtual void startTextInput();
        virtual void stopTextInput();

    protected:
        std::shared_ptr<Window> _window;

        /**
         * Lets go of the window and frees its Skia objects. A subclass calls it first thing in its destructor,
         * while its GL context still exists and is current.
         */
        void detach();

        /**
         * Reads the sample count and stencil bits of the current context's default framebuffer,
         * which the Skia render target must match
         */
        void readFramebufferFormat();

        bool _sharesContext{false};

        bool _dragging{false};

        int    _stencilBits{8};
        int    _samples{0};
        int    _mouseX{0};
        int    _mouseY{0};
        int    _mouseState{0};
        int    _x{0};
        int    _y{0};
        int    _width{0};
        int    _height{0};
        int    _fbWidth{0};
        int    _fbHeight{0};
        bool   _focused{false};
        bool   _minimized{false};
        bool   _maximized{false};
    };
}
