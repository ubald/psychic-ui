#pragma once

#include <memory>
#include "psychic-ui.hpp"

namespace psychic_ui {
    class Window;

    class Application {
    public:
        virtual void init() = 0;
        virtual void mainloop() = 0;
        virtual void open(std::shared_ptr<Window> window) = 0;
        virtual void close(std::shared_ptr<Window> window) = 0;
        virtual void shutdown() = 0;
    };

    class SystemWindow {
        friend class Application;

    public:
        SystemWindow(Application *application, std::shared_ptr<Window> window);

        std::shared_ptr<Window> window() const;

        virtual bool render() = 0;

        int getX() const;
        int getY() const;
        int getWidth() const;
        int getHeight() const;
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

    protected:
        Application             *_application;
        std::shared_ptr<Window> _window;

        bool _dragging{false};
        int  _windowDragMouseX{0};
        int  _windowDragMouseY{0};
        int  _windowDragOffsetX{0};
        int  _windowDragOffsetY{0};
        
        double _lastInteraction{0};
        int    _stencilBits{8};
        int    _samples{4};
        int    _mouseX{0};
        int    _mouseY{0};
        int    _mouseState{0};
        int    _modifiers{0};
        int    _x{0};
        int    _y{0};
        int    _width{0};
        int    _height{0};
        //int    _fbWidth{0};
        //int    _fbHeight{0};
        float  _pixelRatio = 0.f;
        bool   _focused{false};
        bool   _minimized{false};
        bool   _maximized{false};
    };
}
