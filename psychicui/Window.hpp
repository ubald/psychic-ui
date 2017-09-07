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
#include "Div.hpp"
#include "style/StyleManager.hpp"
#include "style/Style.hpp"
#include "components/Menu.hpp"

namespace psychicui {

    class Window : public Div {
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

        void toggleMinimized();
        bool minimized() const;
        void setMinimized(bool minimized);

        void toggleMaximized();
        bool maximized() const;
        void setMaximized(bool minimized);

        void toggleFullscreen();
        bool fullscreen() const;
        void setFullscreen(bool fullscreen);

        bool getVisible() const;
        void setVisible(bool value) override;

        void startDrag();
        void stopDrag();

        const int windowX() const;
        const int windowY() const;
        void setWindowPosition(const int x, const int y);

        const int windowWidth() const;
        const int windowHeight() const;
        void setWindowSize(const int width, const int height);

        void open();
        void close();
        void drawAll();
        virtual void drawContents();
        void drawComponents();

        void openMenu(const std::vector<std::shared_ptr<MenuItem>> &items, const int x, const int y);
        void closeMenu();

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

        // region Default DIVs

        std::shared_ptr<Div> app{};
        std::shared_ptr<Div> modal{};
        std::shared_ptr<Div> menu{};

        // endregion

        // region Rendering

        GrContext *_sk_context{nullptr};
        SkSurface *_sk_surface{nullptr};
        SkCanvas  *_sk_canvas{nullptr};
        int       _stencilBits = 0;
        int       _samples     = 0;

        // endregion

        // region Window

        GLFWwindow *_glfwWindow{nullptr};

        std::string _title;
        bool        _fullscreen{false};
        bool        _minimized{false};
        bool        _maximized{false};
        bool        _windowFocused{false};
        bool        _resizable{true};
        bool        _decorated{true};

        int   _fbWidth{0};
        int   _fbHeight{0};
        float _pixelRatio;
        bool  _dragging{false};
        int   _windowDragMouseX{0};
        int   _windowDragMouseY{0};
        int   _windowDragOffsetX{0};
        int   _windowDragOffsetY{0};
        int   _windowX{0};
        int   _windowY{0};
        int   _windowWidth{0};
        int   _windowHeight{0};
        int   _previousWindowX{0};
        int   _previousWindowY{0};
        int   _previousWindowWidth{0};
        int   _previousWindowHeight{0};

        // endregion

        // region Mouse

        GLFWcursor  *_cursors[(int) Cursor::CursorCount];
        Cursor _mouseCursor;
        int    _mouseState{0};
        int    _modifiers{0};
        int    _mouseX{0};
        int    _mouseY{0};
        bool   _dragActive{false};

        // endregion

        // region Initialization

        void initSkia();
        void getSkiaSurface();
        void attachCallbacks();

        // endregion

        // region Events

        double _lastInteraction{0};

        virtual bool dropEvent(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }

        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
        bool keyboardCharacterEvent(unsigned int codepoint) override;

        // Window Delegate
        virtual void windowMoved(const int x, const int y);
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
        void positionEventCallback(int x, int y);
        void focusEventCallback(int focused);
        void iconifyEventCallback(int iconified);
        void closeEventCallback();

        // endregion

    private:
        // Dont allow direct manipulation by others than the window itself
        using Div::add;
        using Div::remove;
        using Div::removeAll;
    };
}

