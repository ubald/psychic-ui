#pragma once

#include <string>
#include <map>
#include "GrContext.h"
#include "gl/GrGLInterface.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkPaint.h"
#include "opengl.hpp"
#include "Widget.hpp"
#include "Style.hpp"

//#define NANOVG_GL3_IMPLEMENTATION
//#include <nanovg_gl.h>

namespace psychicui {
    class Window : public Widget {
    public:
        static std::map<GLFWwindow *, Window *> windows;

        GLFWwindow *window();


        Window(const std::string &title);
        virtual ~Window();

        const std::string title() const;
        void setTitle(const std::string &title);

        bool minimized() const;
        void setMinimized(bool minimized);

        bool fullscreen() const;
        void setFullscreen(bool fullscreen);

        void setVisible(bool value) override;

//        const Vector2i &setWindowPosition() const;
        void setWindowPosition(const int &x, const int &y);

//        const Vector2i &setWindowSize() const;
        virtual void setWindowSize(const int &width, const int &height);

        void open();
        void drawAll();
        virtual void drawContents();
        void drawWidgets();

        void disposePanel(std::shared_ptr<Panel> panel);
        void centerPanel(std::shared_ptr<Panel> panel);
        void movePanelToFront(std::shared_ptr<Panel> panel);

    protected:
        // GLFW
        GLFWwindow *_window{nullptr};

        // Rendering
        GrContext *_sk_context{nullptr};
        SkSurface *_sk_surface{nullptr};
        SkCanvas  *_sk_canvas{nullptr};
        int       _stencilBits = 0;
        int       _samples     = 0;

        // Window
        std::string _title;
        bool        _fullscreen{false};
        bool        _minimized{false};
        bool        _windowFocused{false};
        bool        _resizable{true};
        bool        _decorated{true};
        GLFWcursor  *_cursors[(int) Cursor::CursorCount];

        int      _fbWidth{0};
        int      _fbHeight{0};
        float    _pixelRatio;
        int      _windowX{0};
        int      _windowY{0};
        int      _previousWindowX{0};
        int      _previousWindowY{0};
        int      _previousWindowWidth{0};
        int      _previousWindowHeight{0};

        double _lastInteraction;

        int  _mouseState{0};
        int  _modifiers{0};
        int  _mouseX{0};
        int  _mouseY{0};
        bool _dragActive{false};

        std::shared_ptr<Widget>              _dragWidget = nullptr;
        std::vector<std::shared_ptr<Widget>> _focusPath;

        void initSkia();
        void getSkiaSurface();
        void attachCallbacks();
        void requestFocus(Widget *widget) override;

        /* Events */
        virtual bool dropEvent(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }

        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
        bool keyboardCharacterEvent(unsigned int codepoint) override;

        // Window Delegate
        virtual void windowResized(const int &width, const int &height);
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
    };
}

