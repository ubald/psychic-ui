#pragma once

#include <string>
#include <map>
#include "opengl.hpp"
#include "Window.hpp"
#include "Widget.hpp"
#include "Style.hpp"

namespace psychicui {
    class Screen : public Widget {
    public:
        static std::map<GLFWwindow *, Screen *> screens;

        GLFWwindow *window();


        Screen(const std::string &title);
        virtual ~Screen();

        const std::string title() const;
        void setTitle(const std::string &title);

        bool minimized() const;
        void setMinimized(bool minimized);

        bool fullscreen() const;
        void setFullscreen(bool fullscreen);

        void setVisible(bool visible) override;

        const Vector2i &windowPosition() const;
        void setWindowPosition(const Vector2i &position);

        const Vector2i &windowSize() const;
        virtual void setWindowSize(const Vector2i &size);

        void open();
        void drawAll();

        virtual void drawContents();
        void drawWidgets();
        void updateFocus(Widget *widget);
        void disposeWindow(Window *window);
        void centerWindow(Window *window);
        void moveWindowToFront(Window *window);

    protected:
        GLFWwindow            *_window{nullptr};
        NVGcontext            *_nvgContext{nullptr};
        std::string           _title;
        Vector2i              _windowPosition{0, 0};
        Vector2i              _fbSize{0, 0};
        float                 _pixelRatio;
        Vector2i              _previousScreenPosition{0, 0};
        Vector2i              _previousScreenSize{0, 0};
        bool                  _fullscreen{false};
        bool                  _minimized{false};
        bool                  _windowFocused{false};
        bool                  _resizable{true};
        bool                  _decorated{true};
        double                _lastInteraction;
        int                   _mouseState{0};
        int                   _modifiers{0};
        Vector2i              _mousePosition{0, 0};
        bool                  _dragActive{false};
        Widget                *_dragWidget = nullptr;
        GLFWcursor            *_cursors[(int) Cursor::CursorCount];
        std::vector<Widget *> _focusPath;

        /* Events */
        virtual bool dropEvent(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }

        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
        bool keyboardCharacterEvent(unsigned int codepoint) override;

        // Window Delegate
        virtual void windowResized(const Vector2i &size);
        virtual void windowActivated();
        virtual void windowDeactivated();
        virtual void windowMinimized();
        virtual void windowRestored();
        virtual bool windowShouldClose();

        // Event handlers
        void cursorPosEventCallback(double x, double y);
        void mouseButtonEventCallback(int button, int action, int modifiers);
        void keyEventCallback(int key, int scancode, int action, int mods);
        void charEventCallback(unsigned int codepoint);
        void dropEventCallback(int count, const char **filenames);
        void scrollEventCallback(double x, double y);
        void resizeEventCallback(int width, int height);
        void focusEventCallback(int focused);
        void iconifyEventCallback(int iconified);
        void closeEventCallback();
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}

