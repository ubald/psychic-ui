#pragma once

#include <string>
#include <unordered_map>
#include "GrContext.h"
#include "gl/GrGLInterface.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkPaint.h"
#include "opengl.hpp"
#include "Component.hpp"
#include "style/StyleManager.hpp"
#include "style/Style.hpp"

namespace psychicui {

    struct MouseMessage {
        int x, y;
    };

    class Window : public Component {
    public:
        static std::unordered_map<GLFWwindow *, Window *> windows;

        Window(const std::string &title);
        virtual ~Window();

        /**
         * GLFW Window
         * @return GLFWWindow*
         */
        GLFWwindow *glfwWindow();

        Window *window() override;

        const std::string title() const;
        void setTitle(const std::string &title);


        bool minimized() const;
        void setMinimized(bool minimized);

        bool fullscreen() const;
        void setFullscreen(bool fullscreen);

        void setVisible(bool value) override;

        const int windowX() const;
        const int windowY() const;
        void setWindowPosition(const int x, const int y);

        const int windowWidth() const;
        const int windowHeight() const;
        void setWindowSize(const int width, const int height);

        void open();
        void drawAll();
        virtual void drawContents();
        void drawComponents();

        // Style

        /**
         * Helper to forward the stylesheet loading to the style manager
         * @tparam T :StyleSheet
         */
        template<typename T>
        void loadStyleSheet() {
            styleManager()->loadStyleSheet<T>();
        }

    protected:
        // GLFW Window
        GLFWwindow *_glfwWindow{nullptr};

        // region Rendering

        GrContext *_sk_context{nullptr};
        SkSurface *_sk_surface{nullptr};
        SkCanvas  *_sk_canvas{nullptr};
        int       _stencilBits = 0;
        int       _samples     = 0;

        // endregion

        // region Window

        std::string _title;
        bool        _fullscreen{false};
        bool        _minimized{false};
        bool        _windowFocused{false};
        bool        _resizable{true};
        bool        _decorated{true};
        GLFWcursor  *_cursors[(int) Cursor::CursorCount];

        // endregion

        int   _fbWidth{0};
        int   _fbHeight{0};
        float _pixelRatio;
        int   _windowX{0};
        int   _windowY{0};
        int   _windowWidth{0};
        int   _windowHeight{0};
        int   _previousWindowX{0};
        int   _previousWindowY{0};
        int   _previousWindowWidth{0};
        int   _previousWindowHeight{0};

        double _lastInteraction;

        // region Mouse

        Cursor _mouseCursor;
        int  _mouseState{0};
        int  _modifiers{0};
        int  _mouseX{0};
        int  _mouseY{0};
        bool _dragActive{false};

        // endregion

        std::shared_ptr<Component>              _dragComponent = nullptr;
        std::vector<std::shared_ptr<Component>> _focusPath;

        void initSkia();
        void getSkiaSurface();
        void attachCallbacks();

        /* Events */
        virtual bool dropEvent(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }

        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
        bool keyboardCharacterEvent(unsigned int codepoint) override;

        // Window Delegate
        virtual void windowResized(const int width, const int height);
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

