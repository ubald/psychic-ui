#pragma once

#include <chrono>
#include <memory>
#include <string>
#include "psychic-ui.hpp"
#include "style/Values.hpp"
#include "themes/Theme.hpp"

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
        SystemWindow(SystemWindow &&) = delete;
        SystemWindow &operator=(SystemWindow &&) = delete;

        std::shared_ptr<Window> window() const;

        /**
         * Draws the window into its framebuffer, without presenting it.
         * @return false when there's nothing to draw (the window is hidden)
         */
        virtual bool render() = 0;

        /**
         * An OpenGL function, as GL loaders return them: to be cast to its own type before it is called
         */
        using GLProc = void (*)();

        /**
         * Address of an OpenGL function in this window's context, which Skia builds its GL interface from
         */
        virtual GLProc glProcAddress(const char *name) const = 0;

        /**
         * Whether something other than Psychic UI also draws with this GL context (a host application):
         * Skia then forgets the GL state it caches before each frame. True for the adapters that wrap
         * a host's window; a host that draws nothing else can turn it off.
         */
        bool sharesContext() const;
        void setSharesContext(bool sharesContext);

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
        virtual void setCursor(Cursor cursor) = 0;
        virtual void startDrag() = 0;
        virtual void stopDrag() = 0;

        /**
         * Whether the window can be moved from inside it, by `Window::isDragArea` or `startDrag()`
         */
        virtual bool supportsDrag() const;
        virtual void setSize(int width, int height) = 0;
        virtual void setPosition(int x, int y) = 0;

        /**
         * The clock the backend's touch events are timed with, for `Window::updateGestures`: the steady clock
         * unless the platform has its own
         */
        virtual std::chrono::nanoseconds gestureTime() const;

        /**
         * Wake the loop that waits for this window's events, so that it draws (`Window::requestFrame`). May be called
         * from another thread. Nothing by default: a loop that doesn't wait needs nothing.
         */
        virtual void wake() {}

        /**
         * Whether the system is in light or dark mode, `Unknown` when it doesn't say (this default). An adapter
         * that knows calls `Window::handleColorSchemeChange` when it changes.
         */
        virtual ColorScheme colorScheme() const { return ColorScheme::Unknown; }

        virtual void startTextInput();
        virtual void stopTextInput();

        /**
         * Where text is being typed (window coordinates), with the caret `cursor` from its left, so that an
         * on-screen keyboard or an input method's candidates stay clear of it. Adapters without either ignore it.
         */
        virtual void setTextInputArea(int x, int y, int width, int height, int cursor);

        /**
         * The system clipboard's text, in UTF-8. Without a system clipboard (this default), a clipboard
         * that only lives in this system window.
         */
        virtual std::string getClipboardText() const;
        virtual void setClipboardText(const std::string &text);

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
        float  _mouseX{0};
        float  _mouseY{0};
        int    _x{0};
        int    _y{0};
        int    _width{0};
        int    _height{0};
        int    _fbWidth{0};
        int    _fbHeight{0};
        bool   _focused{false};
        bool   _minimized{false};
        bool   _maximized{false};

    private:
        std::string _clipboardText{};
    };
}
