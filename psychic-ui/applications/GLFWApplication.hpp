#ifdef WITH_GLFW

#pragma once

#include <memory>
#include "../Window.hpp"
#include "../Application.hpp"

#if defined(PSYCHIC_UI_GLAD)
#if defined(PSYCHIC_UI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
#define GLAD_GLAPI_EXPORT
#endif
#include <glad/glad.h>
#else
#if defined(__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GL_GLEXT_PROTOTYPES
#endif
#endif

#include <GLFW/glfw3.h>

// Special treatment of linux Nvidia opengl headers
#if !defined(_WIN32) && !defined(__APPLE__)
#if !defined(GL_UNIFORM_BUFFER)
#warning PSYCHICUI suspects you have the NVIDIA OpenGL headers installed.  \
             Compilation will likely fail. If it does, you have two choices: \
             (1) Re-install the mesa-libGL header files.                     \
             (2) Compile with PSYCHIC_UI_USE_GLAD.
#endif
#endif

namespace psychic_ui {

    #if defined(__APPLE__)
    extern void disable_saved_application_state_osx();
    #endif

    class GLFWSystemWindow;

    class GLFWApplication : public Application {
        friend class GLFWSystemWindow;

    public:
        void init() override;
        void mainloop() override;
        void open(std::shared_ptr<Window> window) override;
        void close(std::shared_ptr<Window> window) override;
        void shutdown() override;
    protected:
        static std::unordered_map<GLFWwindow *, std::unique_ptr<GLFWSystemWindow>> glfwWindows;

        bool running{false};
    };

    class GLFWSystemWindow : public SystemWindow {
        friend class GLFWApplication;

    public:
        GLFWSystemWindow(GLFWApplication *application, std::shared_ptr<Window> window);
        ~GLFWSystemWindow();
        GLFWwindow *glfwWindow() const;
        bool render() override;

    protected:
        GLFWApplication *_glfwApplication{nullptr};
        GLFWwindow      *_glfwWindow{nullptr};
        
        int  _previousWindowX{0};
        int  _previousWindowY{0};
        int  _previousWindowWidth{0};
        int  _previousWindowHeight{0};

        void setTitle(const std::string &title) override;
        void setFullscreen(bool fullscreen) override;
        bool getMinimized() const override;
        void setMinimized(bool minimized) override;
        bool getMaximized() const override;
        void setMaximized(bool maximized) override;
        void setVisible(bool visible) override;
        void setCursor(int cursor) override;
        void startDrag() override;
        void stopDrag() override;
        void setSize(int width, int height) override;
        void setPosition(int x, int y) override ;


        /**
         * Mapping of internal cursors enum to glfw int cursor
         */
        GLFWcursor *_cursors[6];

        void attachCallbacks();
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

        static Key mapKey(int keycode);
    };
}

#endif
