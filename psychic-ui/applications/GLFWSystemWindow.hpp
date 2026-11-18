#pragma once

#include <memory>
#include "../SystemWindow.hpp"

struct GLFWwindow;
struct GLFWcursor;

namespace psychic_ui {

    /**
     * Shows a Psychic UI `Window` in a GLFW window that a host application owns, the way Dear ImGui's
     * `imgui_impl_glfw` embeds ImGui. The host initializes GLFW, creates the window with an OpenGL 3.3+
     * context and runs the loop; the adapter only installs its input callbacks and draws on request:
     *
     *     GLFWwindow *glfwWindow = glfwCreateWindow(1280, 720, "Host", nullptr, nullptr);
     *     glfwMakeContextCurrent(glfwWindow);
     *     glfwSetKeyCallback(glfwWindow, hostKeyCallback);      // still called: the adapter chains to it
     *
     *     auto ui = std::make_shared<MyWindow>();
     *     psychic_ui::GLFWSystemWindow adapter(ui, glfwWindow);
     *
     *     while (!glfwWindowShouldClose(glfwWindow)) {
     *         glfwPollEvents();
     *         adapter.render();                                   // makes the context current and draws
     *         glfwSwapBuffers(glfwWindow);
     *     }
     *
     * Install your own callbacks before creating the adapter: it calls the ones it replaces first, then
     * handles the event. Destroy the adapter before the window: it puts those callbacks back.
     *
     * Limits are GLFW's: no input method composition (characters arrive as typed), and on Wayland the
     * window can't be moved from inside (`supportsDrag()` is false), so use system decorations there
     * rather than a custom title bar. GLFW has no touch input either: the window stays in pointer mode,
     * whatever the system makes of fingers, and the adapter never changes the mode, so a host with touch
     * input of its own can hand it to `Window::handleTouchPress` and the others.
     */
    class GLFWSystemWindow : public SystemWindow {
    public:
        GLFWSystemWindow(std::shared_ptr<Window> window, GLFWwindow *glfwWindow);
        ~GLFWSystemWindow() override;
        GLFWSystemWindow(const GLFWSystemWindow &) = delete;
        GLFWSystemWindow &operator=(const GLFWSystemWindow &) = delete;
        GLFWSystemWindow(GLFWSystemWindow &&) = delete;
        GLFWSystemWindow &operator=(GLFWSystemWindow &&) = delete;

        GLFWwindow *glfwWindow() const;

        /**
         * Draws the window. When the window was closed from inside (a title bar's close button), marks the
         * GLFW window as to be closed and returns false.
         */
        bool render() override;
        void *glProcAddress(const char *name) const override;

        void setTitle(const std::string &title) override;
        void setFullscreen(bool fullscreen) override;
        bool getMinimized() const override;
        void setMinimized(bool minimized) override;
        bool getMaximized() const override;
        void setMaximized(bool maximized) override;
        void setVisible(bool visible) override;
        void setCursor(Cursor cursor) override;
        void startDrag() override;
        void stopDrag() override;
        bool supportsDrag() const override;

        /**
         * `glfwPostEmptyEvent`, which ends a host's `glfwWaitEvents`: a host that waits draws when
         * `Window::untilNextFrame` says so
         */
        void wake() override;
        void setSize(int width, int height) override;
        void setPosition(int x, int y) override;

        std::string getClipboardText() const override;
        void setClipboardText(const std::string &text) override;

        static Mod mapMods(int mods);
        static Key mapKey(int key);

    private:
        struct Callbacks;

        GLFWwindow                *_glfwWindow{nullptr};
        std::unique_ptr<Callbacks> _previous;

        /**
         * System cursors, indexed by `Cursor`
         */
        GLFWcursor *_cursors[6]{};

        /**
         * Where the window was grabbed, in window coordinates, while dragging it
         */
        double _dragX{0};
        double _dragY{0};
        bool   _warnedAboutDrag{false};

        int _windowedX{0};
        int _windowedY{0};
        int _windowedWidth{0};
        int _windowedHeight{0};

        Mod _modifiers{};

        static GLFWSystemWindow *find(GLFWwindow *glfwWindow);
        void readSize();

        void cursorPosEvent(double x, double y);
        void mouseButtonEvent(int button, int action, int mods);
        void scrollEvent(double x, double y);
        void keyEvent(int key, int scancode, int action, int mods);
        void charEvent(unsigned int codepoint);
        void dropEvent(int count, const char **paths);
        void sizeEvent();
        void positionEvent(int x, int y);
        void focusEvent(int focused);
        void iconifyEvent(int iconified);
        void maximizeEvent(int maximized);
        void closeEvent();
    };
}
